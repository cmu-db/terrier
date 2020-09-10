#include "execution/sql/table_vector_iterator.h"

#include <tbb/parallel_for.h>
#include <tbb/task_arena.h>
#include <tbb/task_scheduler_init.h>

#include <limits>
#include <numeric>
#include <utility>
#include <vector>

#include "catalog/catalog_accessor.h"
#include "execution/exec/execution_context.h"
#include "execution/exec/execution_settings.h"
#include "execution/sql/storage_interface.h"
#include "execution/sql/thread_state_container.h"
#include "execution/util/timer.h"
#include "loggers/execution_logger.h"
#include "storage/index/index.h"

namespace terrier::execution::sql {

TableVectorIterator::TableVectorIterator(exec::ExecutionContext *exec_ctx, uint32_t table_oid, uint32_t *col_oids,
                                         uint32_t num_oids)
    : exec_ctx_(exec_ctx), table_oid_(table_oid), col_oids_(col_oids, col_oids + num_oids) {}

TableVectorIterator::~TableVectorIterator() = default;

bool TableVectorIterator::Init() { return Init(0, storage::DataTable::GetMaxBlocks()); }

bool TableVectorIterator::Init(uint32_t block_start, uint32_t block_end) {
  // No-op if already initialized
  if (IsInitialized()) {
    return true;
  }

  // Set up the table and the iterator.
  table_ = exec_ctx_->GetAccessor()->GetTable(table_oid_);
  TERRIER_ASSERT(table_ != nullptr, "Table must exist!!");
  if (block_start == 0 && block_end == storage::DataTable::GetMaxBlocks()) {
    iter_ = std::make_unique<storage::DataTable::SlotIterator>(table_->begin());
  } else {
    iter_ = std::make_unique<storage::DataTable::SlotIterator>(table_->GetBlockedSlotIterator(block_start, block_end));
  }
  const auto &table_col_map = table_->GetColumnMap();

  // Configure the vector projection, create the column iterators.
  std::vector<storage::col_id_t> col_ids;
  std::vector<TypeId> col_types(col_oids_.size());
  for (uint64_t idx = 0; idx < col_oids_.size(); idx++) {
    auto col_oid = col_oids_[idx];
    auto col_type = GetTypeId(table_col_map.at(col_oid).col_type_);
    auto storage_col_id = table_col_map.at(col_oid).col_id_;

    col_ids.emplace_back(storage_col_id);
    col_types[idx] = col_type;
  }

  // Create an owning vector.
  vector_projection_.SetStorageColIds(col_ids);
  vector_projection_.Initialize(col_types);
  vector_projection_.Reset(common::Constants::K_DEFAULT_VECTOR_SIZE);

  // All good.
  initialized_ = true;
  return true;
}

bool TableVectorIterator::Advance() {
  // Cannot advance if not initialized.
  if (!IsInitialized()) {
    return false;
  }

  // If the iterator is out of data, then we are done.
  if (*iter_ == table_->end() || (**iter_).GetBlock() == nullptr) {
    return false;
  }

  // Otherwise, scan the table to set the vector projection.
  table_->Scan(exec_ctx_->GetTxn(), iter_.get(), &vector_projection_);
  vector_projection_iterator_.SetVectorProjection(&vector_projection_);

  return true;
}

namespace {

class ScanTask {
 public:
  ScanTask(uint32_t table_oid, uint32_t *col_oids, uint32_t num_oids, void *const query_state,
           exec::ExecutionContext *exec_ctx, TableVectorIterator::ScanFn scanner, size_t concurrent_estimate)
      : exec_ctx_(exec_ctx),
        table_oid_(table_oid),
        col_oids_(col_oids),
        num_oids_(num_oids),
        query_state_(query_state),
        thread_state_container_(exec_ctx->GetThreadStateContainer()),
        scanner_(scanner),
        concurrent_estimate_(concurrent_estimate) {}

