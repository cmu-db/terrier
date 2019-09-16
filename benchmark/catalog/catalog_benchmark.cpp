#include <memory>
#include <vector>

#include "benchmark/benchmark.h"
#include "common/scoped_timer.h"

#include "catalog/catalog.h"
#include "catalog/catalog_accessor.h"
#include "catalog/catalog_defs.h"
#include "parser/expression/column_value_expression.h"
#include "parser/expression/constant_value_expression.h"
#include "storage/garbage_collector.h"
#include "storage/index/index_builder.h"
#include "storage/sql_table.h"
#include "storage/storage_defs.h"
#include "transaction/transaction_manager.h"
#include "transaction/transaction_util.h"
#include "type/transient_value_factory.h"

namespace terrier {

class CatalogBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const benchmark::State &state) final {
    // Initialize the transaction manager and GC
    timestamp_manager_ = new transaction::TimestampManager;
    deferred_action_manager_ = new transaction::DeferredActionManager(timestamp_manager_);
    txn_manager_ = new transaction::TransactionManager(timestamp_manager_, deferred_action_manager_, &buffer_pool_,
                                                       true, DISABLED);
    gc_ = new storage::GarbageCollector(timestamp_manager_, deferred_action_manager_, txn_manager_, nullptr);

    // Build out the catalog and commit so that it is visible to other transactions
    catalog_ = new catalog::Catalog(txn_manager_, &block_store_);

    auto txn = txn_manager_->BeginTransaction();
    db_ = catalog_->CreateDatabase(txn, "terrier", true);
    txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

