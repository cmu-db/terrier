#include "benchmark/benchmark.h"
#include "common/scoped_timer.h"
#include "execution/execution_util.h"
#include "execution/vm/module.h"
#include "main/db_main.h"
#include "test_util/tpch/workload.h"

namespace terrier::runner {
class TPCHRunner : public benchmark::Fixture {
 public:
  const int8_t total_num_threads_ = 4;                        // defines the number of terminals (workers threads)
  const uint32_t num_precomputed_txns_per_worker_ = 10;  // Number of txns to run per terminal (worker thread)
  const execution::vm::ExecutionMode mode_ = execution::vm::ExecutionMode::Interpret;

  std::unique_ptr<DBMain> db_main_;
  std::unique_ptr<tpch::Workload> tpch_workload_;

  // TPCH setup
  const std::vector<std::string> tpch_query_filenames_ = {
      "../../../tpl_tables/sample_tpl/tpch_q1.tpl",
      "../../../tpl_tables/sample_tpl/tpch_q4.tpl",
      "../../../tpl_tables/sample_tpl/tpch_q5.tpl",
      "../../../tpl_tables/sample_tpl/tpch_q6.tpl"
      };
  const std::string tpch_table_root_ = "../../../tpl_tables/tables/";
  const std::string tpch_database_name_ = "tpch_db";

  void SetUp(const benchmark::State &state) final {
    terrier::execution::ExecutionUtil::InitTPL();
    auto db_main_builder = DBMain::Builder()
                               .SetUseGC(true)
                               .SetUseCatalog(true)
                               .SetUseGCThread(true)
                               .SetUseMetrics(true)
                               .SetUseMetricsThread(true)
                               .SetBlockStoreSize(1000000)
                               .SetBlockStoreReuse(1000000)
                               .SetRecordBufferSegmentSize(1000000)
                               .SetRecordBufferSegmentReuse(1000000);
    db_main_ = db_main_builder.Build();

    auto metrics_manager = db_main_->GetMetricsManager();
    metrics_manager->EnableMetric(metrics::MetricsComponent::EXECUTION, 0);
    metrics_manager->EnableMetric(metrics::MetricsComponent::GARBAGECOLLECTION, 0);
    metrics_manager->EnableMetric(metrics::MetricsComponent::LOGGING, 0);
  }

  void TearDown(const benchmark::State &state) final {
    terrier::execution::ExecutionUtil::ShutdownTPL();
    // free db main here so we don't need to use the loggers anymore
    db_main_.reset();
  }
};

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(TPCHRunner, Runner)(benchmark::State &state) {
  // Load the TPCH tables and compile the queries
  tpch_workload_ = std::make_unique<tpch::Workload>(common::ManagedPointer<DBMain>(db_main_), tpch_database_name_,
                                                    tpch_table_root_, tpch_query_filenames_);

  for (auto num_threads = 1; num_threads <= total_num_threads_; num_threads += 2) {
    std::this_thread::sleep_for(std::chrono::seconds(2));  // Let GC clean up
    common::WorkerPool thread_pool{static_cast<uint32_t>(num_threads), {}};
    thread_pool.Startup();

    for (int8_t i = 0; i < num_threads; i++) {
      thread_pool.SubmitTask([this, i] { tpch_workload_->Execute(i, num_precomputed_txns_per_worker_, mode_); });
    }

    thread_pool.WaitUntilAllFinished();
    thread_pool.Shutdown();
  }

  // free the workload here so we don't need to use the loggers anymore
  tpch_workload_.reset();
}

BENCHMARK_REGISTER_F(TPCHRunner, Runner)->Unit(benchmark::kMillisecond)->UseManualTime()->Iterations(1);

}  // namespace terrier::runner
