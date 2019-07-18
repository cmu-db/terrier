#include <vector>

#include "catalog/catalog_defs.h"
#include "catalog/database_catalog.h"
#include "catalog/index_schema.h"
#include "catalog/postgres/builder.h"
#include "catalog/postgres/pg_attribute.h"
#include "catalog/postgres/pg_class.h"
#include "catalog/postgres/pg_constraint.h"
#include "catalog/postgres/pg_index.h"
#include "catalog/postgres/pg_namespace.h"
#include "catalog/postgres/pg_type.h"
#include "catalog/schema.h"
#include "storage/index/index.h"
#include "storage/sql_table.h"
#include "transaction/transaction_context.h"
#include "transaction/transaction_defs.h"
#include "transaction/transaction_manager.h"
#include "type/type_id.h"

namespace terrier::catalog {

void DatabaseCatalog::Bootstrap(transaction::TransactionContext *txn) {
  // Declare variable for return values (UNUSED when compiled for release)
  bool UNUSED_ATTRIBUTE retval;

  retval = CreateNamespace(txn, "pg_catalog", NAMESPACE_CATALOG_NAMESPACE_OID);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateNamespace(txn, "public", NAMESPACE_DEFAULT_NAMESPACE_OID);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  BootstrapTypes(txn);

  // pg_namespace and associated indexes
  retval = CreateTableEntry(txn, NAMESPACE_TABLE_OID, NAMESPACE_CATALOG_NAMESPACE_OID, "pg_namespace",new Schema(postgres::Builder::GetNamespaceTableSchema()));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetTablePointer(txn, NAMESPACE_TABLE_OID, namespaces_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, NAMESPACE_TABLE_OID, NAMESPACE_OID_INDEX_OID, "pg_namespace_oid_index", postgres::Builder::GetNamespaceOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, NAMESPACE_OID_INDEX_OID, namespaces_oid_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, NAMESPACE_TABLE_OID, NAMESPACE_NAME_INDEX_OID, "pg_namespace_name_index", postgres::Builder::GetNamespaceOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, NAMESPACE_NAME_INDEX_OID, namespaces_name_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  // pg_class and associated indexes
  retval = CreateTableEntry(txn, CLASS_TABLE_OID, NAMESPACE_CATALOG_NAMESPACE_OID, "pg_class", new Schema(postgres::Builder::GetClassTableSchema()));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetTablePointer(txn, CLASS_TABLE_OID, classes_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CLASS_TABLE_OID, CLASS_OID_INDEX_OID, "pg_class_oid_index", postgres::Builder::GetClassOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CLASS_OID_INDEX_OID, classes_oid_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CLASS_TABLE_OID, CLASS_NAME_INDEX_OID, "pg_class_name_index", postgres::Builder::GetClassNameIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CLASS_NAME_INDEX_OID, classes_name_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CLASS_TABLE_OID, CLASS_NAMESPACE_INDEX_OID, "pg_class_namespace_index", postgres::Builder::GetClassNamespaceIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CLASS_NAMESPACE_INDEX_OID, classes_namespace_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  // pg_index and associated indexes
  retval = CreateTableEntry(txn, INDEX_TABLE_OID, NAMESPACE_CATALOG_NAMESPACE_OID, "pg_index", new Schema(postgres::Builder::GetIndexTableSchema()));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetTablePointer(txn, INDEX_TABLE_OID, indexes_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, INDEX_TABLE_OID, INDEX_OID_INDEX_OID, "pg_index_oid_index", postgres::Builder::GetIndexOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, INDEX_OID_INDEX_OID, indexes_oid_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, INDEX_TABLE_OID, INDEX_TABLE_INDEX_OID, "pg_index_table_index", postgres::Builder::GetIndexTableIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, INDEX_TABLE_INDEX_OID, indexes_table_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  // pg_attribute and associated indexes
  retval = CreateTableEntry(txn, COLUMN_TABLE_OID, NAMESPACE_CATALOG_NAMESPACE_OID, "pg_attribute", new Schema(postgres::Builder::GetColumnTableSchema()));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetTablePointer(txn, COLUMN_TABLE_OID, columns_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, COLUMN_TABLE_OID, COLUMN_OID_INDEX_OID, "pg_attribute_oid_index", postgres::Builder::GetColumnOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, COLUMN_OID_INDEX_OID, columns_oid_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, COLUMN_TABLE_OID, COLUMN_NAME_INDEX_OID, "pg_attribute_name_index", postgres::Builder::GetColumnNameIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, COLUMN_NAME_INDEX_OID, columns_name_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, COLUMN_TABLE_OID, COLUMN_CLASS_INDEX_OID, "pg_attribute_name_index", postgres::Builder::GetColumnClassIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, COLUMN_CLASS_INDEX_OID, columns_class_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  // pg_type and associated indexes
  retval = CreateTableEntry(txn, TYPE_TABLE_OID, NAMESPACE_CATALOG_NAMESPACE_OID, "pg_type", new Schema(postgres::Builder::GetTypeTableSchema()));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetTablePointer(txn, TYPE_TABLE_OID, types_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, TYPE_TABLE_OID, TYPE_OID_INDEX_OID, "pg_type_oid_index", postgres::Builder::GetTypeOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, TYPE_OID_INDEX_OID, types_oid_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, TYPE_TABLE_OID, TYPE_NAME_INDEX_OID, "pg_type_name_index", postgres::Builder::GetTypeNameIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, TYPE_NAME_INDEX_OID, types_name_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, TYPE_TABLE_OID, TYPE_NAMESPACE_INDEX_OID, "pg_type_namespace_index", postgres::Builder::GetTypeNamespaceIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, TYPE_NAMESPACE_INDEX_OID, types_namespace_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  // pg_constraint and associated indexes
  retval = CreateTableEntry(txn, CONSTRAINT_TABLE_OID, NAMESPACE_CATALOG_NAMESPACE_OID, "pg_constraint", new Schema(postgres::Builder::GetConstraintTableSchema()));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetTablePointer(txn, CONSTRAINT_TABLE_OID, constraints_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CONSTRAINT_TABLE_OID, CONSTRAINT_OID_INDEX_OID, "pg_constraint_oid_index", postgres::Builder::GetConstraintOidIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CONSTRAINT_OID_INDEX_OID, constraints_oid_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CONSTRAINT_TABLE_OID, CONSTRAINT_NAME_INDEX_OID, "pg_constraint_name_index", postgres::Builder::GetConstraintNameIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CONSTRAINT_NAME_INDEX_OID, constraints_name_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CONSTRAINT_TABLE_OID, CONSTRAINT_NAMESPACE_INDEX_OID, "pg_constraint_namespace_index", postgres::Builder::GetConstraintNamespaceIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CONSTRAINT_NAMESPACE_INDEX_OID, constraints_namespace_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CONSTRAINT_TABLE_OID, CONSTRAINT_TABLE_INDEX_OID, "pg_constraint_table_index", postgres::Builder::GetConstraintTableIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CONSTRAINT_TABLE_INDEX_OID, constraints_table_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CONSTRAINT_TABLE_OID, CONSTRAINT_INDEX_INDEX_OID, "pg_constraint_index_index", postgres::Builder::GetConstraintIndexIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CONSTRAINT_INDEX_INDEX_OID, constraints_index_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");

  retval = CreateIndexEntry(txn, NAMESPACE_CATALOG_NAMESPACE_OID, CONSTRAINT_TABLE_OID, CONSTRAINT_FOREIGNTABLE_INDEX_OID, "pg_constraint_foreigntable_index", postgres::Builder::GetConstraintForeignTableIndexSchema(db_oid_));
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
  retval = SetIndexPointer(txn, CONSTRAINT_FOREIGNTABLE_INDEX_OID, constraints_foreigntable_index_);
  TERRIER_ASSERT(retval, "Bootstrap operations should not fail");
}

namespace_oid_t DatabaseCatalog::CreateNamespace(transaction::TransactionContext *txn, const std::string &name) {
  namespace_oid_t ns_oid{next_oid_++};
  if (CreateNamespace(txn, name, ns_oid)) {
    return INVALID_NAMESPACE_OID;
  }
  return ns_oid;
}

bool DatabaseCatalog::CreateNamespace(transaction::TransactionContext *txn, const std::string &name,
                                      namespace_oid_t ns_oid) {
  // Step 1: Insert into table
  storage::VarlenEntry name_varlen = postgres::AttributeHelper::CreateVarlen(name);
  // Get & Fill Redo Record
  std::vector<col_oid_t> table_oids{NSPNAME_COL_OID, NSPOID_COL_OID};
  // NOLINTNEXTLINE (C++17 only)
  auto [pri, pm] = namespaces_->InitializerForProjectedRow(table_oids);
  auto *redo = txn->StageWrite(db_oid_, NAMESPACE_TABLE_OID, pri);
  auto *oid_entry = reinterpret_cast<namespace_oid_t *>(redo->Delta()->AccessForceNotNull(pm[NSPOID_COL_OID]));
  auto *name_entry = reinterpret_cast<storage::VarlenEntry *>(redo->Delta()->AccessForceNotNull(pm[NSPNAME_COL_OID]));
  *oid_entry = ns_oid;
  *name_entry = name_varlen;
  // Finally, insert into the table to get the tuple slot
  auto tupleslot = namespaces_->Insert(txn, redo);

  // Step 2: Insert into name index
  auto name_pri = namespaces_name_index_->GetProjectedRowInitializer();
  byte *buffer = common::AllocationUtil::AllocateAligned(name_pri.ProjectedRowSize());
  auto *pr = name_pri.InitializeRow(buffer);
  name_entry = reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(0));
  *name_entry = name_varlen;

  if (!namespaces_name_index_->InsertUnique(txn, *pr, tupleslot)) {
    // There was a name conflict and we need to abort.  Free the buffer and
    // return INVALID_DATABASE_OID to indicate the database was not created.
    delete[] buffer;
    return false;
  }

  // Step 3: Insert into oid index
  auto oid_pri = namespaces_oid_index_->GetProjectedRowInitializer();
  // Reuse buffer since an u32 column is smaller than a varlen column
  pr = oid_pri.InitializeRow(buffer);
  oid_entry = reinterpret_cast<namespace_oid_t *>(pr->AccessForceNotNull(0));
  *oid_entry = ns_oid;
  const bool UNUSED_ATTRIBUTE result = namespaces_oid_index_->InsertUnique(txn, *pr, tupleslot);
  TERRIER_ASSERT(result, "Assigned namespace OID failed to be unique");

  delete[] buffer;
  return true;
}

bool DatabaseCatalog::DeleteNamespace(transaction::TransactionContext *txn, namespace_oid_t ns) {
  // Step 1: Read the oid index
  std::vector<col_oid_t> table_oids{NSPNAME_COL_OID};
  // NOLINTNEXTLINE (C++17 only)
  auto [table_pri, table_pm] = namespaces_->InitializerForProjectedRow(table_oids);
  auto name_pri = namespaces_name_index_->GetProjectedRowInitializer();
  auto oid_pri = namespaces_oid_index_->GetProjectedRowInitializer();
  // Buffer is large enough for all prs.
  byte *buffer = common::AllocationUtil::AllocateAligned(table_pri.ProjectedRowSize());
  auto pr = oid_pri.InitializeRow(buffer);
  // Scan index
  auto *oid_entry = reinterpret_cast<namespace_oid_t *>(pr->AccessForceNotNull(0));
  *oid_entry = ns;
  std::vector<storage::TupleSlot> index_results;
  namespaces_oid_index_->ScanKey(*txn, *pr, &index_results);
  if (index_results.empty()) {
    delete[] buffer;
    return false;
  }
  TERRIER_ASSERT(index_results.size() == 1, "Namespace OID not unique in index");

  // Step 2: Select from the table to get the name
  pr = table_pri.InitializeRow(buffer);
  if (!namespaces_->Select(txn, index_results[0], pr)) {
    // Nothing visible
    delete[] buffer;
    return false;
  }
  auto name_varlen = *reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(table_pm[NSPNAME_COL_OID]));

  // Step 3: Delete from table
  txn->StageDelete(db_oid_, NAMESPACE_TABLE_OID, index_results[0]);
  if (!namespaces_->Delete(txn, index_results[0])) {
    // Someone else has a write-lock
    delete[] buffer;
    return false;
  }

  // Step 4: Delete from oid index
  pr = oid_pri.InitializeRow(buffer);
  oid_entry = reinterpret_cast<namespace_oid_t *>(pr->AccessForceNotNull(table_pm[NSPOID_COL_OID]));
  *oid_entry = ns;
  namespaces_oid_index_->Delete(txn, *pr, index_results[0]);

  // Step 5: Delete from name index
  pr = name_pri.InitializeRow(buffer);
  auto name_entry = reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(table_pm[NSPNAME_COL_OID]));
  *name_entry = name_varlen;
  namespaces_name_index_->Delete(txn, *pr, index_results[0]);

  // Finish
  delete[] buffer;
  return true;
}

namespace_oid_t DatabaseCatalog::GetNamespaceOid(transaction::TransactionContext *txn, const std::string &name) {
  // Step 1: Read the name index
  std::vector<col_oid_t> table_oids{NSPNAME_COL_OID};
  // NOLINTNEXTLINE
  auto [table_pri, table_pm] = namespaces_->InitializerForProjectedRow(table_oids);
  auto name_pri = namespaces_name_index_->GetProjectedRowInitializer();
  // Buffer is large enough to hold all prs
  byte *buffer = common::AllocationUtil::AllocateAligned(table_pri.ProjectedRowSize());
  auto pr = name_pri.InitializeRow(buffer);
  // Scan the name index
  auto *name_entry = reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(0));
  *name_entry = postgres::AttributeHelper::CreateVarlen(name);
  std::vector<storage::TupleSlot> index_results;
  namespaces_name_index_->ScanKey(*txn, *pr, &index_results);
  if (index_results.empty()) {
    delete[] buffer;
    return INVALID_NAMESPACE_OID;
  }
  TERRIER_ASSERT(index_results.size() == 1, "Namespace name not unique in index");

  // Step 2: Scan the table to get the oid
  pr = table_pri.InitializeRow(buffer);
  if (!namespaces_->Select(txn, index_results[0], pr)) {
    // Nothing visible
    delete[] buffer;
    return INVALID_NAMESPACE_OID;
  }
  auto ns_oid = *reinterpret_cast<namespace_oid_t *>(pr->AccessForceNotNull(table_pm[NSPOID_COL_OID]));
  delete[] buffer;
  return ns_oid;
}

template <typename Column>
bool DatabaseCatalog::CreateAttribute(transaction::TransactionContext *txn, uint32_t class_oid, const Column &col,
                                      const parser::AbstractExpression *default_val) {
  // Step 1: Insert into the table
  std::vector<col_oid_t> table_oids{ATTNUM_COL_OID, ATTRELID_COL_OID,   ATTNAME_COL_OID, ATTTYPID_COL_OID,
                                    ATTLEN_COL_OID, ATTNOTNULL_COL_OID, ADBIN_COL_OID,   ADSRC_COL_OID};
  // NOLINTNEXTLINE
  auto [table_pri, table_pm] = columns_->InitializerForProjectedRow(table_oids);
  auto *redo = txn->StageWrite(db_oid_, COLUMN_TABLE_OID, table_pri);
  auto oid_entry = reinterpret_cast<uint32_t *>(redo->Delta()->AccessForceNotNull(table_pm[ATTNUM_COL_OID]));
  auto relid_entry = reinterpret_cast<uint32_t *>(redo->Delta()->AccessForceNotNull(table_pm[ATTRELID_COL_OID]));
  auto name_entry =
      reinterpret_cast<storage::VarlenEntry *>(redo->Delta()->AccessForceNotNull(table_pm[ATTNAME_COL_OID]));
  auto type_entry = reinterpret_cast<type::TypeId *>(redo->Delta()->AccessForceNotNull(table_pm[ATTTYPID_COL_OID]));
  auto len_entry = reinterpret_cast<uint16_t *>(redo->Delta()->AccessForceNotNull(table_pm[ATTLEN_COL_OID]));
  auto notnull_entry = reinterpret_cast<bool *>(redo->Delta()->AccessForceNotNull(table_pm[ATTNOTNULL_COL_OID]));
  auto dbin_entry = reinterpret_cast<intptr_t *>(redo->Delta()->AccessForceNotNull(table_pm[ADBIN_COL_OID]));
  auto dsrc_entry =
      reinterpret_cast<storage::VarlenEntry *>(redo->Delta()->AccessForceNotNull(table_pm[ADSRC_COL_OID]));
  *oid_entry = !col.GetOid();
  *relid_entry = class_oid;
  storage::VarlenEntry name_varlen = postgres::AttributeHelper::MakeNameVarlen<Column>(col);
  *name_entry = name_varlen;
  *type_entry = col.GetType();
  // TODO(Amadou): Figure out what really goes here for varlen
  *len_entry = (col.GetType() == type::TypeId::VARCHAR || col.GetType() == type::TypeId::VARBINARY)
                   ? col.GetMaxVarlenSize()
                   : col.GetAttrSize();
  *notnull_entry = !col.GetNullable();
  *dbin_entry = reinterpret_cast<intptr_t>(default_val);
  storage::VarlenEntry dsrc_varlen = postgres::AttributeHelper::CreateVarlen(default_val->ToJson().dump());
  *dsrc_entry = dsrc_varlen;
  // Finally insert
  auto tupleslot = columns_->Insert(txn, redo);

  // Step 2: Insert into name index
  auto name_pri = columns_name_index_->GetProjectedRowInitializer();
  // Create a buffer large enough for all columns
  auto buffer = common::AllocationUtil::AllocateAligned(name_pri.ProjectedRowSize());
  auto pr = name_pri.InitializeRow(buffer);
  name_entry = reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(0));
  *name_entry = name_varlen;
  relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
  *relid_entry = class_oid;

  if (!columns_name_index_->InsertUnique(txn, *pr, tupleslot)) {
    delete[] buffer;
    return false;
  }

  // Step 3: Insert into oid index
  auto oid_pri = columns_oid_index_->GetProjectedRowInitializer();
  pr = oid_pri.InitializeRow(buffer);
  oid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(0));
  *oid_entry = col.GetOid();
  relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
  *relid_entry = class_oid;

  if (!columns_oid_index_->InsertUnique(txn, *pr, tupleslot)) {
    delete[] buffer;
    return false;
  }

  // Step 4: Insert into class index
  auto class_pri = columns_class_index_->GetProjectedRowInitializer();
  pr = class_pri.InitializeRow(buffer);
  relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(0));
  *relid_entry = class_oid;

