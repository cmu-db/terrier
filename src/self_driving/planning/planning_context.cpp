#include "self_driving/planning/planning_context.h"

#include "catalog/catalog.h"
#include "transaction/transaction_manager.h"

namespace noisepage::selfdriving::pilot {

void PlanningContext::AddDatabase(catalog::db_oid_t db_oid) {
  auto txn = txn_manager_->BeginTransaction();
  db_oid_to_accessor_.emplace(
      std::make_pair(db_oid, catalog_->GetAccessor(common::ManagedPointer(txn), db_oid, DISABLED)));
  db_oid_to_txn_.emplace(std::make_pair(db_oid, txn));
}

void PlanningContext::ClearDatabases() {
  for (auto &[db_oid, txn] : db_oid_to_txn_) txn_manager_->Abort(txn.get());

  db_oid_to_txn_.clear();
  db_oid_to_accessor_.clear();
}
}  // namespace noisepage::selfdriving::pilot