  void operator()(const tbb::blocked_range<uint32_t> &block_range) const {
    // Create the iterator over the specified block range
    TableVectorIterator iter{exec_ctx_, table_oid_, col_oids_, num_oids_};

    // Initialize it
    if (!iter.Init(block_range.begin(), block_range.end())) {
      return;
    }

    // Pull out the thread-local state
    byte *const thread_state = thread_state_container_->AccessCurrentThreadState();
    // Call scanning function
    scanner_(query_state_, thread_state, &iter, concurrent_estimate_);
  }

 private:
  exec::ExecutionContext *exec_ctx_;
  uint32_t table_oid_;
  uint32_t *col_oids_;
  uint32_t num_oids_;
  void *const query_state_;
  ThreadStateContainer *const thread_state_container_;
  TableVectorIterator::ScanFn scanner_ = nullptr;
  uint32_t concurrent_estimate_;
};

class CreateIndexTask {
 public:
  CreateIndexTask(uint32_t table_oid, uint32_t *col_oids, uint32_t num_oids, void *const query_state,
                  exec::ExecutionContext *exec_ctx, TableVectorIterator::CreateIndexFn create_index_fn,
                  sql::StorageInterface *storage_interface, uint32_t index_oid, uint32_t concurrent_estimate)
      : exec_ctx_(exec_ctx),
        table_oid_(table_oid),
        col_oids_(col_oids),
        num_oids_(num_oids),
        query_state_(query_state),
        thread_state_container_(exec_ctx->GetThreadStateContainer()),
        create_index_fn_(create_index_fn),
        storage_interface_(storage_interface),
        index_oid_(index_oid),
        concurrent_estimate_(concurrent_estimate) {}

  void operator()(const tbb::blocked_range<uint32_t> &block_range) const {
    // Create the iterator over the specified block range
    TableVectorIterator iter{exec_ctx_, table_oid_, col_oids_, num_oids_};

    // Initialize it
    if (!iter.Init(block_range.begin(), block_range.end())) {
      return;
    }

    // Pull out the thread-local state
    byte *const thread_state = thread_state_container_->AccessCurrentThreadState();
    // tbb::this_tbb_thread::get_id() could be used here to confirm thread id and the number of tasks
    // An index pr is allocated here to ensure it is thread local to avoid conflict
    auto curr_index = exec_ctx_->GetAccessor()->GetIndex(catalog::index_oid_t(index_oid_));
    auto index_pr_buffer = exec_ctx_->GetMemoryPool()->AllocateAligned(
        curr_index->GetProjectedRowInitializer().ProjectedRowSize(), alignof(uint64_t), false);
    auto index_pr = curr_index->GetProjectedRowInitializer().InitializeRow(index_pr_buffer);
    // calling the scan and insert function
    create_index_fn_(query_state_, thread_state, &iter, index_pr, storage_interface_, concurrent_estimate_);
    // deallocate index pr
    exec_ctx_->GetMemoryPool()->Deallocate(index_pr_buffer,
                                           curr_index->GetProjectedRowInitializer().ProjectedRowSize());
  }