  if (!columns_class_index_->Insert(txn, *pr, tupleslot)) {
    // Should probably not happen.
    delete[] buffer;
    return false;
  }

  delete[] buffer;
  return true;
}

template <typename Column>
std::unique_ptr<Column> DatabaseCatalog::GetAttribute(transaction::TransactionContext *txn,
                                                      storage::VarlenEntry *col_name, uint32_t class_oid) {
  // Step 1: Read Index
  std::vector<col_oid_t> table_oids{ATTNUM_COL_OID, ATTNAME_COL_OID,    ATTTYPID_COL_OID,
                                    ATTLEN_COL_OID, ATTNOTNULL_COL_OID, ADBIN_COL_OID};
  // NOLINTNEXTLINE
  auto [table_pri, table_pm] = columns_->InitializerForProjectedRow(table_oids);
  auto name_pri = columns_name_index_->GetProjectedRowInitializer();
  // Buffer is large enough to hold all prs
  byte *buffer = common::AllocationUtil::AllocateAligned(table_pri.ProjectedRowSize());
  // Scan the name index
  auto pr = name_pri.InitializeRow(buffer);
  auto *name_entry = reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(0));
  *name_entry = *col_name;
  auto *relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
  *relid_entry = class_oid;
  std::vector<storage::TupleSlot> index_results;
  columns_name_index_->ScanKey(*txn, *pr, &index_results);
  if (index_results.empty()) {
    delete[] buffer;
    return nullptr;
  }
  TERRIER_ASSERT(index_results.size() == 1, "Columns name not unique in index");

  // Step 2: Scan the table to get the column
  pr = table_pri.InitializeRow(buffer);
  if (!columns_->Select(txn, index_results[0], pr)) {
    // Nothing visible
    delete[] buffer;
    return nullptr;
  }
  auto col = postgres::AttributeHelper::MakeColumn<Column>(pr, table_pm);
  delete[] buffer;
  return col;
}

