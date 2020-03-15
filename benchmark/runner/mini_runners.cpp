#include <common/macros.h>
#include <random>
#include <utility>

#include "benchmark/benchmark.h"
#include "benchmark_util/benchmark_config.h"
#include "brain/brain_defs.h"
#include "brain/operating_unit.h"
#include "common/scoped_timer.h"
#include "execution/executable_query.h"
#include "execution/execution_util.h"
#include "execution/table_generator/table_generator.h"
#include "execution/util/cpu_info.h"
#include "execution/vm/module.h"
#include "loggers/loggers_util.h"
#include "main/db_main.h"
#include "optimizer/cost_model/forced_cost_model.h"
#include "optimizer/cost_model/trivial_cost_model.h"
#include "optimizer/optimizer.h"
#include "optimizer/properties.h"
#include "optimizer/query_to_operator_transformer.h"

namespace terrier::runner {

/**
 * Static db_main instance
 * This is done so all tests reuse the same DB Main instance
 */
DBMain *db_main = nullptr;

/**
 * Database OID
 * This is done so all tests can use the same database OID
 */
catalog::db_oid_t db_oid{0};

/**
 * Taken from Facebook's folly library.
 * DoNotOptimizeAway helps to ensure that a certain variable
 * is not optimized away by the compiler.
 */
template <typename T>
struct DoNotOptimizeAwayNeedsIndirect {
  using Decayed = typename std::decay<T>::type;

  // First two constraints ensure it can be an "r" operand.
  // std::is_pointer check is because callers seem to expect that
  // doNotOptimizeAway(&x) is equivalent to doNotOptimizeAway(x).
  constexpr static bool VALUE = !std::is_trivially_copyable<Decayed>::value ||
                                sizeof(Decayed) > sizeof(int64_t) || std::is_pointer<Decayed>::value;
};

template <typename T>
auto DoNotOptimizeAway(const T &datum) -> typename std::enable_if<!DoNotOptimizeAwayNeedsIndirect<T>::VALUE>::type {
  // The "r" constraint forces the compiler to make datum available
  // in a register to the asm block, which means that it must have
  // computed/loaded it.  We use this path for things that are <=
  // sizeof(long) (they have to fit), trivial (otherwise the compiler
  // doesn't want to put them in a register), and not a pointer (because
  // DoNotOptimizeAway(&foo) would otherwise be a foot gun that didn't
  // necessarily compute foo).
  //
  // An earlier version of this method had a more permissive input operand
  // constraint, but that caused unnecessary variation between clang and
  // gcc benchmarks.
  asm volatile("" ::"r"(datum));
}

/**
 * To prevent the iterator op i from being optimized out on
 * clang/gcc compilers, gcc requires noinline and noclone
 * to prevent any inlining. noclone is used to prevent gcc
 * from doing interprocedural constant propagation.
 *
 * DoNotOptimizeAway is placed inside the for () loop to
 * ensure that the compiler does not blindly optimize away
 * the for loop.
 *
 * TIGHT_LOOP_OPERATION(uint32_t, PLUS, +)
 * Defines a function called uint32_t_PLUS that adds integers.
 */
#ifdef __clang__
#define TIGHT_LOOP_OPERATION(type, name, op)                       \
  __attribute__((noinline)) type __##type##_##name(size_t count) { \
    type iterator = 1;                                             \
    for (size_t i = 1; i <= count; i++) {                          \
      iterator = iterator op i;                                    \
      DoNotOptimizeAway(i);                                        \
      DoNotOptimizeAway(iterator);                                 \
    }                                                              \
                                                                   \
    return iterator;                                               \
  }

#elif __GNUC__
#define TIGHT_LOOP_OPERATION(type, name, op)                                \
  __attribute__((noinline, noclone)) type __##type##_##name(size_t count) { \
    type iterator = 1;                                                      \
    for (size_t i = 1; i <= count; i++) {                                   \
      iterator = iterator op i;                                             \
      DoNotOptimizeAway(i);                                                 \
      DoNotOptimizeAway(iterator);                                          \
    }                                                                       \
                                                                            \
    return iterator;                                                        \
  }

#endif

/**
 * Arg <0, 1, 2>
 * 0 - Number of columns
 * 1 - Number of rows
 * 2 - Cardinality
 */
static void GenScanArguments(benchmark::internal::Benchmark *b) {
  auto num_cols = {1, 4, 8, 15};
  auto num_rows = {
      std::pair<int, int>{10, 1},           std::pair<int, int>{10, 2},          std::pair<int, int>{10, 8},
      std::pair<int, int>{100, 16},         std::pair<int, int>{100, 32},        std::pair<int, int>{100, 64},
      std::pair<int, int>{10000, 128},      std::pair<int, int>{10000, 1024},    std::pair<int, int>{10000, 4096},
      std::pair<int, int>{10000, 8192},     std::pair<int, int>{1000000, 1024},  std::pair<int, int>{1000000, 16384},
      std::pair<int, int>{1000000, 131072}, std::pair<int, int>{1000000, 524288}};

  for (auto col : num_cols) {
    for (auto row : num_rows) {
      b->Args({col, row.first, row.second});
    }
  }
}

/**
 * Arg <0, 1, 2>
 * 0 - Number of columns
 * 1 - Number of rows
 * 2 - Cardinality
 */
