#pragma once

#include "catalog/schema.h"
#include "storage/projected_row.h"
#include "storage/sql_table.h"
#include "storage/storage_defs.h"
#include "transaction/transaction_context.h"

namespace terrier::catalog::postgres {
constexpr table_oid_t NAMESPACE_TABLE_OID = table_oid_t(11);
constexpr index_oid_t NAMESPACE_OID_INDEX_OID = index_oid_t(12);
constexpr index_oid_t NAMESPACE_NAME_INDEX_OID = index_oid_t(13);
constexpr namespace_oid_t NAMESPACE_CATALOG_NAMESPACE_OID = namespace_oid_t(14);
constexpr namespace_oid_t NAMESPACE_DEFAULT_NAMESPACE_OID = namespace_oid_t(15);
/*
 * Column names of the form "NSP[name]_COL_OID" are present in the PostgreSQL
 * catalog specification and columns of the form "NSP_[name]_COL_OID" are
 * terrier-specific addtions (generally pointers to internal objects).
 */
constexpr col_oid_t NSPOID_COL_OID = col_oid_t(1);   // INTEGER (pkey)
constexpr col_oid_t NSPNAME_COL_OID = col_oid_t(2);  // VARCHAR

constexpr std::array<col_oid_t, 2> PG_NAMESPACE_ALL_COL_OIDS{NSPOID_COL_OID, NSPNAME_COL_OID};
}  // namespace terrier::catalog::postgres