    // Run the GC to flush it down to a clean system
    gc_->PerformGarbageCollection();
    gc_->PerformGarbageCollection();
  }

  void TearDown(const benchmark::State &state) final {
    catalog_->TearDown();
    // Run the GC to clean up transactions
    gc_->PerformGarbageCollection();
    gc_->PerformGarbageCollection();
    gc_->PerformGarbageCollection();

    delete catalog_;  // need to delete catalog_first
    delete gc_;
    delete txn_manager_;
    delete deferred_action_manager_;
    delete timestamp_manager_;
  }

  catalog::Catalog *catalog_;
  storage::RecordBufferSegmentPool buffer_pool_{100, 100};
  storage::BlockStore block_store_{100, 100};
  transaction::TimestampManager *timestamp_manager_;
  transaction::DeferredActionManager *deferred_action_manager_;
  transaction::TransactionManager *txn_manager_;

  storage::GarbageCollector *gc_;
  catalog::db_oid_t db_;
};

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetAccessor)(benchmark::State &state) {
  auto *txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto accessor = catalog_->GetAccessor(txn, db_);
    TERRIER_ASSERT(accessor != nullptr, "getting accessor should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetDatabaseOid)(benchmark::State &state) {
  auto *txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_oid UNUSED_ATTRIBUTE = catalog_->GetDatabaseOid(txn, "terrier");
    TERRIER_ASSERT(test_oid == db_, "getting oid should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetDatabaseCatalog)(benchmark::State &state) {
  auto *txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto dbc UNUSED_ATTRIBUTE = catalog_->GetDatabaseCatalog(txn, db_);
    TERRIER_ASSERT(dbc != nullptr, "getting accessor should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);
  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetIndex)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  // Create the index
  std::vector<catalog::IndexSchema::Column> key_cols{catalog::IndexSchema::Column{
      "id", type::TypeId::INTEGER, false, parser::ColumnValueExpression(db_, table_oid, schema.GetColumn("id").Oid())}};
  auto index_schema = catalog::IndexSchema(key_cols, true, true, false, true);
  auto idx_oid = accessor->CreateIndex(accessor->GetDefaultNamespace(), table_oid, "test_table_idx", index_schema);
  TERRIER_ASSERT(idx_oid != catalog::INVALID_INDEX_OID, "index creation should not fail");
  auto true_schema = accessor->GetIndexSchema(idx_oid);

  storage::index::IndexBuilder index_builder;
  index_builder.SetOid(idx_oid).SetKeySchema(true_schema).SetConstraintType(storage::index::ConstraintType::UNIQUE);
  auto index = index_builder.Build();

  result = accessor->SetIndexPointer(idx_oid, index);
  TERRIER_ASSERT(result, "setting index pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_index UNUSED_ATTRIBUTE = accessor->GetIndex(idx_oid);
    TERRIER_ASSERT(common::ManagedPointer(index) == test_index, "getting index should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetIndexOid)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  // Create the index
  std::vector<catalog::IndexSchema::Column> key_cols{catalog::IndexSchema::Column{
      "id", type::TypeId::INTEGER, false, parser::ColumnValueExpression(db_, table_oid, schema.GetColumn("id").Oid())}};
  auto index_schema = catalog::IndexSchema(key_cols, true, true, false, true);
  auto idx_oid = accessor->CreateIndex(accessor->GetDefaultNamespace(), table_oid, "test_table_idx", index_schema);
  TERRIER_ASSERT(idx_oid != catalog::INVALID_INDEX_OID, "index creation should not fail");
  auto true_schema = accessor->GetIndexSchema(idx_oid);

  storage::index::IndexBuilder index_builder;
  index_builder.SetOid(idx_oid).SetKeySchema(true_schema).SetConstraintType(storage::index::ConstraintType::UNIQUE);
  auto index = index_builder.Build();

  result = accessor->SetIndexPointer(idx_oid, index);
  TERRIER_ASSERT(result, "setting index pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_index UNUSED_ATTRIBUTE = accessor->GetIndexOid("test_table_idx");
    TERRIER_ASSERT(idx_oid == test_index, "getting index oid should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetIndexOids)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  // Create the index
  std::vector<catalog::IndexSchema::Column> key_cols{catalog::IndexSchema::Column{
      "id", type::TypeId::INTEGER, false, parser::ColumnValueExpression(db_, table_oid, schema.GetColumn("id").Oid())}};
  auto index_schema = catalog::IndexSchema(key_cols, true, true, false, true);
  auto idx_oid = accessor->CreateIndex(accessor->GetDefaultNamespace(), table_oid, "test_table_idx", index_schema);
  TERRIER_ASSERT(idx_oid != catalog::INVALID_INDEX_OID, "index creation should not fail");
  auto true_schema = accessor->GetIndexSchema(idx_oid);

  storage::index::IndexBuilder index_builder;
  index_builder.SetOid(idx_oid).SetKeySchema(true_schema).SetConstraintType(storage::index::ConstraintType::UNIQUE);
  auto index = index_builder.Build();

  result = accessor->SetIndexPointer(idx_oid, index);
  TERRIER_ASSERT(result, "setting index pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_indexes UNUSED_ATTRIBUTE = accessor->GetIndexOids(table_oid);
    TERRIER_ASSERT(!test_indexes.empty(), "getting index oids should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetIndexSchema)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  // Create the index
  std::vector<catalog::IndexSchema::Column> key_cols{catalog::IndexSchema::Column{
      "id", type::TypeId::INTEGER, false, parser::ColumnValueExpression(db_, table_oid, schema.GetColumn("id").Oid())}};
  auto index_schema = catalog::IndexSchema(key_cols, true, true, false, true);
  auto idx_oid = accessor->CreateIndex(accessor->GetDefaultNamespace(), table_oid, "test_table_idx", index_schema);
  TERRIER_ASSERT(idx_oid != catalog::INVALID_INDEX_OID, "index creation should not fail");
  auto true_schema = accessor->GetIndexSchema(idx_oid);

  storage::index::IndexBuilder index_builder;
  index_builder.SetOid(idx_oid).SetKeySchema(true_schema).SetConstraintType(storage::index::ConstraintType::UNIQUE);
  auto index = index_builder.Build();

  result = accessor->SetIndexPointer(idx_oid, index);
  TERRIER_ASSERT(result, "setting index pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_idx_schema UNUSED_ATTRIBUTE = accessor->GetIndexSchema(idx_oid);
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetNamespaceOid)(benchmark::State &state) {
  // Create a database and check that it's immediately visible
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);
  const auto ns_oid UNUSED_ATTRIBUTE = accessor->CreateNamespace("test_namespace");
  TERRIER_ASSERT(ns_oid != catalog::INVALID_NAMESPACE_OID, "namespace creation should not fail");
  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();
  accessor = catalog_->GetAccessor(txn, db_);

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_ns_oid UNUSED_ATTRIBUTE = accessor->GetNamespaceOid("test_namespace");
    TERRIER_ASSERT(test_ns_oid == ns_oid, "namespace lookup should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetSchema)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_schema UNUSED_ATTRIBUTE = accessor->GetSchema(table_oid);
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetTable)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_table UNUSED_ATTRIBUTE = accessor->GetTable(table_oid);
    TERRIER_ASSERT(common::ManagedPointer<storage::SqlTable>(table) == test_table, "table lookup should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// NOLINTNEXTLINE
BENCHMARK_DEFINE_F(CatalogBenchmark, GetTableOid)(benchmark::State &state) {
  auto txn = txn_manager_->BeginTransaction();
  auto accessor = catalog_->GetAccessor(txn, db_);

  // Create the column definition (no OIDs)
  std::vector<catalog::Schema::Column> cols;
  cols.emplace_back("id", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  cols.emplace_back("user_col_1", type::TypeId::INTEGER, false,
                    parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
  auto tmp_schema = catalog::Schema(cols);

  auto table_oid = accessor->CreateTable(accessor->GetDefaultNamespace(), "test_table", tmp_schema);
  TERRIER_ASSERT(table_oid != catalog::INVALID_TABLE_OID, "table creation should not fail");
  auto schema = accessor->GetSchema(table_oid);
  auto table = new storage::SqlTable(&block_store_, schema);

  auto result UNUSED_ATTRIBUTE = accessor->SetTablePointer(table_oid, table);
  TERRIER_ASSERT(result, "setting table pointer should not fail");

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  txn = txn_manager_->BeginTransaction();

  // NOLINTNEXTLINE
  for (auto _ : state) {
    const auto test_table_oid UNUSED_ATTRIBUTE = accessor->GetTableOid("test_table");
    TERRIER_ASSERT(test_table_oid == table_oid, "table oid lookup should not fail");
  }

  txn_manager_->Commit(txn, transaction::TransactionUtil::EmptyCallback, nullptr);

  state.SetItemsProcessed(state.iterations());
}

// Catalog benchmarks
BENCHMARK_REGISTER_F(CatalogBenchmark, GetAccessor)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetDatabaseOid)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetDatabaseCatalog)->Unit(benchmark::kNanosecond);

// CatalogAccessor benchmarks
BENCHMARK_REGISTER_F(CatalogBenchmark, GetIndex)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetIndexOid)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetIndexOids)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetIndexSchema)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetNamespaceOid)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetSchema)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetTable)->Unit(benchmark::kNanosecond);
BENCHMARK_REGISTER_F(CatalogBenchmark, GetTableOid)->Unit(benchmark::kNanosecond);
}  // namespace terrier