static void GenNJArguments(benchmark::internal::Benchmark *b) {
  auto num_cols = {1, 4, 8, 15};
  auto num_rows = {std::pair<int, int>{10, 1},       std::pair<int, int>{10, 2},       std::pair<int, int>{10, 8},
                   std::pair<int, int>{100, 16},     std::pair<int, int>{100, 32},     std::pair<int, int>{100, 64},
                   std::pair<int, int>{10000, 128},  std::pair<int, int>{10000, 1024}, std::pair<int, int>{10000, 4096},
                   std::pair<int, int>{10000, 8192}, std::pair<int, int>{20000, 1024}, std::pair<int, int>{20000, 4096},
                   std::pair<int, int>{20000, 8192}, std::pair<int, int>{20000, 16384}};

  for (auto col : num_cols) {
    for (auto row : num_rows) {
      b->Args({col, row.first, row.second});
    }
  }
}

/**
 * Arg <0, 1>
 * 0 - Number of columns
 * 0 - Number of columns
 * 1 - Number of rows
 */
static void GenInsertArguments(benchmark::internal::Benchmark *b) {
  auto num_cols = {1, 4, 8, 15};
  auto num_rows = {1, 100, 10000, 100000};
  for (auto col : num_cols) {
    for (auto row : num_rows) {
      b->Args({col, row});
    }
  }
}

class MiniRunners : public benchmark::Fixture {
 public:
  static constexpr execution::query_id_t OP_INTEGER_PLUS_QID = execution::query_id_t(1);
  static constexpr execution::query_id_t OP_INTEGER_MULTIPLY_QID = execution::query_id_t(2);
  static constexpr execution::query_id_t OP_INTEGER_DIVIDE_QID = execution::query_id_t(3);
  static constexpr execution::query_id_t OP_INTEGER_GEQ_QID = execution::query_id_t(4);
  static constexpr execution::query_id_t OP_DECIMAL_PLUS_QID = execution::query_id_t(5);
  static constexpr execution::query_id_t OP_DECIMAL_MULTIPLY_QID = execution::query_id_t(6);
  static constexpr execution::query_id_t OP_DECIMAL_DIVIDE_QID = execution::query_id_t(7);
  static constexpr execution::query_id_t OP_DECIMAL_GEQ_QID = execution::query_id_t(8);

  static constexpr execution::query_id_t SEQ_SCAN_COL_1_QID = execution::query_id_t(9);
  static constexpr execution::query_id_t SEQ_SCAN_COL_4_QID = execution::query_id_t(10);
  static constexpr execution::query_id_t SEQ_SCAN_COL_8_QID = execution::query_id_t(11);
  static constexpr execution::query_id_t SEQ_SCAN_COL_15_QID = execution::query_id_t(12);

  static constexpr execution::query_id_t BULK_INS_COL_1_QID = execution::query_id_t(13);
  static constexpr execution::query_id_t BULK_INS_COL_4_QID = execution::query_id_t(14);
  static constexpr execution::query_id_t BULK_INS_COL_8_QID = execution::query_id_t(15);
  static constexpr execution::query_id_t BULK_INS_COL_15_QID = execution::query_id_t(16);

  static constexpr execution::query_id_t SORT_COL_1_QID = execution::query_id_t(17);
  static constexpr execution::query_id_t SORT_COL_4_QID = execution::query_id_t(18);
  static constexpr execution::query_id_t SORT_COL_8_QID = execution::query_id_t(19);
  static constexpr execution::query_id_t SORT_COL_15_QID = execution::query_id_t(20);

  static constexpr execution::query_id_t UPDATE_COL_1_QID = execution::query_id_t(21);
  static constexpr execution::query_id_t UPDATE_COL_4_QID = execution::query_id_t(22);
  static constexpr execution::query_id_t UPDATE_COL_8_QID = execution::query_id_t(23);
  static constexpr execution::query_id_t UPDATE_COL_15_QID = execution::query_id_t(24);

  static constexpr execution::query_id_t AGG_COL_1_QID = execution::query_id_t(25);
  static constexpr execution::query_id_t AGG_COL_4_QID = execution::query_id_t(26);
  static constexpr execution::query_id_t AGG_COL_8_QID = execution::query_id_t(27);
  static constexpr execution::query_id_t AGG_COL_15_QID = execution::query_id_t(28);

  static constexpr execution::query_id_t HJ_COL_1_QID = execution::query_id_t(29);
  static constexpr execution::query_id_t HJ_COL_4_QID = execution::query_id_t(30);
  static constexpr execution::query_id_t HJ_COL_8_QID = execution::query_id_t(31);
  static constexpr execution::query_id_t HJ_COL_15_QID = execution::query_id_t(32);

  static constexpr execution::query_id_t NLJ_COL_1_QID = execution::query_id_t(33);
  static constexpr execution::query_id_t NLJ_COL_4_QID = execution::query_id_t(34);
  static constexpr execution::query_id_t NLJ_COL_8_QID = execution::query_id_t(35);
  static constexpr execution::query_id_t NLJ_COL_15_QID = execution::query_id_t(36);

  static constexpr execution::query_id_t IDX_KEY_1_QID = execution::query_id_t(37);
  static constexpr execution::query_id_t IDX_KEY_4_QID = execution::query_id_t(38);
  static constexpr execution::query_id_t IDX_KEY_8_QID = execution::query_id_t(39);
  static constexpr execution::query_id_t IDX_KEY_15_QID = execution::query_id_t(40);