template <typename Column>
std::unique_ptr<Column> DatabaseCatalog::GetAttribute(transaction::TransactionContext *txn, uint32_t col_oid,
                                                      uint32_t class_oid) {
  // Step 1: Read Index
  std::vector<col_oid_t> table_oids{ATTNUM_COL_OID, ATTNAME_COL_OID,    ATTTYPID_COL_OID,
                                    ATTLEN_COL_OID, ATTNOTNULL_COL_OID, ADBIN_COL_OID};
  // NOLINTNEXTLINE
  auto [table_pri, table_pm] = columns_->InitializerForProjectedRow(table_oids);
  auto oid_pri = columns_oid_index_->GetProjectedRowInitializer();
  // Buffer is large enough to hold all prs
  byte *buffer = common::AllocationUtil::AllocateAligned(table_pri.ProjectedRowSize());
  // Scan the oid index
  auto pr = oid_pri.InitializeRow(buffer);
  auto *oid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(0));
  *oid_entry = col_oid;
  auto *relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
  *relid_entry = class_oid;
  std::vector<storage::TupleSlot> index_results;
  columns_oid_index_->ScanKey(*txn, *pr, &index_results);
  if (index_results.empty()) {
    delete[] buffer;
    return nullptr;
  }
  TERRIER_ASSERT(index_results.size() == 1, "Columns oid not unique in index");

  // Step 2: Scan the table to get the column
  pr = table_pri.InitializeRow(buffer);
  if (!columns_->Select(txn, index_results[0], pr)) {
    // Nothing visible
    delete[] buffer;
    return nullptr;
  }
  auto col = postgres::AttributeHelper::MakeColumn<Column>(pr, table_pm);
  delete[] buffer;
  return col;
}

template <typename Column>
std::vector<std::unique_ptr<Column>> DatabaseCatalog::GetAttributes(transaction::TransactionContext *txn,
                                                                    uint32_t class_oid) {
  // Step 1: Read Index
  std::vector<col_oid_t> table_oids{ATTNUM_COL_OID, ATTNAME_COL_OID,    ATTTYPID_COL_OID,
                                    ATTLEN_COL_OID, ATTNOTNULL_COL_OID, ADBIN_COL_OID};
  // NOLINTNEXTLINE
  auto [table_pri, table_pm] = columns_->InitializerForProjectedRow(table_oids);
  auto class_pri = columns_class_index_->GetProjectedRowInitializer();
  // Buffer is large enough to hold all prs
  byte *buffer = common::AllocationUtil::AllocateAligned(table_pri.ProjectedRowSize());
  // Scan the class index
  auto pr = class_pri.InitializeRow(buffer);
  auto *relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
  *relid_entry = class_oid;
  std::vector<storage::TupleSlot> index_results;
  columns_class_index_->ScanKey(*txn, *pr, &index_results);
  if (index_results.empty()) {
    delete[] buffer;
    return nullptr;
  }

  // Step 2: Scan the table to get the columns
  std::vector<std::unique_ptr<Column>> cols;
  pr = table_pri.InitializeRow(buffer);
  for (const auto &slot : index_results) {
    if (!columns_->Select(txn, slot, pr)) {
      // Nothing visible
      delete[] buffer;
      return nullptr;
    }
    cols.emplace_back(postgres::AttributeHelper::MakeColumn<Column>(pr, table_pm));
  }
  delete[] buffer;
  return cols;
}

template <typename Column>
void DatabaseCatalog::DeleteColumns(transaction::TransactionContext *txn, uint32_t class_oid) {
  // Step 1: Read Index
  std::vector<col_oid_t> table_oids{ATTNUM_COL_OID, ATTNAME_COL_OID,    ATTTYPID_COL_OID,
                                    ATTLEN_COL_OID, ATTNOTNULL_COL_OID, ADBIN_COL_OID};
  // NOLINTNEXTLINE
  auto [table_pri, table_pm] = columns_->InitializerForProjectedRow(table_oids);
  auto class_pri = columns_class_index_->GetProjectedRowInitializer();
  // Buffer is large enough to hold all prs
  byte *buffer = common::AllocationUtil::AllocateAligned(table_pri.ProjectedRowSize());
  // Scan the class index
  auto pr = class_pri.InitializeRow(buffer);
  auto *relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
  *relid_entry = class_oid;
  std::vector<storage::TupleSlot> index_results;
  columns_class_index_->ScanKey(*txn, *pr, &index_results);
  if (index_results.empty()) {
    delete[] buffer;
  }

  // Step 2: Scan the table to get the columns
  pr = table_pri.InitializeRow(buffer);
  for (const auto &slot : index_results) {
    if (!columns_->Select(txn, slot, pr)) {
      // Nothing visible
      delete[] buffer;
      return;
    }
    auto col = postgres::AttributeHelper::MakeColumn<Column>(pr, table_pm);
    // 1. Delete from class index
    class_pri = columns_class_index_->GetProjectedRowInitializer();
    pr = class_pri.InitializeRow(buffer);
    relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(0));
    *relid_entry = class_oid;
    columns_class_index_->Delete(txn, *pr, slot);

    // 2. Delete from oid index
    auto oid_pri = columns_oid_index_->GetProjectedRowInitializer();
    pr = oid_pri.InitializeRow(buffer);
    auto oid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(0));
    *oid_entry = !col->GetOid();
    relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
    *relid_entry = class_oid;
    columns_oid_index_->Delete(txn, *pr, slot);

    // 3. Delete from name index
    auto name_pri = columns_name_index_->GetProjectedRowInitializer();
    pr = name_pri.InitializeRow(buffer);
    auto name_entry = reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(0));
    *name_entry = postgres::AttributeHelper::MakeNameVarlen(*col);
    relid_entry = reinterpret_cast<uint32_t *>(pr->AccessForceNotNull(1));
    *relid_entry = class_oid;
    columns_name_index_->Delete(txn, *pr, slot);
  }
  delete[] buffer;
}

table_oid_t DatabaseCatalog::CreateTable(transaction::TransactionContext *txn, namespace_oid_t ns,
                                         const std::string &name, const Schema &schema) {
  const table_oid_t table_oid = static_cast<table_oid_t>(next_oid_++);

  // Write the col oids into a new Schema object
  col_oid_t next_col_oid(1);
  auto *const schema_ptr = new Schema(schema);
  txn->RegisterAbortAction([=]() { delete schema_ptr; });
  // TODO(Matt): when AbstractExpressions are added to Schema::Column as a field for default
  // value, we need to make sure Column gets a properly written copy constructor to deep
  // copy those to guarantee that this copy mechanism still works
  for (auto &column : schema_ptr->columns_) {
    column.oid_ = next_col_oid++;
  }

  return CreateTableEntry(txn, table_oid, ns, name, schema_ptr) ? table_oid : INVALID_TABLE_OID;
}

bool DatabaseCatalog::DeleteTable(transaction::TransactionContext *const txn, const table_oid_t table) {
  std::vector<storage::TupleSlot> index_results;
  auto oid_pri = classes_oid_index_->GetProjectedRowInitializer();

  auto [pr_init, pr_map] = classes_->InitializerForProjectedRow(PG_CLASS_ALL_COL_OIDS);

  TERRIER_ASSERT(pr_init.ProjectedRowSize() >= oid_pri.ProjectedRowSize(),
                 "Buffer must be allocated for largest ProjectedRow size");
  auto *const buffer = common::AllocationUtil::AllocateAligned(pr_init.ProjectedRowSize());
  auto *key_pr = oid_pri.InitializeRow(buffer);

  // Find the entry using the index
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table);
  classes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with a table_oid.
    // That implies that it was visible to us. Maybe the table was dropped or renamed twice by the same txn?
    delete[] buffer;
    return false;
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  // Select the tuple out of the table before deletion. We need the attributes to do index deletions later
  auto *table_pr = pr_init.InitializeRow(buffer);
  auto result = classes_->Select(txn, index_results[0], table_pr);
  TERRIER_ASSERT(result, "Select must succeed if the index scan gave a visible result.");

  // Delete from pg_classes table
  txn->StageDelete(db_oid_, CLASS_TABLE_OID, index_results[0]);
  result = classes_->Delete(txn, index_results[0]);
  if (!result) {
    // write-write conflict. Someone beat us to this operation.
    delete[] buffer;
    return false;
  }

  // Get the attributes we need for indexes
  const table_oid_t table_oid =
      *(reinterpret_cast<const table_oid_t *const>(table_pr->AccessForceNotNull(pr_map[RELOID_COL_OID])));
  TERRIER_ASSERT(table == table_oid,
                 "table oid from pg_classes did not match what was found by the index scan from the argument.");
  const namespace_oid_t ns_oid =
      *(reinterpret_cast<const namespace_oid_t *const>(table_pr->AccessForceNotNull(pr_map[RELNAMESPACE_COL_OID])));
  const storage::VarlenEntry name_varlen =
      *(reinterpret_cast<const storage::VarlenEntry *const>(table_pr->AccessForceNotNull(pr_map[RELNAME_COL_OID])));

  const auto oid_index_init = classes_oid_index_->GetProjectedRowInitializer();
  const auto name_index_init = classes_name_index_->GetProjectedRowInitializer();
  const auto ns_index_init = classes_namespace_index_->GetProjectedRowInitializer();

  // Delete from oid_index
  auto *index_pr = oid_index_init.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *const>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table_oid);
  classes_oid_index_->Delete(txn, *index_pr, index_results[0]);

  // Delete from name_index
  index_pr = name_index_init.InitializeRow(buffer);
  *(reinterpret_cast<storage::VarlenEntry *const>(index_pr->AccessForceNotNull(0))) = name_varlen;
  *(reinterpret_cast<namespace_oid_t *>(index_pr->AccessForceNotNull(1))) = ns_oid;
  classes_name_index_->Delete(txn, *index_pr, index_results[0]);

  // Delete from namespace_index
  index_pr = ns_index_init.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *const>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(ns_oid);
  classes_namespace_index_->Delete(txn, *index_pr, index_results[0]);

  // Get the attributes we need for delete
  auto *const schema_ptr =
      *(reinterpret_cast<const Schema *const *const>(table_pr->AccessForceNotNull(pr_map[REL_SCHEMA_COL_OID])));
  auto *const table_ptr =
      *(reinterpret_cast<storage::SqlTable *const *const>(table_pr->AccessForceNotNull(pr_map[REL_PTR_COL_OID])));

  // Everything succeeded from an MVCC standpoint, register deferred
  // Register a deferred action for the GC with txn manager. See base function comment.
  auto *const txn_manager = txn->GetTransactionManager();
  txn->RegisterCommitAction([=]() {
    txn_manager->DeferAction([=]() {
      // Defer an action upon commit to delete the table. Delete index will need a double deferral.
      delete schema_ptr;
      delete table_ptr;
    });
  });

  delete[] buffer;
  return true;
}