 private:
  exec::ExecutionContext *exec_ctx_;
  uint32_t table_oid_;
  uint32_t *col_oids_;
  uint32_t num_oids_;
  void *const query_state_;
  ThreadStateContainer *const thread_state_container_;
  TableVectorIterator::CreateIndexFn create_index_fn_ = nullptr;
  sql::StorageInterface *storage_interface_ = nullptr;
  uint32_t index_oid_ = 0;
  uint32_t concurrent_estimate_ = 0;
};

}  // namespace

bool TableVectorIterator::ParallelScan(uint32_t table_oid, uint32_t *col_oids, uint32_t num_oids,
                                       void *const query_state, exec::ExecutionContext *exec_ctx,
                                       const TableVectorIterator::ScanFn scan_fn, const uint32_t min_grain_size) {
  // Lookup table
  const auto table = exec_ctx->GetAccessor()->GetTable(catalog::table_oid_t{table_oid});
  if (table == nullptr) {
    return false;
  }

  // Time
  util::Timer<std::milli> timer;
  timer.Start();

  // Execute parallel scan
  tbb::task_scheduler_init scan_scheduler;
  size_t num_threads = tbb::task_scheduler_init::default_num_threads();
  size_t num_tasks = (table->table_.data_table_->GetNumBlocks() / min_grain_size) + 1;
  size_t concurrent_estimate = std::min(num_threads, num_tasks);

  tbb::blocked_range<uint32_t> block_range(0, table->table_.data_table_->GetNumBlocks(), min_grain_size);
  tbb::parallel_for(block_range,
                    ScanTask(table_oid, col_oids, num_oids, query_state, exec_ctx, scan_fn, concurrent_estimate));

  timer.Stop();

  TERRIER_ASSERT(common::thread_context.metrics_store_ != nullptr, "non-null metrics store");

  double tps = table->GetNumTuple() / timer.GetElapsed() / 1000.0;
  EXECUTION_LOG_TRACE("Scanned {} blocks ({} tuples) in {} ms ({:.3f} mtps)", table->table_.data_table_->GetNumBlocks(),
                      table->GetNumTuple(), timer.GetElapsed(), tps);

  return true;
}

bool TableVectorIterator::ParallelCreateIndex(uint32_t table_oid, uint32_t *col_oids, uint32_t num_oids,
                                              void *const query_state, exec::ExecutionContext *exec_ctx,
                                              const TableVectorIterator::CreateIndexFn create_index_fn,
                                              sql::StorageInterface *storage_interface, uint32_t index_oid,
                                              execution::pipeline_id_t pipeline_id) {
  // Lookup table
  const auto table = exec_ctx->GetAccessor()->GetTable(catalog::table_oid_t{table_oid});
  if (table == nullptr) {
    return false;
  }

  // Do we need this?
  util::Timer<std::milli> timer;
  timer.Start();

  // Specify the number of thread for execution
  // TODO(wz2) Should we be using K_MIN_BLOCK_RANGE_SIZE
  size_t num_threads = exec_ctx->GetExecutionSettings().GetNumCreateIndexThreads();
  size_t num_tasks = table->table_.data_table_->GetNumBlocks();
  tbb::task_arena limited_arena(num_threads);
  tbb::blocked_range<uint32_t> block_range(0, table->table_.data_table_->GetNumBlocks());
  size_t concurrent_estimate = std::min(num_threads, num_tasks);

  // TODO(wuwenw): A static partitioner is used for the experimental purpose, may need a better way to split workload
  limited_arena.execute([&block_range, &table_oid, &col_oids, &num_oids, &query_state, &exec_ctx, &create_index_fn,
                         &storage_interface, &index_oid, concurrent_estimate] {
    tbb::parallel_for(block_range,
                      CreateIndexTask(table_oid, col_oids, num_oids, query_state, exec_ctx, create_index_fn,
                                      storage_interface, index_oid, concurrent_estimate),
                      tbb::static_partitioner());
  });

  timer.Stop();

  // Register a CREATE_INDEX_MAIN for the memory
  brain::ExecOUFeatureVector ouvec;
  auto index = exec_ctx->GetAccessor()->GetIndex(catalog::index_oid_t(index_oid));
  exec_ctx->InitializeParallelOUFeatureVector(&ouvec, pipeline_id);
  size_t tuples = index->GetSize();
  ouvec.pipeline_features_[0].SetNumRows(tuples);

  // Without more significant overhead, we actually don't have the cardinality
  // of the input tuples. As an unfortunate approximation, we just use the
  // number of tuples inserted into the index.
  ouvec.pipeline_features_[0].SetCardinality(tuples);
  ouvec.pipeline_features_[0].SetNumConcurrent(0);
  exec_ctx->SetMemoryUseOverride(index->EstimateHeapUsage());
  exec_ctx->StartPipelineTracker(pipeline_id);
  exec_ctx->EndPipelineTracker(exec_ctx->GetQueryId(), pipeline_id, &ouvec);

  double tps = table->GetNumTuple() / timer.GetElapsed() / 1000.0;
  EXECUTION_LOG_TRACE("Scanned {} blocks ({} tuples) in {} ms ({:.3f} mtps)", table->table_.data_table_->GetNumBlocks(),
                      table->GetNumTuple(), timer.GetElapsed(), tps);

  return true;
}
}  // namespace terrier::execution::sql