  const uint64_t optimizer_timeout_ = 1000000;
  const execution::vm::ExecutionMode mode_ = execution::vm::ExecutionMode::Interpret;

  TIGHT_LOOP_OPERATION(uint32_t, PLUS, +);
  TIGHT_LOOP_OPERATION(uint32_t, MULTIPLY, *);
  TIGHT_LOOP_OPERATION(uint32_t, DIVIDE, /);
  TIGHT_LOOP_OPERATION(uint32_t, GEQ, >=);
  TIGHT_LOOP_OPERATION(double, PLUS, +);
  TIGHT_LOOP_OPERATION(double, MULTIPLY, *);
  TIGHT_LOOP_OPERATION(double, DIVIDE, /);
  TIGHT_LOOP_OPERATION(double, GEQ, >=);

  void SetUp(const benchmark::State &state) final {
    catalog_ = db_main->GetCatalogLayer()->GetCatalog();
    txn_manager_ = db_main->GetTransactionLayer()->GetTransactionManager();
    metrics_manager_ = db_main->GetMetricsManager();
  }

  void BenchmarkSqlStatement(execution::query_id_t qid, const std::string &query, brain::PipelineOperatingUnits *units,
                             std::unique_ptr<optimizer::AbstractCostModel> cost_model, bool commit) {
    auto txn = txn_manager_->BeginTransaction();
    auto stmt_list = parser::PostgresParser::BuildParseTree(query);

    auto accessor = catalog_->GetAccessor(common::ManagedPointer(txn), db_oid);
    auto binder = binder::BindNodeVisitor(common::ManagedPointer(accessor), "test_db");
    binder.BindNameToNode(stmt_list->GetStatement(0), stmt_list.get());

    auto transformer = optimizer::QueryToOperatorTransformer(common::ManagedPointer(accessor));
    auto plan = transformer.ConvertToOpExpression(stmt_list->GetStatement(0), stmt_list.get());

    std::unique_ptr<planner::AbstractPlanNode> out_plan;
    auto optimizer = optimizer::Optimizer(std::move(cost_model), optimizer_timeout_);
    if (stmt_list->GetStatement(0)->GetType() == parser::StatementType::SELECT) {
      auto sel_stmt = stmt_list->GetStatement(0).CastManagedPointerTo<parser::SelectStatement>();
      auto output = sel_stmt->GetSelectColumns();
      auto property_set = optimizer::PropertySet();

      if (sel_stmt->GetSelectOrderBy()) {
        std::vector<optimizer::OrderByOrderingType> sort_dirs;
        std::vector<common::ManagedPointer<parser::AbstractExpression>> sort_exprs;

        auto order_by = sel_stmt->GetSelectOrderBy();
        auto types = order_by->GetOrderByTypes();
        auto exprs = order_by->GetOrderByExpressions();
        for (size_t idx = 0; idx < order_by->GetOrderByExpressionsSize(); idx++) {
          sort_exprs.emplace_back(exprs[idx]);
          sort_dirs.push_back(types[idx] == parser::OrderType::kOrderAsc ? optimizer::OrderByOrderingType::ASC
                                                                         : optimizer::OrderByOrderingType::DESC);
        }

        auto sort_prop = new optimizer::PropertySort(sort_exprs, sort_dirs);
        property_set.AddProperty(sort_prop);
      }

      auto query_info = optimizer::QueryInfo(parser::StatementType::SELECT, std::move(output), &property_set);
      out_plan =
          optimizer.BuildPlanTree(txn, accessor.get(), db_main->GetStatsStorage().Get(), query_info, std::move(plan));
    } else {
      auto property_set = optimizer::PropertySet();
      auto query_info = optimizer::QueryInfo(stmt_list->GetStatement(0)->GetType(), {}, &property_set);
      out_plan =
          optimizer.BuildPlanTree(txn, accessor.get(), db_main->GetStatsStorage().Get(), query_info, std::move(plan));
    }

    execution::ExecutableQuery::query_identifier.store(qid);
    auto exec_ctx = std::make_unique<execution::exec::ExecutionContext>(
        db_oid, common::ManagedPointer(txn), execution::exec::NoOpResultConsumer(), out_plan->GetOutputSchema().Get(),
        common::ManagedPointer(accessor));
    auto exec_query = execution::ExecutableQuery(common::ManagedPointer(out_plan), common::ManagedPointer(exec_ctx));
    exec_ctx->SetPipelineOperatingUnits(common::ManagedPointer(units));
    exec_query.Run(common::ManagedPointer(exec_ctx), mode_);

    if (commit)
      txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
    else
      txn_manager_->Abort(txn);
  }