std::pair<uint32_t, postgres::ClassKind> DatabaseCatalog::GetClassOidKind(transaction::TransactionContext *const txn,
                                                                          const namespace_oid_t ns_oid,
                                                                          const std::string &name) {
  std::vector<storage::TupleSlot> index_results;
  auto name_pri = classes_name_index_->GetProjectedRowInitializer();

  // Create the necessary varlen for storage operations
  storage::VarlenEntry name_varlen;
  byte *varlen_contents = nullptr;
  if (name.size() > storage::VarlenEntry::InlineThreshold()) {
    varlen_contents = common::AllocationUtil::AllocateAligned(name.size());
    std::memcpy(varlen_contents, name.data(), name.size());
    name_varlen = storage::VarlenEntry::Create(varlen_contents, static_cast<uint>(name.size()), true);
  } else {
    name_varlen = storage::VarlenEntry::CreateInline(reinterpret_cast<const byte *const>(name.data()),
                                                     static_cast<uint>(name.size()));
  }

  // Name is a larger projected row (16-byte key vs 4-byte key), sow we can reuse
  // the buffer for both index operations if we allocate to the larger one.
  auto *const buffer = common::AllocationUtil::AllocateAligned(name_pri.ProjectedRowSize());
  auto pr = name_pri.InitializeRow(buffer);
  *(reinterpret_cast<storage::VarlenEntry *>(pr->AccessForceNotNull(0))) = name_varlen;
  *(reinterpret_cast<namespace_oid_t *>(pr->AccessForceNotNull(1))) = ns_oid;

  classes_name_index_->ScanKey(*txn, *pr, &index_results);
  if (varlen_contents != nullptr) {
    delete[] varlen_contents;
  }

  if (index_results.empty()) {
    delete[] buffer;
    // If the OID is invalid, we don't care the class kind and return a random one.
    return std::make_pair(0, postgres::ClassKind::REGULAR_TABLE);
  }
  TERRIER_ASSERT(index_results.size() == 1, "name not unique in classes_name_index_");

  const auto table_pri = classes_->InitializerForProjectedRow({RELOID_COL_OID, RELKIND_COL_OID}).first;
  TERRIER_ASSERT(table_pri.ProjectedRowSize() <= name_pri.ProjectedRowSize(),
                 "I want to reuse this buffer because I'm lazy and malloc is slow but it needs to be big enough.");
  pr = table_pri.InitializeRow(buffer);
  const auto result UNUSED_ATTRIBUTE = classes_->Select(txn, index_results[0], pr);
  TERRIER_ASSERT(result, "Index already verified visibility. This shouldn't fail.");

  // This code assumes ordering of attribute by size in the ProjectedRow (size of kind is smaller than size of oid)
  auto oid = *(reinterpret_cast<const uint32_t *const>(pr->AccessForceNotNull(0)));
  auto kind = *(reinterpret_cast<const postgres::ClassKind *const>(pr->AccessForceNotNull(1)));

  delete[] buffer;

  return std::make_pair(oid, kind);
}

table_oid_t DatabaseCatalog::GetTableOid(transaction::TransactionContext *const txn, const namespace_oid_t ns,
                                         const std::string &name) {
  auto oid_pair = GetClassOidKind(txn, ns, name);
  if (oid_pair.second != postgres::ClassKind::REGULAR_TABLE) {
    // User called GetTableOid on an object that doesn't have type REGULAR_TABLE
    return INVALID_TABLE_OID;
  }
  return table_oid_t(oid_pair.first);
}

/**
 * Inform the catalog of where the underlying storage for a table is
 * @param table OID in the catalog
 * @param table_ptr to the memory where the storage is
 * @return whether the operation was successful
 * @warning The table pointer that is passed in must be on the heap as the
 * catalog will take ownership of it and schedule its deletion with the GC
 * at the appropriate time.
 */
bool DatabaseCatalog::SetTablePointer(transaction::TransactionContext *txn, table_oid_t table,
                                      storage::SqlTable *table_ptr) {
  TERRIER_ASSERT(table_ptr != nullptr, "Why are you inserting nullptr here? That seems wrong.");
  std::vector<storage::TupleSlot> index_results;
  auto oid_pri = classes_oid_index_->GetProjectedRowInitializer();

  // Do not need to store the projection map because it is only a single column
  auto pr_init = classes_->InitializerForProjectedRow({REL_PTR_COL_OID}).first;
  TERRIER_ASSERT(pr_init.ProjectedRowSize() >= oid_pri.ProjectedRowSize(), "Buffer must allocated to fit largest PR");
  auto *const buffer = common::AllocationUtil::AllocateAligned(pr_init.ProjectedRowSize());
  auto *key_pr = oid_pri.InitializeRow(buffer);

  // Find the entry using the index
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table);
  classes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with a table_oid.
    // That implies that it was visible to us. Maybe the table was dropped or renamed twice by the same txn?
    delete[] buffer;
    return false;
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  delete[] buffer;

  auto *update_redo = txn->StageWrite(db_oid_, CLASS_TABLE_OID, pr_init);
  auto *update_pr = update_redo->Delta();
  auto *const table_ptr_ptr = update_pr->AccessForceNotNull(0);
  *(reinterpret_cast<uintptr_t *>(table_ptr_ptr)) = reinterpret_cast<uintptr_t>(table_ptr);

  return classes_->Update(txn, update_redo);
}

/**
 * Obtain the storage pointer for a SQL table
 * @param table to which we want the storage object
 * @return the storage object corresponding to the passed OID
 */
common::ManagedPointer<storage::SqlTable> DatabaseCatalog::GetTable(transaction::TransactionContext *txn,
                                                                    table_oid_t table) {
  auto ptr_pair = GetClassPtrKind(txn, static_cast<uint32_t>(table));
  if (ptr_pair.second != postgres::ClassKind::REGULAR_TABLE) {
    // User called GetTable with an OID for an object that doesn't have type REGULAR_TABLE
    return common::ManagedPointer<storage::SqlTable>(nullptr);
  }
  return common::ManagedPointer(reinterpret_cast<storage::SqlTable *>(ptr_pair.first));
}

bool DatabaseCatalog::RenameTable(transaction::TransactionContext *txn, table_oid_t table, const std::string &name) {
  // TODO(John): Implement
  TERRIER_ASSERT(false, "Not implemented");
  return false;
}

bool DatabaseCatalog::UpdateSchema(transaction::TransactionContext *txn, table_oid_t table, Schema *new_schema) {
  // TODO(John): Implement
  TERRIER_ASSERT(false, "Not implemented");
  return false;
}

const Schema &DatabaseCatalog::GetSchema(transaction::TransactionContext *txn, table_oid_t table) {
  auto ptr_pair = GetClassSchemaPtrKind(txn, static_cast<uint32_t>(table));
  TERRIER_ASSERT(ptr_pair.second == postgres::ClassKind::REGULAR_TABLE, "Requested a table schema for a non-table");
  return *reinterpret_cast<Schema *>(ptr_pair.first);
}

std::vector<constraint_oid_t> DatabaseCatalog::GetConstraints(transaction::TransactionContext *txn, table_oid_t table) {
  // TODO(John): Implement
  TERRIER_ASSERT(false, "Not implemented");
  return {};
}

std::vector<index_oid_t> DatabaseCatalog::GetIndexes(transaction::TransactionContext *txn, table_oid_t oid) {
  std::vector<index_oid_t> index_oids;
  std::vector<storage::TupleSlot> index_scan_results;

  // Initialize PR for index scan
  auto oid_pri = indexes_table_index_->GetProjectedRowInitializer();

  // Do not need projection map when there is only one column
  auto pr_init = indexes_->InitializerForProjectedRow({INDOID_COL_OID}).first;
  TERRIER_ASSERT(pr_init.ProjectedRowSize() >= oid_pri.ProjectedRowSize(),
                 "Buffer must be allocated to fit largest PR");
  auto *const buffer = common::AllocationUtil::AllocateAligned(pr_init.ProjectedRowSize());

  // Find all entries for the given table using the index
  auto *key_pr = oid_pri.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(oid);
  indexes_table_index_->ScanKey(*txn, *key_pr, &index_scan_results);

  // If we found no indexes, return an empty list
  if (index_scan_results.empty()) {
    return index_oids;
  }

  auto *select_pr = pr_init.InitializeRow(buffer);
  for (auto &slot : index_scan_results) {
    const auto result UNUSED_ATTRIBUTE = classes_->Select(txn, slot, select_pr);
    TERRIER_ASSERT(result, "Index already verified visibility. This shouldn't fail.");
    index_oids.emplace_back(*(reinterpret_cast<index_oid_t *>(select_pr->AccessForceNotNull(0))));
  }
  delete[] buffer;
  return index_oids;
}

