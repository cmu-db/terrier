#include "optimizer/statistics/stats_storage.h"

#include <memory>
#include <utility>

#include "loggers/optimizer_logger.h"

namespace noisepage::optimizer {

// TODO(Joe) deal with dirty stats
common::ManagedPointer<TableStats> StatsStorage::GetTableStats(const catalog::db_oid_t database_id,
                                                               const catalog::table_oid_t table_id,
                                                               catalog::CatalogAccessor *accessor) {
  StatsStorageKey stats_storage_key = std::make_pair(database_id, table_id);
  auto table_it = table_stats_storage_.find(stats_storage_key);

  if (table_it != table_stats_storage_.end()) {
    return common::ManagedPointer<TableStats>(table_it->second);
  }

  // TODO(Joe) would be nice to only pull in columns that we need instead of the whole table
  InsertTableStats(database_id, table_id, accessor->GetTableStatistics(table_id));

  return common::ManagedPointer<TableStats>(table_stats_storage_.at({database_id, table_id}));
}

void StatsStorage::InsertTableStats(catalog::db_oid_t database_id, catalog::table_oid_t table_id,
                                    std::unique_ptr<TableStats> table_stats) {
  StatsStorageKey stats_storage_key = std::make_pair(database_id, table_id);
  NOISEPAGE_ASSERT(table_stats_storage_.count(stats_storage_key) == 0,
                   "There already exists a TableStats object with the given oids.");
  table_stats_storage_.emplace(stats_storage_key, std::move(table_stats));
}

void StatsStorage::DeleteTableStats(catalog::db_oid_t database_id, catalog::table_oid_t table_id) {
  StatsStorageKey stats_storage_key = std::make_pair(database_id, table_id);
  NOISEPAGE_ASSERT(table_stats_storage_.count(stats_storage_key) != 0,
                   "There is no TableStats object with the given oids");
  table_stats_storage_.erase(table_stats_storage_.find(stats_storage_key));
}
}  // namespace noisepage::optimizer