  void BenchmarkIndexScan(int key_num, int num_rows, int lookup_size) {
    auto txn = txn_manager_->BeginTransaction();
    auto accessor = catalog_->GetAccessor(common::ManagedPointer(txn), db_oid);
    auto exec_ctx = std::make_unique<execution::exec::ExecutionContext>(db_oid, common::ManagedPointer(txn), nullptr,
                                                                        nullptr, common::ManagedPointer(accessor));
    exec_ctx->SetExecutionMode(static_cast<uint8_t>(mode_));

    execution::query_id_t qid;
    switch (key_num) {
      case 1:
        qid = IDX_KEY_1_QID;
        break;
      case 4:
        qid = IDX_KEY_4_QID;
        break;
      case 8:
        qid = IDX_KEY_8_QID;
        break;
      case 15:
        qid = IDX_KEY_15_QID;
        break;
      default:
        UNREACHABLE("Invalid key_size for index scan");
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    exec_ctx->SetPipelineOperatingUnits(common::ManagedPointer(&units));
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::IDX_SCAN, num_rows, 4 * key_num, key_num, lookup_size);
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));

    std::stringstream table_name;
    table_name << "INTEGERIdxKey" << key_num << "Row" << num_rows;
    auto tbl_oid = accessor->GetTableOid(table_name.str());
    auto indexes = accessor->GetIndexes(tbl_oid);
    auto index = indexes[0].first;

    exec_ctx->StartResourceTracker(metrics::MetricsComponent::EXECUTION_PIPELINE);

    std::vector<storage::TupleSlot> results;
    if (lookup_size == 1) {
      auto *key_buffer =
          common::AllocationUtil::AllocateAligned(index->GetProjectedRowInitializer().ProjectedRowSize());
      auto *const scan_key_pr = index->GetProjectedRowInitializer().InitializeRow(key_buffer);

      std::default_random_engine generator;
      const uint32_t random_key =
          std::uniform_int_distribution(static_cast<uint32_t>(0), static_cast<uint32_t>(num_rows - 1))(generator);
      for (int i = 0; i < key_num; i++) {
        *reinterpret_cast<uint32_t *>(scan_key_pr->AccessForceNotNull(i)) = random_key;
      }

      index->ScanKey(*txn, *scan_key_pr, &results);
      results.clear();
      delete[] key_buffer;
    } else {
      uint32_t high_key;
      uint32_t low_key;
      auto *high_buffer =
          common::AllocationUtil::AllocateAligned(index->GetProjectedRowInitializer().ProjectedRowSize());
      auto *low_buffer =
          common::AllocationUtil::AllocateAligned(index->GetProjectedRowInitializer().ProjectedRowSize());

      std::default_random_engine generator;
      low_key = std::uniform_int_distribution(static_cast<uint32_t>(0),
                                              static_cast<uint32_t>(num_rows - lookup_size))(generator);
      high_key = low_key + lookup_size - 1;
      auto *const low_key_pr = index->GetProjectedRowInitializer().InitializeRow(low_buffer);
      auto *const high_key_pr = index->GetProjectedRowInitializer().InitializeRow(high_buffer);
      for (int i = 0; i < key_num; i++) {
        *reinterpret_cast<uint32_t *>(low_key_pr->AccessForceNotNull(i)) = low_key;
        *reinterpret_cast<uint32_t *>(high_key_pr->AccessForceNotNull(i)) = high_key;
      }

      index->ScanAscending(*txn, storage::index::ScanType::Closed, key_num, low_key_pr, high_key_pr, 0, &results);
      results.clear();
      delete[] low_buffer;
      delete[] high_buffer;
    }

    exec_ctx->EndPipelineTracker(OP_INTEGER_MULTIPLY_QID, execution::pipeline_id_t(0));

    txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  }

  void BenchmarkArithmetic(brain::ExecutionOperatingUnitType type, size_t num_elem) {
    auto txn = txn_manager_->BeginTransaction();
    auto accessor = catalog_->GetAccessor(common::ManagedPointer(txn), db_oid);
    auto exec_ctx = std::make_unique<execution::exec::ExecutionContext>(db_oid, common::ManagedPointer(txn), nullptr,
                                                                        nullptr, common::ManagedPointer(accessor));
    exec_ctx->SetExecutionMode(static_cast<uint8_t>(mode_));
    exec_ctx->StartResourceTracker(metrics::MetricsComponent::EXECUTION_PIPELINE);

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    exec_ctx->SetPipelineOperatingUnits(common::ManagedPointer(&units));
    pipe0_vec.emplace_back(type, num_elem, 4, 1, num_elem);
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));

    switch (type) {
      case brain::ExecutionOperatingUnitType::OP_INTEGER_PLUS_OR_MINUS: {
        uint32_t ret = __uint32_t_PLUS(num_elem);
        exec_ctx->EndPipelineTracker(OP_INTEGER_PLUS_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_INTEGER_MULTIPLY: {
        uint32_t ret = __uint32_t_MULTIPLY(num_elem);
        exec_ctx->EndPipelineTracker(OP_INTEGER_MULTIPLY_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_INTEGER_DIVIDE: {
        uint32_t ret = __uint32_t_DIVIDE(num_elem);
        exec_ctx->EndPipelineTracker(OP_INTEGER_DIVIDE_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_INTEGER_COMPARE: {
        uint32_t ret = __uint32_t_GEQ(num_elem);
        exec_ctx->EndPipelineTracker(OP_INTEGER_GEQ_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_DECIMAL_PLUS_OR_MINUS: {
        double ret = __double_PLUS(num_elem);
        exec_ctx->EndPipelineTracker(OP_DECIMAL_PLUS_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_DECIMAL_MULTIPLY: {
        double ret = __double_MULTIPLY(num_elem);
        exec_ctx->EndPipelineTracker(OP_DECIMAL_MULTIPLY_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_DECIMAL_DIVIDE: {
        double ret = __double_DIVIDE(num_elem);
        exec_ctx->EndPipelineTracker(OP_DECIMAL_DIVIDE_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      case brain::ExecutionOperatingUnitType::OP_DECIMAL_COMPARE: {
        double ret = __double_GEQ(num_elem);
        exec_ctx->EndPipelineTracker(OP_DECIMAL_GEQ_QID, execution::pipeline_id_t(0));
        DoNotOptimizeAway(ret);
        break;
      }
      default:
        UNREACHABLE("Unsupported ExecutionOperatingUnitType");
        break;
    }

    txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  }

 protected:
  common::ManagedPointer<catalog::Catalog> catalog_;
  common::ManagedPointer<transaction::TransactionManager> txn_manager_;
  common::ManagedPointer<metrics::MetricsManager> metrics_manager_;
};

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, InsertRunners)(benchmark::State &state) {
  auto num_cols = state.range(0);
  auto num_rows = state.range(1);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    execution::query_id_t qid;
    switch (num_cols) {
      case 1:
        qid = BULK_INS_COL_1_QID;
        break;
      case 4:
        qid = BULK_INS_COL_4_QID;
        break;
      case 8:
        qid = BULK_INS_COL_8_QID;
        break;
      case 15:
        qid = BULK_INS_COL_15_QID;
        break;
      default:
        UNREACHABLE("Unexpected number of columns for BulkInsert");
        break;
    }

    // Create temporary table schema
    std::vector<catalog::Schema::Column> cols;
    for (uint32_t j = 1; j <= num_cols; j++) {
      std::stringstream col_name;
      col_name << "col" << j;
      cols.emplace_back(col_name.str(), type::TypeId::INTEGER, false,
                        terrier::parser::ConstantValueExpression(type::TransientValueFactory::GetInteger(0)));
    }
    catalog::Schema tmp_schema(cols);

    // Create table
    catalog::table_oid_t tbl_oid;
    {
      auto txn = txn_manager_->BeginTransaction();
      auto accessor = catalog_->GetAccessor(common::ManagedPointer(txn), db_oid);
      tbl_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "tmp_table", tmp_schema);
      auto &schema = accessor->GetSchema(tbl_oid);
      auto *tmp_table = new storage::SqlTable(db_main->GetStorageLayer()->GetBlockStore(), schema);
      accessor->SetTablePointer(tbl_oid, tmp_table);
      txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
    }

    std::stringstream query;
    query << "INSERT INTO tmp_table VALUES ";

    std::mt19937 generator{};
    std::uniform_int_distribution<int> distribution(0, INT_MAX);
    for (uint32_t idx = 0; idx < num_rows; idx++) {
      query << "(";
      for (uint32_t i = 1; i <= num_cols; i++) {
        query << distribution(generator);
        if (i != num_cols) {
          query << ",";
        } else {
          query << ") ";
        }
      }

      if (idx != num_rows - 1) {
        query << ", ";
      }
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::INSERT, num_rows, 4 * num_cols, num_cols,
                           static_cast<double>(num_rows));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));

    uint64_t elapsed_ms;
    {
      common::ScopedTimer<std::chrono::milliseconds> timer(&elapsed_ms);
      metrics_manager_->RegisterThread();
      BenchmarkSqlStatement(qid, query.str(), &units, std::make_unique<optimizer::TrivialCostModel>(), true);
      metrics_manager_->Aggregate();
      metrics_manager_->UnregisterThread();
    }

    // Drop the table
    {
      auto txn = txn_manager_->BeginTransaction();
      auto accessor = catalog_->GetAccessor(common::ManagedPointer(txn), db_oid);
      accessor->DropTable(tbl_oid);
      txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
    }

    state.SetIterationTime(static_cast<double>(elapsed_ms) / 1000.0);

    auto gc = db_main->GetStorageLayer()->GetGarbageCollector();
    gc->PerformGarbageCollection();
    gc->PerformGarbageCollection();
  }

  state.SetItemsProcessed(num_rows);
}

/**
 * Arg: <0, 1>
 * 0 - Number of columns
 * 1 - Number of rows to insert
 */
BENCHMARK_REGISTER_F(MiniRunners, InsertRunners)
    ->Unit(benchmark::kMillisecond)
    ->UseManualTime()
    ->Iterations(1)
    ->Apply(GenInsertArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, UpdateRunners)(benchmark::State &state) {
  auto num_cols = state.range(0);
  auto num_rows = state.range(1);

  execution::query_id_t qid;
  switch (num_cols) {
    case 1:
      qid = UPDATE_COL_1_QID;
      break;
    case 4:
      qid = UPDATE_COL_4_QID;
      break;
    case 8:
      qid = UPDATE_COL_8_QID;
      break;
    case 15:
      qid = UPDATE_COL_15_QID;
      break;
    default:
      UNREACHABLE("Unexpected number of columns for UpdateRunners");
      break;
  }

  // NOLINTNEXTLINE
  for (auto _ : state) {
    // Create temporary table schema
    std::stringstream query;
    query << "UPDATE "
          << "INTEGERCol15Row" << num_rows << "Car1 SET ";
    std::vector<catalog::Schema::Column> cols;
    std::mt19937 generator{};
    std::uniform_int_distribution<int> distribution(0, INT_MAX);
    for (uint32_t j = 1; j <= num_cols; j++) {
      query << "col" << j << " = " << distribution(generator);
      if (j != num_cols) {
        query << ", ";
      }
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::UPDATE, num_rows, 4 * num_cols, num_cols,
                           static_cast<double>(num_rows));
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, num_rows, 15 * 4, 15,
                           static_cast<double>(num_rows));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));

    uint64_t elapsed_ms;
    {
      common::ScopedTimer<std::chrono::milliseconds> timer(&elapsed_ms);
      metrics_manager_->RegisterThread();
      BenchmarkSqlStatement(qid, query.str(), &units, std::make_unique<optimizer::TrivialCostModel>(), false);
      metrics_manager_->Aggregate();
      metrics_manager_->UnregisterThread();
    }

    state.SetIterationTime(static_cast<double>(elapsed_ms) / 1000.0);
  }

  state.SetItemsProcessed(num_rows);
}

/**
 * Arg: <0, 1>
 * 0 - Number of columns to update
 * 1 - Row number
 */
BENCHMARK_REGISTER_F(MiniRunners, UpdateRunners)
    ->Unit(benchmark::kMillisecond)
    ->UseManualTime()
    ->Iterations(1)
    ->Apply(GenInsertArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, SeqScanRunners)(benchmark::State &state) {
  auto num_col = state.range(0);
  auto row = state.range(1);
  auto car = state.range(2);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();

    execution::query_id_t qid;
    switch (num_col) {
      case 1:
        qid = SEQ_SCAN_COL_1_QID;
        break;
      case 4:
        qid = SEQ_SCAN_COL_4_QID;
        break;
      case 8:
        qid = SEQ_SCAN_COL_8_QID;
        break;
      case 15:
        qid = SEQ_SCAN_COL_15_QID;
        break;
      default:
        UNREACHABLE("Unexpected number of columns for SeqScan");
        break;
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));

    std::stringstream cols;
    for (auto i = 1; i <= num_col; i++) {
      cols << "col" << i;
      if (i != num_col) {
        cols << ", ";
      }
    }

    std::stringstream tbl_name;
    tbl_name << "SELECT " << (cols.str()) << " FROM INTEGERCol15Row" << row << "Car" << car;
    BenchmarkSqlStatement(qid, tbl_name.str(), &units, std::make_unique<optimizer::TrivialCostModel>(), true);
    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(row);
}

/**
 * Arg: <0, 1, 2>
 * 0 - Number of columns to select
 * 1 - Number of rows
 * 2 - Cardinality
 */
BENCHMARK_REGISTER_F(MiniRunners, SeqScanRunners)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Apply(GenScanArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, SortRunners)(benchmark::State &state) {
  auto num_col = state.range(0);
  auto row = state.range(1);
  auto car = state.range(2);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();

    execution::query_id_t qid;
    switch (num_col) {
      case 1:
        qid = SORT_COL_1_QID;
        break;
      case 4:
        qid = SORT_COL_4_QID;
        break;
      case 8:
        qid = SORT_COL_8_QID;
        break;
      case 15:
        qid = SORT_COL_15_QID;
        break;
      default:
        UNREACHABLE("Unexpected number of columns for Sort");
        break;
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    brain::ExecutionOperatingUnitFeatureVector pipe1_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SORT_BUILD, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe1_vec.emplace_back(brain::ExecutionOperatingUnitType::SORT_ITERATE, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));
    units.RecordOperatingUnit(execution::pipeline_id_t(1), std::move(pipe1_vec));

    std::stringstream cols;
    for (auto i = 1; i <= num_col; i++) {
      cols << "col" << i;
      if (i != num_col) {
        cols << ", ";
      }
    }

    std::stringstream tbl_name;
    tbl_name << "SELECT " << (cols.str()) << " FROM INTEGERCol15Row" << row << "Car" << car << " ORDER BY "
             << (cols.str());
    BenchmarkSqlStatement(qid, tbl_name.str(), &units, std::make_unique<optimizer::TrivialCostModel>(), true);
    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(row);
}

/**
 * Arg: <0, 1, 2>
 * 0 - Number of columns to select
 * 1 - Number of rows
 * 2 - Cardinality
 */
BENCHMARK_REGISTER_F(MiniRunners, SortRunners)->Unit(benchmark::kMillisecond)->Iterations(1)->Apply(GenScanArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, HashJoinRunners)(benchmark::State &state) {
  auto num_col = state.range(0);
  auto row = state.range(1);
  auto car = state.range(2);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();

    execution::query_id_t qid;
    switch (num_col) {
      case 1:
        qid = HJ_COL_1_QID;
        break;
      case 4:
        qid = HJ_COL_4_QID;
        break;
      case 8:
        qid = HJ_COL_8_QID;
        break;
      case 15:
        qid = HJ_COL_15_QID;
        break;
      default:
        UNREACHABLE("Unexpected number of columns for HJ");
        break;
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    brain::ExecutionOperatingUnitFeatureVector pipe1_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::HASHJOIN_BUILD, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe1_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe1_vec.emplace_back(brain::ExecutionOperatingUnitType::HASHJOIN_PROBE, row, 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe1_vec.emplace_back(brain::ExecutionOperatingUnitType::OP_INTEGER_COMPARE, row, 4, 1, static_cast<double>(car));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));
    units.RecordOperatingUnit(execution::pipeline_id_t(1), std::move(pipe1_vec));

    std::stringstream query;
    query << "SELECT ";
    for (auto i = 1; i <= num_col; i++) {
      query << "b.col" << i;
      if (i != num_col) {
        query << ", ";
      }
    }

    std::stringstream tbl_name;
    tbl_name << "INTEGERCol15Row" << row << "Car" << car;
    query << " FROM " << tbl_name.str() << ", " << tbl_name.str() << " as b WHERE ";
    for (auto i = 1; i <= num_col; i++) {
      query << (tbl_name.str()) << ".col" << i << " = b.col" << i;
      if (i != num_col) {
        query << " AND ";
      }
    }

    BenchmarkSqlStatement(qid, query.str(), &units, std::make_unique<optimizer::ForcedCostModel>(true), true);
    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(row);
}

/**
 * Arg: <0, 1, 2>
 * 0 - Number of columns to select
 * 1 - Number of rows
 * 2 - Cardinality
 */
BENCHMARK_REGISTER_F(MiniRunners, HashJoinRunners)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Apply(GenScanArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, NestedLoopJoinRunners)(benchmark::State &state) {
  auto num_col = state.range(0);
  auto row = state.range(1);
  auto car = state.range(2);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();

    execution::query_id_t qid;
    switch (num_col) {
      case 1:
        qid = NLJ_COL_1_QID;
        break;
      case 4:
        qid = NLJ_COL_4_QID;
        break;
      case 8:
        qid = NLJ_COL_8_QID;
        break;
      case 15:
        qid = NLJ_COL_15_QID;
        break;
      default:
        UNREACHABLE("Unexpected number of columns for HJ");
        break;
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, row * (row + 1), 4 * num_col, num_col,
                           static_cast<double>(car));
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::OP_INTEGER_COMPARE, row * (row + 1), 4 * num_col, num_col,
                           static_cast<double>(car));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));

    std::stringstream query;
    query << "SELECT ";
    for (auto i = 1; i <= num_col; i++) {
      query << "b.col" << i;
      if (i != num_col) {
        query << ", ";
      }
    }

    std::stringstream tbl_name;
    tbl_name << "INTEGERCol15Row" << row << "Car" << car;
    query << " FROM " << tbl_name.str() << ", " << tbl_name.str() << " as b WHERE ";
    for (auto i = 1; i <= num_col; i++) {
      query << (tbl_name.str()) << ".col" << i << " = b.col" << i;
      if (i != num_col) {
        query << " AND ";
      }
    }

    BenchmarkSqlStatement(qid, query.str(), &units, std::make_unique<optimizer::ForcedCostModel>(false), true);
    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(row);
}