index_oid_t DatabaseCatalog::CreateIndex(transaction::TransactionContext *txn, namespace_oid_t ns,
                                         const std::string &name, table_oid_t table, const IndexSchema &schema) {
  const index_oid_t index_oid = static_cast<index_oid_t>(next_oid_++);
  return CreateIndexEntry(txn, ns, table, index_oid, name, schema) ? index_oid : INVALID_INDEX_OID;
}

bool DatabaseCatalog::DeleteIndex(transaction::TransactionContext *txn, index_oid_t index) {
  std::vector<storage::TupleSlot> index_results;
  // Initialize PRs for pg_class
  auto class_oid_pri = classes_oid_index_->GetProjectedRowInitializer();
  auto [class_pr_init, class_pr_map] = classes_->InitializerForProjectedRow(PG_CLASS_ALL_COL_OIDS);

  // Allocate buffer for largest PR
  TERRIER_ASSERT(class_pr_init.ProjectedRowSize() >= class_oid_pri.ProjectedRowSize(),
                 "Buffer must be allocated for largest ProjectedRow size");
  auto *const buffer = common::AllocationUtil::AllocateAligned(class_pr_init.ProjectedRowSize());
  auto *key_pr = class_oid_pri.InitializeRow(buffer);

  // Find the entry using the index
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(index);
  classes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with an index_oid.
    // That implies that it was visible to us. Maybe the index was dropped or renamed twice by the same txn?
    delete[] buffer;
    return false;
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  // Select the tuple out of the table before deletion. We need the attributes to do index deletions later
  auto *table_pr = class_pr_init.InitializeRow(buffer);
  auto result = classes_->Select(txn, index_results[0], table_pr);
  TERRIER_ASSERT(result, "Select must succeed if the index scan gave a visible result.");

  // Delete from pg_classes table
  txn->StageDelete(db_oid_, CLASS_TABLE_OID, index_results[0]);
  result = classes_->Delete(txn, index_results[0]);
  if (!result) {
    // write-write conflict. Someone beat us to this operation.
    delete[] buffer;
    return false;
  }

  // Get the attributes we need for pg_class indexes
  table_oid_t table_oid =
      *(reinterpret_cast<const table_oid_t *const>(table_pr->AccessForceNotNull(class_pr_map[RELOID_COL_OID])));
  TERRIER_ASSERT(CLASS_TABLE_OID == table_oid,
                 "table oid from pg_classes did not match what was found by the index scan from the argument.");
  const namespace_oid_t ns_oid = *(
      reinterpret_cast<const namespace_oid_t *const>(table_pr->AccessForceNotNull(class_pr_map[RELNAMESPACE_COL_OID])));
  const storage::VarlenEntry name_varlen = *(
      reinterpret_cast<const storage::VarlenEntry *const>(table_pr->AccessForceNotNull(class_pr_map[RELNAME_COL_OID])));

  const auto class_oid_index_init = classes_oid_index_->GetProjectedRowInitializer();
  const auto class_name_index_init = classes_name_index_->GetProjectedRowInitializer();
  const auto class_ns_index_init = classes_namespace_index_->GetProjectedRowInitializer();

  // Delete from classes_oid_index_
  auto *index_pr = class_oid_index_init.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *const>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table_oid);
  classes_oid_index_->Delete(txn, *index_pr, index_results[0]);

  // Delete from classes_name_index_
  index_pr = class_name_index_init.InitializeRow(buffer);
  *(reinterpret_cast<storage::VarlenEntry *const>(index_pr->AccessForceNotNull(0))) = name_varlen;
  *(reinterpret_cast<namespace_oid_t *>(index_pr->AccessForceNotNull(1))) = ns_oid;
  classes_name_index_->Delete(txn, *index_pr, index_results[0]);

  // Delete from classes_namespace_index_
  index_pr = class_ns_index_init.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *const>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(ns_oid);
  classes_namespace_index_->Delete(txn, *index_pr, index_results[0]);

  // Get the attributes we need for the deferred delete
  // TODO(Gus, John): See TODO in deferred actions lambda below
  //  auto *const schema_ptr =
  //      *(reinterpret_cast<const index::IndexSchema *const
  //      *const>(table_pr->AccessForceNotNull(pr_map[REL_SCHEMA_COL_OID])));
  auto *const index_ptr =
      *(reinterpret_cast<storage::SqlTable *const *const>(table_pr->AccessForceNotNull(class_pr_map[REL_PTR_COL_OID])));

  // Now we need to delete from pg_index and its indexes
  // Initialize PRs for pg_index
  auto index_oid_pr = indexes_oid_index_->GetProjectedRowInitializer();
  auto index_table_pr = indexes_table_index_->GetProjectedRowInitializer();
  auto [index_pr_init, index_pr_map] = indexes_->InitializerForProjectedRow({INDOID_COL_OID, INDRELID_COL_OID});
  TERRIER_ASSERT((class_pr_init.ProjectedRowSize() >= index_pr_init.ProjectedRowSize()) &&
                     (class_pr_init.ProjectedRowSize() >= index_oid_pr.ProjectedRowSize()) &&
                     (class_pr_init.ProjectedRowSize() >= index_table_pr.ProjectedRowSize()),
                 "Buffer must be allocated for largest ProjectedRow size");

  // Find the entry in pg_index using the oid index
  index_results.clear();
  key_pr = index_oid_pr.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(index);
  indexes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with an index_oid.
    // That implies that it was visible to us. Maybe the index was dropped or renamed twice by the same txn?
    delete[] buffer;
    return false;
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  // Select the tuple out of pg_index before deletion. We need the attributes to do index deletions later
  table_pr = index_pr_init.InitializeRow(buffer);
  result = indexes_->Select(txn, index_results[0], table_pr);
  TERRIER_ASSERT(result, "Select must succeed if the index scan gave a visible result.");

  // Confirm we got the right entry in pg_index
  const index_oid_t index_oid =
      *(reinterpret_cast<const index_oid_t *const>(table_pr->AccessForceNotNull(index_pr_map[INDOID_COL_OID])));
  TERRIER_ASSERT(index == index_oid,
                 "index oid from pg_index did not match what was found by the index scan from the argument.");

  // Delete from pg_index table
  txn->StageDelete(db_oid_, INDEX_TABLE_OID, index_results[0]);
  result = indexes_->Delete(txn, index_results[0]);
  TERRIER_ASSERT(
      result,
      "Delete from pg_index should always succeed as write-write conflicts are detected during delete from pg_class");

  // Get the table oid
  table_oid =
      *(reinterpret_cast<const table_oid_t *const>(table_pr->AccessForceNotNull(index_pr_map[INDRELID_COL_OID])));

  // Delete from indexes_oid_index
  index_pr = index_oid_pr.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *const>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(index);
  indexes_oid_index_->Delete(txn, *index_pr, index_results[0]);

  // Delete from indexes_table_index
  index_pr = index_table_pr.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *const>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table_oid);
  indexes_table_index_->Delete(txn, *index_pr, index_results[0]);

  // Everything succeeded from an MVCC standpoint, so register a deferred action for the GC to delete the index with txn
  // manager. See base function comment.
  auto *const txn_manager = txn->GetTransactionManager();
  txn->RegisterCommitAction([=]() {
    txn_manager->DeferAction([=]() {
      txn_manager->DeferAction([=]() {
        // Defer an action upon commit to delete the table. Delete index needs a double deferr]al because we need to
        // guarantee that all the defered actions of transactions that are currently running are executed BEFORE we
        // actually delete the index, as those defered actions could also touch the index.
        // TODO(Gus, John): If the catalog performs a deep copy of the index schema when we call CreateIndex, then we
        // need to delete the schema here. Currently the caller gives us an index pointer which we store in the catalog,
        // so I'm not going to delete it here. delete schema_ptr;
        delete index_ptr;
      });
    });
  });

  delete[] buffer;
  return true;
}

bool DatabaseCatalog::SetIndexPointer(transaction::TransactionContext *txn, index_oid_t index,
                                      storage::index::Index *index_ptr) {
  TERRIER_ASSERT(index_ptr != nullptr, "Why are you inserting nullptr here? That seems wrong.");
  std::vector<storage::TupleSlot> index_results;
  auto oid_pri = classes_oid_index_->GetProjectedRowInitializer();

  // Do not need to store the projection map because it is only a single column
  auto pr_init = classes_->InitializerForProjectedRow({REL_PTR_COL_OID}).first;
  TERRIER_ASSERT(pr_init.ProjectedRowSize() >= oid_pri.ProjectedRowSize(), "Buffer must allocated to fit largest PR");
  auto *const buffer = common::AllocationUtil::AllocateAligned(pr_init.ProjectedRowSize());
  auto *key_pr = oid_pri.InitializeRow(buffer);

  // Find the entry using the index
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(index);
  classes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with an index_oid.
    // That implies that it was visible to us. Maybe the index was dropped or renamed twice by the same txn?
    delete[] buffer;
    return false;
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  delete[] buffer;

  auto *update_redo = txn->StageWrite(db_oid_, CLASS_TABLE_OID, pr_init);
  auto *update_pr = update_redo->Delta();
  auto *const table_ptr_ptr = update_pr->AccessForceNotNull(0);
  *(reinterpret_cast<uintptr_t *>(table_ptr_ptr)) = reinterpret_cast<uintptr_t>(index_ptr);

  return classes_->Update(txn, update_redo);
}

common::ManagedPointer<storage::index::Index> DatabaseCatalog::GetIndex(transaction::TransactionContext *txn,
                                                                        index_oid_t index) {
  auto ptr_pair = GetClassPtrKind(txn, static_cast<uint32_t>(index));
  if (ptr_pair.second != postgres::ClassKind::INDEX) {
    // User called GetTable with an OID for an object that doesn't have type REGULAR_TABLE
    return common::ManagedPointer<storage::index::Index>(nullptr);
  }
  return common::ManagedPointer(reinterpret_cast<storage::index::Index *>(ptr_pair.first));
}

