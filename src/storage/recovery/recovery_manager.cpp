#include <vector>

#include "storage/recovery/recovery_manager.h"

#include "storage/write_ahead_log/log_io.h"
namespace terrier::storage {

void RecoveryManager::RecoverFromLogs() {
  // Replay logs until the log provider no longer gives us logs
  while (true) {
    auto *log_record = log_provider_->GetNextRecord();

    if (log_record == nullptr) break;

    // If the record is a commit or abort, we replay it, which will replay all its buffered records. Otherwise, we
    // buffer the record.
    if (log_record->RecordType() == LogRecordType::COMMIT || log_record->RecordType() == LogRecordType::ABORT) {
      ReplayTransaction(log_record);
    } else {
      buffered_changes_map_[log_record->TxnBegin()].push_back(log_record);
    }
  }
  TERRIER_ASSERT(buffered_changes_map_.empty(), "All buffered changes should have been processed");
}

void RecoveryManager::ReplayTransaction(LogRecord *log_record) {
  TERRIER_ASSERT(log_record->RecordType() == LogRecordType::COMMIT || log_record->RecordType() == LogRecordType::ABORT,
                 "Records should only be replayed when a commit or abort record is seen");

  // If we are aborting, we can free and discard all buffered changes. Nothing needs to be replayed
  if (log_record->RecordType() == LogRecordType::ABORT) {
    for (auto *buffered_record : buffered_changes_map_[log_record->TxnBegin()]) {
      delete[] reinterpret_cast<byte *>(buffered_record);
    }
    buffered_changes_map_.erase(log_record->TxnBegin());
  } else {
    TERRIER_ASSERT(log_record->RecordType() == LogRecordType::COMMIT, "Should only replay when we see a commit record");
    // Begin a txn to replay changes with
    auto *txn = txn_manager_->BeginTransaction();

    // Apply all buffered changes. They should all succeed. After applying we can safely delete the record
    for (auto *buffered_record : buffered_changes_map_[log_record->TxnBegin()]) {
      bool result UNUSED_ATTRIBUTE = true;

      if (buffered_record->RecordType() == LogRecordType::DELETE) {
        auto *delete_record = buffered_record->GetUnderlyingRecordBodyAs<DeleteRecord>();

        // Get tuple slot
        TERRIER_ASSERT(tuple_slot_map_.find(delete_record->GetTupleSlot()) != tuple_slot_map_.end(),
                       "Tuple slot must already be mapped if Delete record is found");
        auto new_tuple_slot = tuple_slot_map_[delete_record->GetTupleSlot()];

        // Delete the tuple
        auto *sql_table = GetSqlTable(delete_record->GetDatabaseOid(), delete_record->GetTableOid());
        result = sql_table->Delete(txn, new_tuple_slot);
        // We can delete the TupleSlot from the map
        tuple_slot_map_.erase(delete_record->GetTupleSlot());
      } else {
        TERRIER_ASSERT(buffered_record->RecordType() == LogRecordType::REDO, "Must be a redo record");
        auto *redo_record = buffered_record->GetUnderlyingRecordBodyAs<RedoRecord>();

        // Search the map for the tuple slot. If the tuple slot is not in the map, then we have not seen this tuple slot
        // before, so this record is an Insert. If we have seen it before, then the record is an Update
        auto *sql_table = GetSqlTable(redo_record->GetDatabaseOid(), redo_record->GetTableOid());
        auto search = tuple_slot_map_.find(redo_record->GetTupleSlot());
        if (search == tuple_slot_map_.end()) {
          // Save the old tuple slot, and reset the tuple slot in the record
          auto old_tuple_slot = redo_record->GetTupleSlot();
          redo_record->SetTupleSlot(TupleSlot(nullptr, 0));
          // Insert will always succeed
          auto new_tuple_slot = sql_table->Insert(txn, redo_record);
          // Create a mapping of the old to new tuple. The new tuple slot should be used for updates and deletes.
          tuple_slot_map_[old_tuple_slot] = new_tuple_slot;
        } else {
          auto new_tuple_slot = search->second;
          redo_record->SetTupleSlot(new_tuple_slot);
          // We should return the output of update. An update can fail for a write write conflict, but this will be
          // resolved when we see an abort record. The caller of this function will buffer the redo_record if it fails
          result = sql_table->Update(txn, redo_record);
        }
      }
      TERRIER_ASSERT(result, "Buffered changes should always succeed during commit");
      delete[] reinterpret_cast<byte *>(buffered_record);
    }
    buffered_changes_map_.erase(log_record->TxnBegin());
    // Commit the txn
    txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  }
  delete[] reinterpret_cast<byte *>(log_record);
}
}  // namespace terrier::storage