/**
 * Arg: <0, 1, 2>
 * 0 - Number of columns to select
 * 1 - Number of rows
 * 2 - Cardinality
 */
BENCHMARK_REGISTER_F(MiniRunners, NestedLoopJoinRunners)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Apply(GenNJArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, AggregateRunners)(benchmark::State &state) {
  auto num_col = state.range(0);
  auto row = state.range(1);
  auto car = state.range(2);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();

    execution::query_id_t qid;
    switch (num_col) {
      case 1:
        qid = AGG_COL_1_QID;
        break;
      case 4:
        qid = AGG_COL_4_QID;
        break;
      case 8:
        qid = AGG_COL_8_QID;
        break;
      case 15:
        qid = AGG_COL_15_QID;
        break;
      default:
        UNREACHABLE("Unexpected number of columns for Aggregate");
        break;
    }

    brain::PipelineOperatingUnits units;
    brain::ExecutionOperatingUnitFeatureVector pipe0_vec;
    brain::ExecutionOperatingUnitFeatureVector pipe1_vec;
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::SEQ_SCAN, row, 60, 15, static_cast<double>(car));
    pipe0_vec.emplace_back(brain::ExecutionOperatingUnitType::AGGREGATE_BUILD, 4 * num_col, num_col, row,
                           static_cast<double>(car));
    pipe1_vec.emplace_back(brain::ExecutionOperatingUnitType::AGGREGATE_ITERATE, car, 0, 0, static_cast<double>(car));
    pipe1_vec.emplace_back(brain::ExecutionOperatingUnitType::OP_INTEGER_PLUS_OR_MINUS, car, 4, 1,
                           static_cast<double>(car));
    units.RecordOperatingUnit(execution::pipeline_id_t(0), std::move(pipe0_vec));
    units.RecordOperatingUnit(execution::pipeline_id_t(1), std::move(pipe1_vec));

    std::stringstream query;
    query << "SELECT COUNT(*) FROM INTEGERCol15Row" << row << "Car" << car << " GROUP BY ";
    for (auto i = 1; i <= num_col; i++) {
      query << "col15";
      if (i != num_col) {
        query << ", ";
      }
    }

    BenchmarkSqlStatement(qid, query.str(), &units, std::make_unique<optimizer::TrivialCostModel>(), true);
    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(row);
}