index_oid_t DatabaseCatalog::GetIndexOid(transaction::TransactionContext *txn, namespace_oid_t ns,
                                         const std::string &name) {
  auto oid_pair = GetClassOidKind(txn, ns, name);
  if (oid_pair.second != postgres::ClassKind::INDEX) {
    // User called GetIndexOid on an object that doesn't have type INDEX
    return INVALID_INDEX_OID;
  }
  return index_oid_t(oid_pair.first);
}

const IndexSchema &DatabaseCatalog::GetIndexSchema(transaction::TransactionContext *txn, index_oid_t index) {
  auto ptr_pair = GetClassSchemaPtrKind(txn, static_cast<uint32_t>(index));
  TERRIER_ASSERT(ptr_pair.second == postgres::ClassKind::INDEX, "Requested an index schema for a non-index");
  return *reinterpret_cast<IndexSchema *>(ptr_pair.first);
}

void DatabaseCatalog::TearDown(transaction::TransactionContext *txn) {
  std::vector<parser::AbstractExpression *> expressions;
  std::vector<Schema *> table_schemas;
  std::vector<storage::SqlTable *> tables;
  std::vector<IndexSchema *> index_schemas;
  std::vector<storage::index::Index *> indexes;

  std::vector<col_oid_t> col_oids;

  // pg_class (schemas & objects) [this is the largest projection]
  col_oids.emplace_back(RELKIND_COL_OID);
  col_oids.emplace_back(REL_SCHEMA_COL_OID);
  col_oids.emplace_back(REL_PTR_COL_OID);
  auto [pci, pm] = classes_->InitializerForProjectedColumns(col_oids, 100);

  byte *buffer = common::AllocationUtil::AllocateAligned(pci.ProjectedColumnsSize());
  auto pc = pci.Initialize(buffer);

  // Fetch pointers to the start each in the projected columns
  auto classes = reinterpret_cast<postgres::ClassKind *>(pc->ColumnStart(pm[RELKIND_COL_OID]));
  auto schemas = reinterpret_cast<void **>(pc->ColumnStart(pm[REL_SCHEMA_COL_OID]));
  auto objects = reinterpret_cast<void **>(pc->ColumnStart(pm[REL_PTR_COL_OID]));

  // Scan the table
  auto table_iter = classes_->begin();
  while (table_iter != classes_->end()) {
    classes_->Scan(txn, &table_iter, pc);
    for (uint i = 0; i < pc->NumTuples(); i++) {
      switch (classes[i]) {
        case postgres::ClassKind::REGULAR_TABLE:
          table_schemas.emplace_back(reinterpret_cast<Schema *>(schemas[i]));
          tables.emplace_back(reinterpret_cast<storage::SqlTable *>(objects[i]));
          break;
        case postgres::ClassKind::INDEX:
          index_schemas.emplace_back(reinterpret_cast<IndexSchema *>(schemas[i]));
          indexes.emplace_back(reinterpret_cast<storage::index::Index *>(objects[i]));
          break;
        default:
          throw std::runtime_error("Unimplemented destructor needed");
      }
    }
  }

  // pg_attribute (expressions)
  col_oids.clear();
  col_oids.emplace_back(ADBIN_COL_OID);
  std::tie(pci, pm) = columns_->InitializerForProjectedColumns(col_oids, 100);
  pc = pci.Initialize(buffer);

  auto exprs = reinterpret_cast<parser::AbstractExpression **>(pc->ColumnStart(0));

  table_iter = columns_->begin();
  while (table_iter != columns_->end()) {
    columns_->Scan(txn, &table_iter, pc);

    for (uint i = 0; i < pc->NumTuples(); i++) {
      expressions.emplace_back(exprs[i]);
    }
  }

  // pg_constraint (expressions)
  col_oids.clear();
  col_oids.emplace_back(CONBIN_COL_OID);
  std::tie(pci, pm) = constraints_->InitializerForProjectedColumns(col_oids, 100);
  pc = pci.Initialize(buffer);

  exprs = reinterpret_cast<parser::AbstractExpression **>(pc->ColumnStart(0));

  table_iter = constraints_->begin();
  while (table_iter != constraints_->end()) {
    constraints_->Scan(txn, &table_iter, pc);

    for (uint i = 0; i < pc->NumTuples(); i++) {
      expressions.emplace_back(exprs[i]);
    }
  }

  // No new transactions can see these object but there may be deferred index
  // and other operation.  Therefore, we need to defer the deallocation on delete
  txn->RegisterCommitAction([=, tables{std::move(tables)}, indexes{std::move(indexes)},
                             table_schemas{std::move(table_schemas)}, index_schemas{std::move(index_schemas)},
                             expressions{std::move(expressions)}] {
    txn->GetTransactionManager()->DeferAction(
        [=, tables{std::move(tables)}, indexes{std::move(indexes)}, table_schemas{std::move(table_schemas)},
         index_schemas{std::move(index_schemas)}, expressions{std::move(expressions)}] {
          for (auto table : tables) delete table;

          for (auto index : indexes) delete index;

          for (auto schema : table_schemas) delete schema;

          for (auto schema : index_schemas) delete schema;

          for (auto expr : expressions) delete expr;
        });
  });
}

bool DatabaseCatalog::CreateIndexEntry(transaction::TransactionContext *const txn, const namespace_oid_t ns_oid,
                                       const table_oid_t table_oid, const index_oid_t index_oid,
                                       const std::string &name, const IndexSchema &schema) {
  auto idx_schema = new IndexSchema(schema);
  // First, insert into pg_class
  auto [pr_init, pr_map] = classes_->InitializerForProjectedRow(PG_CLASS_ALL_COL_OIDS);

  auto *const class_insert_redo = txn->StageWrite(db_oid_, CLASS_TABLE_OID, pr_init);
  auto *const class_insert_pr = class_insert_redo->Delta();

  // Write the index_oid into the PR
  auto index_oid_offset = pr_map[RELOID_COL_OID];
  auto *index_oid_ptr = class_insert_pr->AccessForceNotNull(index_oid_offset);
  *(reinterpret_cast<uint32_t *>(index_oid_ptr)) = static_cast<uint32_t>(index_oid);

  // Create the necessary varlen for storage operations
  storage::VarlenEntry name_varlen;
  if (name.size() > storage::VarlenEntry::InlineThreshold()) {
    byte *contents = common::AllocationUtil::AllocateAligned(name.size());
    std::memcpy(contents, name.data(), name.size());
    name_varlen = storage::VarlenEntry::Create(contents, static_cast<uint>(name.size()), true);
  } else {
    name_varlen = storage::VarlenEntry::CreateInline(reinterpret_cast<const byte *const>(name.data()),
                                                     static_cast<uint>(name.size()));
  }

  // Write the name into the PR
  const auto name_offset = pr_map[RELNAME_COL_OID];
  auto *const name_ptr = class_insert_pr->AccessForceNotNull(name_offset);
  *(reinterpret_cast<storage::VarlenEntry *>(name_ptr)) = name_varlen;

  // Write the ns_oid into the PR
  const auto ns_offset = pr_map[RELNAMESPACE_COL_OID];
  auto *const ns_ptr = class_insert_pr->AccessForceNotNull(ns_offset);
  *(reinterpret_cast<uint32_t *>(ns_ptr)) = static_cast<uint32_t>(ns_oid);

  // Write the kind into the PR
  const auto kind_offset = pr_map[RELKIND_COL_OID];
  auto *const kind_ptr = class_insert_pr->AccessForceNotNull(kind_offset);
  *(reinterpret_cast<char *>(kind_ptr)) = static_cast<char>(postgres::ClassKind::INDEX);

  // Write the index_schema_ptr into the PR
  const auto index_schema_ptr_offset = pr_map[REL_SCHEMA_COL_OID];
  auto *const index_schema_ptr_ptr = class_insert_pr->AccessForceNotNull(index_schema_ptr_offset);
  *(reinterpret_cast<uintptr_t *>(index_schema_ptr_ptr)) = reinterpret_cast<uintptr_t>(idx_schema);

  // Set next_col_oid to NULL because indexes don't need col_oid
  const auto next_col_oid_offset = pr_map[REL_NEXTCOLOID_COL_OID];
  class_insert_pr->SetNull(next_col_oid_offset);

  // Set index_ptr to NULL because it gets set by execution layer after instantiation
  const auto index_ptr_offset = pr_map[REL_PTR_COL_OID];
  class_insert_pr->SetNull(index_ptr_offset);

  // Insert into pg_class table
  const auto class_tuple_slot = classes_->Insert(txn, class_insert_redo);

  // Now we insert into indexes on pg_class
  // Get PR initializers allocate a buffer from the largest one
  const auto class_oid_index_init = classes_oid_index_->GetProjectedRowInitializer();
  const auto class_name_index_init = classes_name_index_->GetProjectedRowInitializer();
  const auto class_ns_index_init = classes_namespace_index_->GetProjectedRowInitializer();
  TERRIER_ASSERT((class_name_index_init.ProjectedRowSize() >= class_oid_index_init.ProjectedRowSize()) &&
                     (class_name_index_init.ProjectedRowSize() >= class_ns_index_init.ProjectedRowSize()),
                 "Index buffer must be allocated based on the largest PR initializer");
  auto *index_buffer = common::AllocationUtil::AllocateAligned(class_name_index_init.ProjectedRowSize());

  // Insert into oid_index
  auto *index_pr = class_oid_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<uint32_t *>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(index_oid);
  if (!classes_oid_index_->InsertUnique(txn, *index_pr, class_tuple_slot)) {
    // There was an oid conflict and we need to abort.  Free the buffer and
    // return INVALID_TABLE_OID to indicate the database was not created.
    delete[] index_buffer;
    return false;
  }

  // Insert into name_index
  index_pr = class_name_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<storage::VarlenEntry *>(index_pr->AccessForceNotNull(0))) = name_varlen;
  if (!classes_name_index_->InsertUnique(txn, *index_pr, class_tuple_slot)) {
    // There was a name conflict and we need to abort.  Free the buffer and
    // return INVALID_TABLE_OID to indicate the database was not created.
    delete[] index_buffer;
    return false;
  }

  // Insert into namespace_index
  index_pr = class_ns_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<uint32_t *>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(ns_oid);
  const auto result UNUSED_ATTRIBUTE = classes_namespace_index_->Insert(txn, *index_pr, class_tuple_slot);
  TERRIER_ASSERT(!result, "Insertion into non-unique namespace index failed.");

  // Next, insert index metadata into pg_index
  std::tie(pr_init, pr_map) = indexes_->InitializerForProjectedRow(PG_INDEX_ALL_COL_OIDS);
  auto *const indexes_insert_redo = txn->StageWrite(db_oid_, INDEX_TABLE_OID, pr_init);
  auto *const indexes_insert_pr = indexes_insert_redo->Delta();

  // Write the index_oid into the PR
  index_oid_offset = pr_map[INDOID_COL_OID];
  index_oid_ptr = indexes_insert_pr->AccessForceNotNull(index_oid_offset);
  *(reinterpret_cast<uint32_t *>(index_oid_ptr)) = static_cast<uint32_t>(index_oid);

  // Write the table_oid for the table the index is for into the PR
  const auto rel_oid_offset = pr_map[INDRELID_COL_OID];
  auto *const rel_oid_ptr = indexes_insert_pr->AccessForceNotNull(rel_oid_offset);
  *(reinterpret_cast<uint32_t *>(rel_oid_ptr)) = static_cast<uint32_t>(table_oid);

  // Write boolean values to PR
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDISUNIQUE_COL_OID]))) = idx_schema->is_unique_;
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDISPRIMARY_COL_OID]))) =
      idx_schema->is_primary_;
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDISEXCLUSION_COL_OID]))) =
      idx_schema->is_exclusion_;
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDIMMEDIATE_COL_OID]))) =
      idx_schema->is_immediate_;
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDISVALID_COL_OID]))) = idx_schema->is_valid_;
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDISREADY_COL_OID]))) = idx_schema->is_ready_;
  *(reinterpret_cast<bool *>(indexes_insert_pr->AccessForceNotNull(pr_map[INDISLIVE_COL_OID]))) = idx_schema->is_live_;

  // Insert into pg_index table
  const auto indexes_tuple_slot = indexes_->Insert(txn, indexes_insert_redo);

  // Now insert into the indexes on pg_index
  // Get PR initializers and allocate a buffer from the largest one
  const auto indexes_oid_index_init = indexes_oid_index_->GetProjectedRowInitializer();
  const auto indexes_table_index_init = indexes_table_index_->GetProjectedRowInitializer();
  TERRIER_ASSERT((class_name_index_init.ProjectedRowSize() >= indexes_oid_index_init.ProjectedRowSize()) &&
                     (class_name_index_init.ProjectedRowSize() > indexes_table_index_init.ProjectedRowSize()),
                 "Index buffer must be allocated based on the largest PR initializer");

  // Insert into indexes_oid_index
  index_pr = indexes_oid_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<uint32_t *>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(index_oid);
  if (!indexes_oid_index_->InsertUnique(txn, *index_pr, indexes_tuple_slot)) {
    // There was an oid conflict and we need to abort.  Free the buffer and
    // return INVALID_TABLE_OID to indicate the database was not created.
    delete[] index_buffer;
    return false;
  }

  // Insert into (non-unique) indexes_table_index
  index_pr = indexes_table_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<uint32_t *>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table_oid);
  if (!indexes_table_index_->Insert(txn, *index_pr, indexes_tuple_slot)) {
    // There was duplicate value. Free the buffer and
    // return INVALID_TABLE_OID to indicate the database was not created.
    delete[] index_buffer;
    return false;
  }

  // Free the buffer, we are finally done
  delete[] index_buffer;

  return true;
}

