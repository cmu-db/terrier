#include "storage/checkpoints/checkpoint.h"
#include <common/worker_pool.h>
#include <storage/block_compactor.h>
#include <storage/recovery/disk_log_provider.h>
#include <fstream>
#include "common/constants.h"
namespace terrier::storage {

bool Checkpoint::TakeCheckpoint(const std::string &path, catalog::db_oid_t db, const char *cur_log_file,
                                uint32_t num_threads, common::WorkerPool *thread_pool_) {
  // get db catalog accessor
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(static_cast<common::ManagedPointer<transaction::TransactionContext>>(txn), db);
  std::unordered_set<catalog::table_oid_t> table_oids = accessor->GetAllTableOids();

  // lock the log_serializer buffer to prevent further logging
  common::ManagedPointer<LogSerializerTask> log_serializer_task = log_manager_->log_serializer_task_;
  log_serializer_task->flush_queue_latch_.Lock();

  // copy all data tables to serialize later
  for (const auto &oid : table_oids) {
    // copy data table
    common::ManagedPointer<storage::SqlTable> curr_sql_table = accessor->GetTable(oid);
    storage::DataTable *curr_data_table = curr_sql_table->table_.data_table_;
    std::list<RawBlock *> new_blocks(curr_data_table->blocks_);
    storage::DataTable *new_table = new storage::DataTable(
        curr_data_table->block_store_, curr_data_table->GetBlockLayout(), curr_data_table->layout_version_);
    new_table->blocks_ = new_blocks;
    queue.emplace_back(oid, new_table);
  }
  std::string tmp = "_catalog";
  std::string catalog_file = std::string(cur_log_file) + tmp; //catalog file: test.log_catalog
  log_serializer_task->flush_queue_latch_.Unlock();

  // filter catalog logs to catalog file
  FilterCatalogLogs(cur_log_file, catalog_file);
  remove(cur_log_file);

  // create empty log file for other logs
  std::ofstream new_log_file(cur_log_file, std::ios::in | std::ios::app);
  log_manager_->ResetLogFilePath(cur_log_file);

  auto workload = [&](uint32_t worker_id) {
    // copy contents of table to disk
    WriteToDisk(path, accessor, db);
  };
  for (auto i = 0u; i < num_threads; i++) {
    thread_pool_->SubmitTask([i, &workload] { workload(i); });
  }
  thread_pool_->WaitUntilAllFinished();
  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  if (queue.size() > 0) {
    // the table oid that failed to be backup
    return false;
  }
  return true;
}

void Checkpoint::WriteToDisk(const std::string &path, const std::unique_ptr<catalog::CatalogAccessor> &accessor,
                             catalog::db_oid_t db_oid) {
  while (queue.size() > 0) {
    queue_latch.lock();
    if (queue.size() <= 0) {
      queue_latch.unlock();
      return;
    }
    catalog::table_oid_t curr_table_oid = queue[0].first;
    storage::DataTable *data_table = queue[0].second;
    queue.erase(queue.begin());
    queue_latch.unlock();

    // get daga table
    std::string out_file = path + GenFileName(db_oid, curr_table_oid);

    const BlockLayout &layout = data_table->GetBlockLayout();
    std::vector<type::TypeId> column_types;
    column_types.resize(layout.NumColumns());
    for (RawBlock *block : data_table->blocks_) {
      auto &arrow_metadata = data_table->accessor_.GetArrowBlockMetadata(block);
      for (storage::col_id_t col_id : layout.AllColumns()) {
        if (layout.IsVarlen(col_id)) {
          arrow_metadata.GetColumnInfo(layout, col_id).Type() = storage::ArrowColumnType::GATHERED_VARLEN;
          column_types[!col_id] = type::TypeId::VARCHAR;
        } else {
          arrow_metadata.GetColumnInfo(layout, col_id).Type() = storage::ArrowColumnType::FIXED_LENGTH;
          column_types[!col_id] = type::TypeId::INTEGER;
        }
      }
    }

    // compact blocks into arrow format
    storage::BlockCompactor compactor;
    for (RawBlock *block : data_table->blocks_) {
      compactor.PutInQueue(block);
      compactor.ProcessCompactionQueue(deferred_action_manager_.Get(), txn_manager_.Get());  // compaction pass
      gc_->PerformGarbageCollection();
      compactor.PutInQueue(block);
      compactor.ProcessCompactionQueue(deferred_action_manager_.Get(), txn_manager_.Get());  // gathering pass
    }

    // write to disk
    storage::ArrowSerializer arrow_serializer(*data_table);
    arrow_serializer.ExportTable(out_file, &column_types, false);
  }
}

void Checkpoint::FilterCatalogLogs(const std::string &old_log_path, const std::string &new_log_path) {
  DiskLogProvider log_provider(old_log_path);
  std::ofstream outfile(new_log_path, std::ios::out | std::ios::binary | std::ios::app);
  std::ifstream infile(old_log_path, std::ios::in | std::ios::binary);


  while (true) {
    auto pos = log_provider.in_.read_head_;
    auto pair = log_provider.GetNextRecord();
    auto *log_record = pair.first;

    // If we have exhausted all the logs, break from the loop
    if (log_record == nullptr) break;
    auto cur_pos = log_provider.in_.read_head_;
//    record_size = log_record->Size();
    uint32_t size = cur_pos - pos;
    if (cur_pos < pos) size = (cur_pos + common::Constants::LOG_BUFFER_SIZE) - pos;
    auto buf = new char[size];

    if (log_record->RecordType() == LogRecordType::ABORT || log_record->RecordType() == LogRecordType::COMMIT) {
      infile.read(reinterpret_cast<char *>(buf), size);
      outfile.write(reinterpret_cast<char *>(buf), size);
//      o << "A/C " << log_record->Size()<<std::endl;
    } else {
      if (log_record->RecordType() == LogRecordType::REDO) {
        auto *record_body = log_record->GetUnderlyingRecordBodyAs<RedoRecord>();
        infile.read(reinterpret_cast<char *>(buf), size);
        if ((uint32_t)(record_body->GetTableOid()) < catalog::START_OID) {
          outfile.write(reinterpret_cast<char *>(buf), size);
//          o << "REDO " << record_body->GetDatabaseOid() << ' ' << log_record->Size()<<' '<<record_body->GetTableOid()<<std::endl;
        }
      } else if (log_record->RecordType() == LogRecordType::DELETE) {
        auto *record_body = log_record->GetUnderlyingRecordBodyAs<DeleteRecord>();
        infile.read(reinterpret_cast<char *>(buf), size);
        if ((uint32_t)(record_body->GetTableOid()) < catalog::START_OID) {
          outfile.write(reinterpret_cast<char *>(buf), size);
//          o << "DELETE " << record_body->GetDatabaseOid() << ' ' << record_body->GetTableOid() <<' '<< log_record->Size()<<std::endl;
        }
      }
    }
    delete[] buf;
  }

  infile.close();
  outfile.close();
//  o.close();

}

}  // namespace terrier::storage