/**
 * Arg: <0, 1, 2>
 * 0 - Number of columns to select
 * 1 - Number of rows
 * 2 - Cardinality
 */
BENCHMARK_REGISTER_F(MiniRunners, AggregateRunners)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Apply(GenScanArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, IndexScanRunners)(benchmark::State &state) {
  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();
    BenchmarkIndexScan(state.range(0), state.range(1), state.range(2));
    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(state.range(2));
}

/**
 * Arg: <0, 1, 2>
 * 0 - Key Sizes
 * 1 - Idx Size
 *
 * 0 - ExecutionOperatingType
 * 1 - Iteration count
 * 2 - Lookup size
 */
static void GenIdxScanArguments(benchmark::internal::Benchmark *b) {
  auto key_sizes = {1, 4, 8, 15};
  auto idx_sizes = {1, 100, 1000, 10000};
  std::vector<double> lookup_sizes = {0.1, 0.25, 0.33, 0.5, 0.66, 0.75, 0.9, 1};
  for (auto key_size : key_sizes) {
    for (auto idx_size : idx_sizes) {
      std::unordered_set<int> lookups;
      for (auto lookup_size : lookup_sizes) {
        auto size = static_cast<int64_t>(idx_size * lookup_size);
        if (size > 0 && lookups.find(size) == lookups.end()) {
          lookups.insert(size);
          b->Args({key_size, idx_size, size});
        }
      }
    }
  }
}