type_oid_t DatabaseCatalog::GetTypeOidForType(type::TypeId type) { return type_oid_t(static_cast<uint8_t>(type)); }

void DatabaseCatalog::InsertType(transaction::TransactionContext *txn, type::TypeId internal_type,
                                 const std::string &name, namespace_oid_t namespace_oid, int16_t len, bool by_val,
                                 postgres::Type type_category) {
  std::vector<col_oid_t> table_col_oids;
  table_col_oids.emplace_back(TYPOID_COL_OID);
  table_col_oids.emplace_back(TYPNAME_COL_OID);
  table_col_oids.emplace_back(TYPNAMESPACE_COL_OID);
  table_col_oids.emplace_back(TYPLEN_COL_OID);
  table_col_oids.emplace_back(TYPBYVAL_COL_OID);
  table_col_oids.emplace_back(TYPTYPE_COL_OID);
  auto initializer_pair = types_->InitializerForProjectedRow(table_col_oids);
  auto initializer = initializer_pair.first;
  auto col_map = initializer_pair.second;

  // Stage the write into the table
  auto redo_record = txn->StageWrite(db_oid_, TYPE_TABLE_OID, initializer);
  auto *delta = redo_record->Delta();

  // Populate oid
  auto offset = col_map[TYPOID_COL_OID];
  auto type_oid = GetTypeOidForType(internal_type);
  *(reinterpret_cast<uint32_t *>(delta->AccessForceNotNull(offset))) = static_cast<uint32_t>(type_oid);

  // Populate type name
  offset = col_map[TYPNAME_COL_OID];
  storage::VarlenEntry name_varlen;
  if (name.size() > storage::VarlenEntry::InlineThreshold()) {
    byte *contents = common::AllocationUtil::AllocateAligned(name.size());
    std::memcpy(contents, name.data(), name.size());
    name_varlen = storage::VarlenEntry::Create(contents, static_cast<uint32_t>(name.size()), true);
  } else {
    name_varlen = storage::VarlenEntry::CreateInline(reinterpret_cast<const byte *>(name.data()),
                                                     static_cast<uint32_t>(name.size()));
  }
  *(reinterpret_cast<storage::VarlenEntry *>(delta->AccessForceNotNull(offset))) = name_varlen;

  // Populate namespace
  offset = col_map[TYPNAMESPACE_COL_OID];
  *(reinterpret_cast<uint32_t *>(delta->AccessForceNotNull(offset))) = static_cast<uint32_t>(namespace_oid);

  // Populate len
  offset = col_map[TYPLEN_COL_OID];
  *(reinterpret_cast<int16_t *>(delta->AccessForceNotNull(offset))) = len;

  // Populate byval
  offset = col_map[TYPBYVAL_COL_OID];
  *(reinterpret_cast<bool *>(delta->AccessForceNotNull(offset))) = by_val;

  // Populate type
  offset = col_map[TYPTYPE_COL_OID];
  auto type = static_cast<uint8_t>(type_category);
  *(reinterpret_cast<uint8_t *>(delta->AccessForceNotNull(offset))) = type;

  // Insert into table
  auto tuple_slot = types_->Insert(txn, redo_record);

  // Allocate buffer of largest size needed
  TERRIER_ASSERT((types_name_index_->GetProjectedRowInitializer().ProjectedRowSize() >=
                  types_oid_index_->GetProjectedRowInitializer().ProjectedRowSize()) &&
                     (types_name_index_->GetProjectedRowInitializer().ProjectedRowSize() >=
                      types_namespace_index_->GetProjectedRowInitializer().ProjectedRowSize()),
                 "Buffer must be allocated for largest ProjectedRow size");
  byte *buffer =
      common::AllocationUtil::AllocateAligned(types_name_index_->GetProjectedRowInitializer().ProjectedRowSize());

  // Insert into oid index
  auto oid_index_delta = types_oid_index_->GetProjectedRowInitializer().InitializeRow(buffer);
  auto oid_index_offset = types_oid_index_->GetKeyOidToOffsetMap().at(catalog::indexkeycol_oid_t(1));
  *(reinterpret_cast<uint32_t *>(oid_index_delta->AccessForceNotNull(oid_index_offset))) =
      static_cast<uint32_t>(type_oid);
  auto result UNUSED_ATTRIBUTE = types_oid_index_->InsertUnique(txn, *oid_index_delta, tuple_slot);
  TERRIER_ASSERT(result, "Insert into type oid index should always succeed");

  // Insert into (namespace_oid, name) index
  auto name_index_delta = types_name_index_->GetProjectedRowInitializer().InitializeRow(buffer);
  // Populate namespace
  auto name_index_offset = types_name_index_->GetKeyOidToOffsetMap().at(catalog::indexkeycol_oid_t(1));
  *(reinterpret_cast<uint32_t *>(name_index_delta->AccessForceNotNull(name_index_offset))) =
      static_cast<uint32_t>(namespace_oid);
  // Populate type name
  name_index_offset = types_name_index_->GetKeyOidToOffsetMap().at(catalog::indexkeycol_oid_t(2));
  *(reinterpret_cast<storage::VarlenEntry *>(name_index_delta->AccessForceNotNull(name_index_offset))) = name_varlen;
  result = types_name_index_->InsertUnique(txn, *name_index_delta, tuple_slot);
  TERRIER_ASSERT(result, "Insert into type name index should always succeed");

  // Insert into (non-unique) namespace oid index
  auto namespace_index_delta = types_namespace_index_->GetProjectedRowInitializer().InitializeRow(buffer);
  auto namespace_index_offset = types_namespace_index_->GetKeyOidToOffsetMap().at(catalog::indexkeycol_oid_t(1));
  *(reinterpret_cast<uint32_t *>(namespace_index_delta->AccessForceNotNull(namespace_index_offset))) =
      static_cast<uint32_t>(namespace_oid);
  result = types_namespace_index_->Insert(txn, *name_index_delta, tuple_slot);
  TERRIER_ASSERT(result, "Insert into type namespace index should always succeed");

  delete[] buffer;
}

