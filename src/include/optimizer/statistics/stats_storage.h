#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "catalog/catalog_accessor.h"
#include "catalog/catalog_defs.h"
#include "common/hash_util.h"
#include "common/macros.h"
#include "common/managed_pointer.h"
#include "optimizer/statistics/column_stats.h"
#include "optimizer/statistics/table_stats.h"

namespace noisepage::optimizer {
/**
 * Hashable type for database and table oid pair
 */
using StatsStorageKey = std::pair<noisepage::catalog::db_oid_t, noisepage::catalog::table_oid_t>;
}  // namespace noisepage::optimizer

namespace std {  // NOLINT
/**
 * template for std::hash of StatsStorageKey
 */
template <>
struct hash<noisepage::optimizer::StatsStorageKey> {
  /**
   * Hashes a StatsStorageKey object.
   * @param stats_storage_key - StatsStorageKey object
   * @return the hash for the StatsStorageKey
   */
  size_t operator()(const noisepage::optimizer::StatsStorageKey &stats_storage_key) const {
    noisepage::common::hash_t hash = noisepage::common::HashUtil::Hash(get<0>(stats_storage_key));
    hash =
        noisepage::common::HashUtil::CombineHashes(hash, noisepage::common::HashUtil::Hash(get<1>(stats_storage_key)));
    return hash;
  }
};

/**
 * template for std::equal_to of StatsStorageKey
 */
template <>
struct equal_to<noisepage::optimizer::StatsStorageKey> {
  /**
   * Checks for equality between two StatsStorageKey objects
   * @param lhs - StatsStorageKey on left side of equality
   * @param rhs - StatsStorageKey on right side of equality
   * @return whether the StatsStorageKey objects are equal
   */
  bool operator()(const noisepage::optimizer::StatsStorageKey &lhs,
                  const noisepage::optimizer::StatsStorageKey &rhs) const {
    return get<0>(lhs) == get<0>(rhs) && get<1>(lhs) == get<1>(rhs);
  }
};
}  // namespace std

namespace noisepage::optimizer {
/**
 * Manages all the existing table stats objects. Stores them in an
 * unordered map and keeps track of them using their database and table oids. Can
 * add, update, or delete table stats objects from the storage map.
 */
class StatsStorage {
 public:

  /**
   * Using given database and table ids,
   * select a pointer to the TableStats objects in the table stats storage map.
   * @param database_id - oid of database
   * @param table_id - oid of table
   * @return pointer to a TableStats object
   */
  common::ManagedPointer<TableStats> GetTableStats(catalog::db_oid_t database_id, catalog::table_oid_t table_id, catalog::CatalogAccessor *accessor);

 private:
  /**
   * Inserts a TableStats pointer in the table stats storage map.
   * (note: you must std::move the TableStats object you pass in.)
   * @param database_id - oid of database
   * @param table_id - oid of table
   * @param table_stats - TableStats object to be inserted
   */
  void InsertTableStats(catalog::db_oid_t database_id, catalog::table_oid_t table_id,
                        std::unique_ptr<TableStats> table_stats);

  /**
   * Remove a TableStats object.
   * @param database_id - oid of database
   * @param table_id - oid of table
   * @return whether TableStats object was successfully removed
   */
  void DeleteTableStats(catalog::db_oid_t database_id, catalog::table_oid_t table_id);

 private:
  /**
   * The following tests check to make sure the protected insert/delete functions work.
   */
  FRIEND_TEST(StatsStorageTests, GetTableStatsTest);
  FRIEND_TEST(StatsStorageTests, InsertTableStatsTest);
  FRIEND_TEST(StatsStorageTests, DeleteTableStatsTest);

  /**
   * An unordered map mapping StatsStorageKey objects (database_id and table_id) to
   * TableStats pointers. This represents the storage for TableStats objects.
   */
  std::unordered_map<StatsStorageKey, std::unique_ptr<TableStats>> table_stats_storage_;
};
}  // namespace noisepage::optimizer