BENCHMARK_REGISTER_F(MiniRunners, IndexScanRunners)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Apply(GenIdxScanArguments);

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(MiniRunners, ArithmeticRunners)(benchmark::State &state) {
  // NOLINTNEXTLINE
  for (auto _ : state) {
    metrics_manager_->RegisterThread();

    // state.range(0) is the OperatingUnitType
    // state.range(1) is the size
    BenchmarkArithmetic(static_cast<brain::ExecutionOperatingUnitType>(state.range(0)),
                        static_cast<size_t>(state.range(1)));

    metrics_manager_->Aggregate();
    metrics_manager_->UnregisterThread();
  }

  state.SetItemsProcessed(state.range(1));
}

/**
 * Arg: <0, 1>
 * 0 - ExecutionOperatingType
 * 1 - Iteration count
 */
static void GenArithArguments(benchmark::internal::Benchmark *b) {
  auto operators = {brain::ExecutionOperatingUnitType::OP_INTEGER_PLUS_OR_MINUS,
                    brain::ExecutionOperatingUnitType::OP_INTEGER_MULTIPLY,
                    brain::ExecutionOperatingUnitType::OP_INTEGER_DIVIDE,
                    brain::ExecutionOperatingUnitType::OP_INTEGER_COMPARE,
                    brain::ExecutionOperatingUnitType::OP_DECIMAL_PLUS_OR_MINUS,
                    brain::ExecutionOperatingUnitType::OP_DECIMAL_MULTIPLY,
                    brain::ExecutionOperatingUnitType::OP_DECIMAL_DIVIDE,
                    brain::ExecutionOperatingUnitType::OP_DECIMAL_COMPARE};

  auto counts = {10, 100, 10000, 1000000, 100000000};
  for (auto op : operators) {
    for (auto count : counts) {
      b->Args({static_cast<int64_t>(op), count});
    }
  }
}

