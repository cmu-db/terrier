#include "execution/exec/execution_context.h"
#include "execution/exec/output.h"
#include "execution/execution_util.h"
#include "main/db_main.h"
#include "test_util/test_harness.h"
#include "test_util/tpcc/builder.h"
#include "test_util/tpcc/database.h"
#include "test_util/tpcc/loader.h"
#include "test_util/tpcc/worker.h"
#include "traffic_cop/traffic_cop_util.h"
#include "transaction/transaction_util.h"

namespace terrier {

class TpccExecutionTest : public TerrierTest {
  void SetUp() override {
    db_main_ = terrier::DBMain::Builder()
                   .SetUseGC(true)
                   .SetUseGCThread(true)
                   .SetUseCatalog(true)
                   .SetUseStatsStorage(true)
                   .SetUseExecution(true)
                   .Build();

    stats_storage_ = db_main_->GetStatsStorage();
    catalog_ = db_main_->GetCatalogLayer()->GetCatalog();
    txn_manager_ = db_main_->GetTransactionLayer()->GetTransactionManager();

    SetUpTPCC();
  }

  void TearDown() override {
    tpcc::Util::UnregisterIndexesForGC(db_main_->GetStorageLayer()->GetGarbageCollector(),
                                       common::ManagedPointer(tpcc_db_));
  }

  std::unique_ptr<DBMain> db_main_;

 public:
  std::unique_ptr<tpcc::Database> tpcc_db_;
  common::ManagedPointer<optimizer::StatsStorage> stats_storage_;
  common::ManagedPointer<catalog::Catalog> catalog_;
  common::ManagedPointer<transaction::TransactionManager> txn_manager_;
  static constexpr std::string_view db_name_ = "tpcc";

  const uint8_t num_threads_ = 1;
  const uint64_t optimizer_timeout_ = 5000;

 private:
  void SetUpTPCC() {
    tpcc::Builder tpcc_builder(db_main_->GetStorageLayer()->GetBlockStore(), catalog_, txn_manager_);
    tpcc_db_.reset(tpcc_builder.Build(storage::index::IndexType::HASHMAP));

    tpcc::Util::RegisterIndexesForGC(db_main_->GetStorageLayer()->GetGarbageCollector(),
                                     common::ManagedPointer(tpcc_db_));

    // Workers and thread pool are only used for parallel population of the database
    //    std::vector<tpcc::Worker> workers;
    //    workers.reserve(num_threads_);
    //    for (int8_t i = 0; i < num_threads_; i++) {
    //      workers.emplace_back(tpcc_db_.get());
    //    }
    //    common::WorkerPool thread_pool{num_threads_, {}};

    //    tpcc::Loader::PopulateDatabase(txn_manager_, tpcc_db_.get(), &workers, &thread_pool);
  }
};

// NOLINTNEXTLINE
TEST_F(TpccExecutionTest, SimpleTest) {
  std::string query = "SELECT * from \"NEW ORDER\"";

  auto *const txn = txn_manager_->BeginTransaction();
  const auto accessor = catalog_->GetAccessor(common::ManagedPointer(txn), tpcc_db_->db_oid_);
  auto physical_plan = trafficcop::TrafficCopUtil::ParseBindAndOptimize(
      common::ManagedPointer(txn), common::ManagedPointer(accessor), stats_storage_, query, std::string(db_name_),
      optimizer_timeout_);

  execution::exec::OutputPrinter printer{physical_plan->GetOutputSchema().Get()};

  std::cout << physical_plan->ToJson() << std::endl;

  auto exec_ctx = std::make_unique<execution::exec::ExecutionContext>(tpcc_db_->db_oid_, common::ManagedPointer(txn),
                                                                      printer, physical_plan->GetOutputSchema().Get(),
                                                                      common::ManagedPointer(accessor));

  execution::ExecutionUtil::CompileAndRun(physical_plan.get(), exec_ctx.get());

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
}

}  // namespace terrier