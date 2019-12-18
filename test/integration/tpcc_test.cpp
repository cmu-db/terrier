#include <random>
#include <string>
#include <vector>
#include "catalog/catalog.h"
#include "common/macros.h"
#include "common/scoped_timer.h"
#include "common/worker_pool.h"
#include "metrics/logging_metric.h"
#include "metrics/metrics_thread.h"
#include "storage/garbage_collector_thread.h"
#include "storage/storage_defs.h"
#include "test_util/test_harness.h"
#include "test_util/tpcc/builder.h"
#include "test_util/tpcc/database.h"
#include "test_util/tpcc/loader.h"
#include "test_util/tpcc/worker.h"
#include "test_util/tpcc/workload.h"
#include "transaction/transaction_manager.h"

namespace terrier::tpcc {

#define LOG_FILE_NAME "./tpcc.log"

/**
 * The behavior in these tests mimics that of /benchmark/integration/tpcc_benchmark.cpp. If something changes here, it
 * should probably change there as well.
 */
class TPCCTests : public TerrierTest {
 public:
  void SetUp() final {
    TerrierTest::SetUp();
    unlink(LOG_FILE_NAME);
    for (const auto &file : metrics::LoggingMetricRawData::FILES) unlink(std::string(file).c_str());
  }

  void TearDown() final {
    TerrierTest::TearDown();
    unlink(LOG_FILE_NAME);
  }

  const uint64_t blockstore_size_limit_ =
      1000;  // May need to increase this if num_threads_ or num_precomputed_txns_per_worker_ are greatly increased
  // (table sizes grow with a bigger workload)
  const uint64_t blockstore_reuse_limit_ = 1000;
  const uint64_t buffersegment_size_limit_ = 1000000;
  const uint64_t buffersegment_reuse_limit_ = 1000000;
  storage::BlockStore block_store_{blockstore_size_limit_, blockstore_reuse_limit_};
  storage::RecordBufferSegmentPool buffer_pool_{buffersegment_size_limit_, buffersegment_reuse_limit_};
  std::default_random_engine generator_;
  storage::LogManager *log_manager_ = DISABLED;  // logging enabled will override this value

  const int8_t num_threads_ = 4;  // defines the number of terminals (workers running txns) and warehouses for the
  // benchmark. Sometimes called scale factor
  const uint32_t num_precomputed_txns_per_worker_ = 10000;  // Number of txns to run per terminal (worker thread)
  TransactionWeights txn_weights_;                          // default txn_weights. See definition for values

  common::WorkerPool thread_pool_{static_cast<uint32_t>(num_threads_), {}};
  common::DedicatedThreadRegistry *thread_registry_ = nullptr;

  // Settings for log manager
  const uint64_t num_log_buffers_ = 100;
  const std::chrono::microseconds log_serialization_interval_{10};
  const std::chrono::milliseconds log_persist_interval_{20};
  const uint64_t log_persist_threshold_ = (1U << 20U);  // 1MB

  storage::GarbageCollector *gc_ = nullptr;
  storage::GarbageCollectorThread *gc_thread_ = nullptr;
  const std::chrono::milliseconds gc_period_{10};
  const std::chrono::milliseconds metrics_period_{100};