BENCHMARK_REGISTER_F(MiniRunners, ArithmeticRunners)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Apply(GenArithArguments);

void InitializeRunnersState() {
  terrier::execution::CpuInfo::Instance();
  terrier::execution::ExecutionUtil::InitTPL();
  auto db_main_builder = DBMain::Builder()
                             .SetUseGC(true)
                             .SetUseCatalog(true)
                             .SetUseStatsStorage(true)
                             .SetUseMetrics(true)
                             .SetBlockStoreSize(1000000)
                             .SetBlockStoreReuse(1000000)
                             .SetRecordBufferSegmentSize(1000000)
                             .SetRecordBufferSegmentReuse(1000000);

  db_main = db_main_builder.Build().release();

  auto block_store = db_main->GetStorageLayer()->GetBlockStore();
  auto catalog = db_main->GetCatalogLayer()->GetCatalog();
  auto txn_manager = db_main->GetTransactionLayer()->GetTransactionManager();
  db_main->GetMetricsManager()->EnableMetric(metrics::MetricsComponent::EXECUTION_PIPELINE, 0);

  // Create the database
  auto txn = txn_manager->BeginTransaction();
  db_oid = catalog->CreateDatabase(common::ManagedPointer(txn), "test_db", true);

  // Load the database
  auto accessor = catalog->GetAccessor(common::ManagedPointer(txn), db_oid);
  auto exec_ctx = std::make_unique<execution::exec::ExecutionContext>(db_oid, common::ManagedPointer(txn), nullptr,
                                                                      nullptr, common::ManagedPointer(accessor));

  execution::sql::TableGenerator table_gen(exec_ctx.get(), block_store, accessor->GetDefaultNamespace());
  table_gen.GenerateTestTables(true);
  table_gen.GenerateMiniRunnerIndexes();

  txn_manager->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
}

void EndRunnersState() {
  terrier::execution::ExecutionUtil::ShutdownTPL();
  db_main->GetMetricsManager()->Aggregate();
  db_main->GetMetricsManager()->ToCSV();
  // free db main here so we don't need to use the loggers anymore
  delete db_main;
}

}  // namespace terrier::runner

int main(int argc, char **argv) {
  terrier::LoggersUtil::Initialize();

  // Benchmark Config Environment Variables
  // Check whether we are being passed environment variables to override configuration parameter
  // for this benchmark run.
  const char *env_num_threads = std::getenv(terrier::ENV_NUM_THREADS);
  if (env_num_threads != nullptr) terrier::BenchmarkConfig::num_threads = atoi(env_num_threads);

  const char *env_logfile_path = std::getenv(terrier::ENV_LOGFILE_PATH);
  if (env_logfile_path != nullptr) terrier::BenchmarkConfig::logfile_path = std::string_view(env_logfile_path);

  terrier::runner::InitializeRunnersState();
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  terrier::runner::EndRunnersState();

  terrier::LoggersUtil::ShutDown();

  return 0;
}
