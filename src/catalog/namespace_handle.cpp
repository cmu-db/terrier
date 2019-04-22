#include "catalog/namespace_handle.h"
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "catalog/catalog.h"
#include "catalog/schema.h"
#include "loggers/catalog_logger.h"
#include "storage/block_layout.h"
#include "storage/sql_table.h"
#include "storage/storage_defs.h"
#include "type/type_id.h"

namespace terrier::catalog {

const std::vector<SchemaCol> NamespaceHandle::schema_cols_ = {{0, "oid", type::TypeId::INTEGER},
                                                              {1, "nspname", type::TypeId::VARCHAR}};

const std::vector<SchemaCol> NamespaceHandle::unused_schema_cols_ = {
    {2, "nspowner", type::TypeId::INTEGER},
    {3, "nspacl", type::TypeId::VARCHAR},
};

std::shared_ptr<NamespaceEntry> NamespaceHandle::GetNamespaceEntry(transaction::TransactionContext *txn,
                                                                   namespace_oid_t oid) {
  std::vector<type::TransientValue> search_vec, ret_row;
  search_vec.push_back(type::TransientValueFactory::GetInteger(!oid));
  ret_row = pg_namespace_hrw_->FindRow(txn, search_vec);
  return std::make_shared<NamespaceEntry>(oid, pg_namespace_hrw_, std::move(ret_row));
}

std::shared_ptr<NamespaceEntry> NamespaceHandle::GetNamespaceEntry(transaction::TransactionContext *txn,
                                                                   const std::string &name) {
  std::vector<type::TransientValue> search_vec, ret_row;
  search_vec.push_back(type::TransientValueFactory::GetNull(type::TypeId::INTEGER));
  search_vec.push_back(type::TransientValueFactory::GetVarChar(name));
  ret_row = pg_namespace_hrw_->FindRow(txn, search_vec);
  if (ret_row.empty()) {
    return nullptr;
  }
  namespace_oid_t oid(type::TransientValuePeeker::PeekInteger(ret_row[0]));
  return std::make_shared<NamespaceEntry>(oid, pg_namespace_hrw_, std::move(ret_row));
}

namespace_oid_t NamespaceHandle::NameToOid(transaction::TransactionContext *txn, const std::string &name) {
  auto nse = GetNamespaceEntry(txn, name);
  return namespace_oid_t(type::TransientValuePeeker::PeekInteger(nse->GetColumn(0)));
}

TableHandle NamespaceHandle::GetTableHandle(transaction::TransactionContext *txn, const std::string &nsp_name) {
  auto ns_oid = NameToOid(txn, nsp_name);
  return GetTableHandle(txn, ns_oid);
}

TableHandle NamespaceHandle::GetTableHandle(transaction::TransactionContext *txn, namespace_oid_t ns_oid) {
  std::string pg_class("pg_class");
  std::string pg_namespace("pg_namespace");
  std::string pg_tablespace("pg_tablespace");
  return TableHandle(catalog_, ns_oid, catalog_->GetCatalogTable(db_oid_, pg_class),
                     catalog_->GetCatalogTable(db_oid_, pg_namespace),
                     catalog_->GetCatalogTable(db_oid_, pg_tablespace));
}

void NamespaceHandle::AddEntry(transaction::TransactionContext *txn, const std::string &name) {
  std::vector<type::TransientValue> row;

  row.emplace_back(type::TransientValueFactory::GetInteger(catalog_->GetNextOid()));
  row.emplace_back(type::TransientValueFactory::GetVarChar(name));
  catalog_->SetUnusedColumns(&row, NamespaceHandle::unused_schema_cols_);
  pg_namespace_hrw_->InsertRow(txn, row);
}

bool NamespaceHandle::DeleteEntry(transaction::TransactionContext *txn, const std::shared_ptr<NamespaceEntry> &entry) {
  std::vector<type::TransientValue> search_vec;
  auto ns_oid_int = entry->GetIntegerColumn("oid");
  // get the oid of this row
  search_vec.emplace_back(type::TransientValueFactory::GetInteger(ns_oid_int));

  // lookup and get back the projected column. Recover the tuple_slot
  auto proj_col_p = pg_namespace_hrw_->FindRowProjCol(txn, search_vec);
  auto tuple_slot_p = proj_col_p->TupleSlots();
  // delete
  bool status = pg_namespace_hrw_->GetSqlTable()->Delete(txn, *tuple_slot_p);
  delete[] reinterpret_cast<byte *>(proj_col_p);
  return status;
}

SqlTableRW *NamespaceHandle::Create(transaction::TransactionContext *txn, Catalog *catalog, db_oid_t db_oid,
                                    const std::string &name) {
  catalog::SqlTableRW *storage_table;

  // get an oid
  table_oid_t storage_table_oid(catalog->GetNextOid());

  // uninitialized storage
  storage_table = new catalog::SqlTableRW(storage_table_oid);

  // columns we use
  for (auto col : NamespaceHandle::schema_cols_) {
    storage_table->DefineColumn(col.col_name, col.type_id, false, col_oid_t(catalog->GetNextOid()));
  }

  // columns we don't use
  for (auto col : NamespaceHandle::unused_schema_cols_) {
    storage_table->DefineColumn(col.col_name, col.type_id, false, col_oid_t(catalog->GetNextOid()));
  }
  // now actually create, with the provided schema
  storage_table->Create();
  catalog->AddToMaps(db_oid, storage_table_oid, name, storage_table);
  // catalog->AddColumnsToPGAttribute(txn, db_oid, storage_table->GetSqlTable());
  return storage_table;
}

}  // namespace terrier::catalog