  void RunTPCC(const bool logging_enabled, const bool metrics_enabled, const storage::index::IndexType type) {
    // one TPCC worker = one TPCC terminal = one thread
    std::vector<Worker> workers;
    workers.reserve(num_threads_);

    // we need transactions, TPCC database, and GC
    metrics::MetricsThread *metrics_thread = nullptr;
    if (metrics_enabled) {
      metrics_thread = new metrics::MetricsThread(metrics_period_);
      metrics_thread->GetMetricsManager().EnableMetric(metrics::MetricsComponent::LOGGING);
      metrics_thread->GetMetricsManager().EnableMetric(metrics::MetricsComponent::TRANSACTION);
      thread_registry_ =
          new common::DedicatedThreadRegistry(common::ManagedPointer(&(metrics_thread->GetMetricsManager())));
    } else {
      thread_registry_ = new common::DedicatedThreadRegistry(DISABLED);
    }

    if (logging_enabled) {
      log_manager_ =
          new storage::LogManager(LOG_FILE_NAME, num_log_buffers_, log_serialization_interval_, log_persist_interval_,
                                  log_persist_threshold_, &buffer_pool_, common::ManagedPointer(thread_registry_));
      log_manager_->Start();
    }

    transaction::TimestampManager timestamp_manager;
    transaction::DeferredActionManager deferred_action_manager(&timestamp_manager);
    transaction::TransactionManager txn_manager(&timestamp_manager, &deferred_action_manager, &buffer_pool_, true,
                                                log_manager_);
    catalog::Catalog catalog(&txn_manager, &block_store_);
    Builder tpcc_builder(&block_store_, &catalog, &txn_manager);

    // Precompute all of the input arguments for every txn to be run. We want to avoid the overhead at benchmark time
    const auto precomputed_args =
        PrecomputeArgs(&generator_, txn_weights_, num_threads_, num_precomputed_txns_per_worker_);

    // build the TPCC database
    auto *const tpcc_db = tpcc_builder.Build(type);

    // prepare the workers
    workers.clear();
    for (int8_t i = 0; i < num_threads_; i++) {
      workers.emplace_back(tpcc_db);
    }

    // populate the tables and indexes, as well as force log manager to log all changes
    Loader::PopulateDatabase(&txn_manager, tpcc_db, &workers, &thread_pool_);
    if (logging_enabled) log_manager_->ForceFlush();

    gc_ = new storage::GarbageCollector(&timestamp_manager, &deferred_action_manager, &txn_manager, DISABLED);
    gc_thread_ = new storage::GarbageCollectorThread(gc_, gc_period_, nullptr);
    Util::RegisterIndexesForGC(&(gc_thread_->GetGarbageCollector()), tpcc_db);
    std::this_thread::sleep_for(std::chrono::seconds(2));  // Let GC clean up

    // run the TPCC workload to completion
    for (int8_t i = 0; i < num_threads_; i++) {
      thread_pool_.SubmitTask([i, tpcc_db, &txn_manager, precomputed_args, &workers] {
        Workload(i, tpcc_db, &txn_manager, precomputed_args, &workers);
      });
    }
    thread_pool_.WaitUntilAllFinished();

    // cleanup
    Util::UnregisterIndexesForGC(&(gc_thread_->GetGarbageCollector()), tpcc_db);
    delete gc_thread_;
    catalog.TearDown();
    deferred_action_manager.FullyPerformGC(gc_, log_manager_);
    thread_pool_.Shutdown();
    if (logging_enabled) {
      log_manager_->PersistAndStop();
      delete log_manager_;
    }
    delete gc_;
    delete thread_registry_;
    if (metrics_enabled) {
      delete metrics_thread;
    }
    delete tpcc_db;

    CleanUpVarlensInPrecomputedArgs(&precomputed_args);
  }
};

// NOLINTNEXTLINE
TEST_F(TPCCTests, WithoutLoggingBwTreeIndexes) { RunTPCC(false, false, storage::index::IndexType::BWTREE); }

// NOLINTNEXTLINE
TEST_F(TPCCTests, WithoutLoggingHashIndexes) { RunTPCC(false, false, storage::index::IndexType::HASHMAP); }

// NOLINTNEXTLINE
TEST_F(TPCCTests, WithLogging) { RunTPCC(true, false, storage::index::IndexType::HASHMAP); }

// NOLINTNEXTLINE
TEST_F(TPCCTests, WithLoggingAndMetrics) { RunTPCC(true, true, storage::index::IndexType::HASHMAP); }

}  // namespace terrier::tpcc