void DatabaseCatalog::BootstrapTypes(transaction::TransactionContext *txn) {
  InsertType(txn, type::TypeId::INVALID, "INVALID", NAMESPACE_CATALOG_NAMESPACE_OID, 1, true, postgres::Type::BASE);

  InsertType(txn, type::TypeId::BOOLEAN, "BOOLEAN", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(bool), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::TINYINT, "TINYINT", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(int8_t), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::SMALLINT, "SMALLINT", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(int16_t), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::INTEGER, "INTEGER", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(int32_t), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::BIGINT, "BIGINT", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(int64_t), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::DECIMAL, "DECIMAL", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(double), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::TIMESTAMP, "TIMESTAMP", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(type::timestamp_t),
             true, postgres::Type::BASE);

  InsertType(txn, type::TypeId::DATE, "DATE", NAMESPACE_CATALOG_NAMESPACE_OID, sizeof(type::date_t), true,
             postgres::Type::BASE);

  InsertType(txn, type::TypeId::VARCHAR, "VARCHAR", NAMESPACE_CATALOG_NAMESPACE_OID, -1, false, postgres::Type::BASE);

  InsertType(txn, type::TypeId::VARBINARY, "VARBINARY", NAMESPACE_CATALOG_NAMESPACE_OID, -1, false,
             postgres::Type::BASE);
}

bool DatabaseCatalog::CreateTableEntry(transaction::TransactionContext *const txn, const table_oid_t table_oid,
                                       const namespace_oid_t ns_oid, const std::string &name, Schema *schema) {
  auto [pr_init, pr_map] = classes_->InitializerForProjectedRow(PG_CLASS_ALL_COL_OIDS);

  auto *const insert_redo = txn->StageWrite(db_oid_, CLASS_TABLE_OID, pr_init);
  auto *const insert_pr = insert_redo->Delta();

  // Write the ns_oid into the PR
  const auto ns_offset = pr_map[RELNAMESPACE_COL_OID];
  auto *const ns_ptr = insert_pr->AccessForceNotNull(ns_offset);
  *(reinterpret_cast<uint32_t *>(ns_ptr)) = static_cast<uint32_t>(ns_oid);

  // Write the table_oid into the PR
  const auto table_oid_offset = pr_map[RELOID_COL_OID];
  auto *const table_oid_ptr = insert_pr->AccessForceNotNull(table_oid_offset);
  *(reinterpret_cast<uint32_t *>(table_oid_ptr)) = static_cast<uint32_t>(table_oid);

  auto next_col_oid = schema->GetColumns()[-1].GetOid();

  // Write the next_col_oid into the PR
  const auto next_col_oid_offset = pr_map[REL_NEXTCOLOID_COL_OID];
  auto *const next_col_oid_ptr = insert_pr->AccessForceNotNull(next_col_oid_offset);
  *(reinterpret_cast<uint32_t *>(next_col_oid_ptr)) = static_cast<uint32_t>(next_col_oid);

  // Write the schema_ptr into the PR
  const auto schema_ptr_offset = pr_map[REL_SCHEMA_COL_OID];
  auto *const schema_ptr_ptr = insert_pr->AccessForceNotNull(schema_ptr_offset);
  *(reinterpret_cast<uintptr_t *>(schema_ptr_ptr)) = reinterpret_cast<uintptr_t>(schema);

  // Set table_ptr to NULL because it gets set by execution layer after instantiation
  const auto table_ptr_offset = pr_map[REL_PTR_COL_OID];
  insert_pr->SetNull(table_ptr_offset);

  // Write the kind into the PR
  const auto kind_offset = pr_map[RELKIND_COL_OID];
  auto *const kind_ptr = insert_pr->AccessForceNotNull(kind_offset);
  *(reinterpret_cast<char *>(kind_ptr)) = static_cast<char>(postgres::ClassKind::REGULAR_TABLE);

  // Create the necessary varlen for storage operations
  storage::VarlenEntry name_varlen;
  if (name.size() > storage::VarlenEntry::InlineThreshold()) {
    byte *contents = common::AllocationUtil::AllocateAligned(name.size());
    std::memcpy(contents, name.data(), name.size());
    name_varlen = storage::VarlenEntry::Create(contents, static_cast<uint>(name.size()), true);
  } else {
    name_varlen = storage::VarlenEntry::CreateInline(reinterpret_cast<const byte *const>(name.data()),
                                                     static_cast<uint>(name.size()));
  }

  // Write the name into the PR
  const auto name_offset = pr_map[RELNAME_COL_OID];
  auto *const name_ptr = insert_pr->AccessForceNotNull(name_offset);
  *(reinterpret_cast<storage::VarlenEntry *>(name_ptr)) = name_varlen;

  // Insert into pg_class table
  const auto tuple_slot = classes_->Insert(txn, insert_redo);

  // Get PR initializers and allocate a buffer from the largest one
  const auto oid_index_init = classes_oid_index_->GetProjectedRowInitializer();
  const auto name_index_init = classes_name_index_->GetProjectedRowInitializer();
  const auto ns_index_init = classes_namespace_index_->GetProjectedRowInitializer();
  auto *const index_buffer = common::AllocationUtil::AllocateAligned(name_index_init.ProjectedRowSize());

  // Insert into oid_index
  auto *index_pr = oid_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<uint32_t *>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(table_oid);
  if (!classes_oid_index_->InsertUnique(txn, *index_pr, tuple_slot)) {
    // There was an oid conflict and we need to abort.  Free the buffer and
    // return INVALID_TABLE_OID to indicate the database was not created.
    delete[] index_buffer;
    return false;
  }

  // Insert into name_index
  index_pr = name_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<storage::VarlenEntry *>(index_pr->AccessForceNotNull(0))) = name_varlen;
  *(reinterpret_cast<namespace_oid_t *>(index_pr->AccessForceNotNull(1))) = ns_oid;
  if (!classes_name_index_->InsertUnique(txn, *index_pr, tuple_slot)) {
    // There was a name conflict and we need to abort.  Free the buffer and
    // return INVALID_TABLE_OID to indicate the database was not created.
    delete[] index_buffer;
    return false;
  }

  // Insert into namespace_index
  index_pr = ns_index_init.InitializeRow(index_buffer);
  *(reinterpret_cast<uint32_t *>(index_pr->AccessForceNotNull(0))) = static_cast<uint32_t>(ns_oid);
  const auto result UNUSED_ATTRIBUTE = classes_namespace_index_->Insert(txn, *index_pr, tuple_slot);
  TERRIER_ASSERT(!result, "Insertion into non-unique namespace index failed.");

  delete[] index_buffer;

  return true;
}

std::pair<void *, postgres::ClassKind> DatabaseCatalog::GetClassPtrKind(transaction::TransactionContext *txn,
                                                                        uint32_t oid) {
  std::vector<storage::TupleSlot> index_results;

  // Initialize both PR initializers, allocate buffer using size of largest one so we can reuse buffer
  auto oid_pri = classes_oid_index_->GetProjectedRowInitializer();

  // Since these two attributes are fixed size and one is larger than the other we know PTR will be 0 and KIND will be 1
  auto pr_init = classes_->InitializerForProjectedRow({REL_PTR_COL_OID, RELKIND_COL_OID}).first;
  TERRIER_ASSERT(pr_init.ProjectedRowSize() >= oid_pri.ProjectedRowSize(),
                 "Buffer must be allocated to fit largest PR");
  auto *const buffer = common::AllocationUtil::AllocateAligned(pr_init.ProjectedRowSize());

  // Find the entry using the index
  auto *key_pr = oid_pri.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = oid;
  classes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with an oid.
    // That implies that it was visible to us. Maybe the object was dropped or renamed twice by the same txn?
    delete[] buffer;
    return {nullptr, postgres::ClassKind::REGULAR_TABLE};
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  auto *select_pr = pr_init.InitializeRow(buffer);
  const auto result UNUSED_ATTRIBUTE = classes_->Select(txn, index_results[0], select_pr);
  TERRIER_ASSERT(result, "Index already verified visibility. This shouldn't fail.");

  auto *const ptr = *(reinterpret_cast<void *const *const>(select_pr->AccessForceNotNull(0)));
  auto kind = *(reinterpret_cast<const postgres::ClassKind *const>(select_pr->AccessForceNotNull(1)));

  delete[] buffer;
  return {ptr, kind};
}

std::pair<void *, postgres::ClassKind> DatabaseCatalog::GetClassSchemaPtrKind(transaction::TransactionContext *txn,
                                                                              uint32_t oid) {
  std::vector<storage::TupleSlot> index_results;

  // Initialize both PR initializers, allocate buffer using size of largest one so we can reuse buffer
  auto oid_pri = classes_oid_index_->GetProjectedRowInitializer();

  // Since these two attributes are fixed size and one is larger than the other we know PTR will be 0 and KIND will be 1
  auto pr_init = classes_->InitializerForProjectedRow({REL_SCHEMA_COL_OID, RELKIND_COL_OID}).first;
  TERRIER_ASSERT(pr_init.ProjectedRowSize() >= oid_pri.ProjectedRowSize(),
                 "Buffer must be allocated to fit largest PR");
  auto *const buffer = common::AllocationUtil::AllocateAligned(pr_init.ProjectedRowSize());

  // Find the entry using the index
  auto *key_pr = oid_pri.InitializeRow(buffer);
  *(reinterpret_cast<uint32_t *>(key_pr->AccessForceNotNull(0))) = oid;
  classes_oid_index_->ScanKey(*txn, *key_pr, &index_results);
  if (index_results.empty()) {
    // TODO(Matt): we should verify what postgres does in this case
    // Index scan didn't find anything. This seems weird since we were able to enter this function with an oid.
    // That implies that it was visible to us. Maybe the object was dropped or renamed twice by the same txn?
    delete[] buffer;
    return {nullptr, postgres::ClassKind::REGULAR_TABLE};
  }
  TERRIER_ASSERT(index_results.size() == 1, "You got more than one result from a unique index. How did you do that?");

  auto *select_pr = pr_init.InitializeRow(buffer);
  const auto result UNUSED_ATTRIBUTE = classes_->Select(txn, index_results[0], select_pr);
  TERRIER_ASSERT(result, "Index already verified visibility. This shouldn't fail.");

  auto *const ptr = *(reinterpret_cast<void *const *const>(select_pr->AccessForceNotNull(0)));
  auto kind = *(reinterpret_cast<const postgres::ClassKind *const>(select_pr->AccessForceNotNull(1)));

  delete[] buffer;
  return {ptr, kind};
}

}  // namespace terrier::catalog
