#include "transaction/transaction_manager.h"
#include <utility>

namespace terrier::transaction {
TransactionContext *TransactionManager::BeginTransaction() {
  common::SharedLatch::ScopedSharedLatch guard(&commit_latch_);
  timestamp_t start_time = time_++;
  // TODO(Tianyu):
  // Maybe embed this into the data structure, or use an object pool?
  // Doing this with std::map or other data structure is risky though, as they may not
  // guarantee that the iterator or underlying pointer is stable across operations.
  // (That is, they may change as concurrent inserts and deletes happen)
  auto *result = new TransactionContext(start_time, start_time + INT64_MIN, buffer_pool_, log_manager_);
  table_latch_.Lock();
  auto ret UNUSED_ATTRIBUTE = curr_running_txns_.emplace(result->StartTime(), result);
  TERRIER_ASSERT(ret.second, "commit start time should be globally unique");
  table_latch_.Unlock();
  return result;
}

timestamp_t TransactionManager::Commit(TransactionContext *const txn, const std::function<void()> &callback) {
  common::SharedLatch::ScopedExclusiveLatch guard(&commit_latch_);
  // TODO(Tianyu): Potentially don't need to get a commit time for read-only txns
  const timestamp_t commit_time = time_++;
  // Flip all timestamps to be committed
  for (auto &it : txn->undo_buffer_) it.Timestamp().store(commit_time);
  table_latch_.Lock();
  const timestamp_t start_time = txn->StartTime();
  size_t result UNUSED_ATTRIBUTE = curr_running_txns_.erase(start_time);
  TERRIER_ASSERT(result == 1, "Committed transaction did not exist in global transactions table");
  txn->TxnId().store(commit_time);

  if (log_manager_ != LOGGING_DISABLED && !txn->undo_buffer_.Empty()) {
    // At this point the commit has already happened for the rest of the system.
    // Here we will manually add a commit record and flush the buffer to ensure the logger
    // sees this record.
    byte *commit_record = txn->redo_buffer_.NewEntry(storage::CommitRecord::Size());
    storage::CommitRecord::Initialize(commit_record, txn->StartTime(), commit_time);
    log_manager_->RegisterTransactionFlushedCallback(txn->StartTime(), callback);
    txn->redo_buffer_.Finish();
  }
  if (gc_enabled_) completed_txns_.push_front(txn);
  table_latch_.Unlock();
  // TODO(Tianyu): Is this the right thing to do?
  if (log_manager_ == LOGGING_DISABLED) callback();
  return commit_time;
}

void TransactionManager::Abort(TransactionContext *const txn) {
  // no latch required on undo since all operations are transaction-local
  timestamp_t txn_id = txn->TxnId().load();  // will not change
  for (auto &it : txn->undo_buffer_) Rollback(txn_id, it);
  table_latch_.Lock();
  const timestamp_t start_time = txn->StartTime();
  size_t ret UNUSED_ATTRIBUTE = curr_running_txns_.erase(start_time);
  TERRIER_ASSERT(ret == 1, "Aborted transaction did not exist in global transactions table");
  if (gc_enabled_) completed_txns_.push_front(txn);
  table_latch_.Unlock();
  if (log_manager_ != LOGGING_DISABLED) txn->redo_buffer_.Discard();
}

timestamp_t TransactionManager::OldestTransactionStartTime() const {
  table_latch_.Lock();
  auto oldest_txn = curr_running_txns_.begin();
  timestamp_t result = (oldest_txn != curr_running_txns_.end()) ? oldest_txn->second->StartTime() : time_.load();
  table_latch_.Unlock();
  return result;
}

TransactionQueue TransactionManager::CompletedTransactionsForGC() {
  table_latch_.Lock();
  TransactionQueue hand_to_gc(std::move(completed_txns_));
  TERRIER_ASSERT(completed_txns_.empty(), "TransactionManager's queue should now be empty.");
  table_latch_.Unlock();
  return hand_to_gc;
}

void TransactionManager::Rollback(const timestamp_t txn_id, const storage::UndoRecord &record) const {
  storage::DataTable *const table = record.Table();
  const storage::TupleSlot slot = record.Slot();
  storage::UndoRecord *const version_ptr = table->AtomicallyReadVersionPtr(slot, table->accessor_);
  // We do not hold the lock. Should just return
  if (version_ptr == nullptr || version_ptr->Timestamp().load() != txn_id) return;
  // Re-apply the before image
  for (uint16_t i = 0; i < version_ptr->Delta()->NumColumns(); i++)
    storage::StorageUtil::CopyAttrFromProjection(table->accessor_, slot, *(version_ptr->Delta()), i);
  // Remove this delta record from the version chain, effectively releasing the lock. At this point, the tuple
  // has been restored to its original form. No CAS needed since we still hold the write lock at time of the atomic
  // write.
  table->AtomicallyWriteVersionPtr(slot, table->accessor_, version_ptr->Next());
}
}  // namespace terrier::transaction
