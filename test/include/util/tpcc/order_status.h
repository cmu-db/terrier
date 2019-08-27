#pragma once

#include <map>
#include <string>
#include <vector>
#include "catalog/catalog_defs.h"
#include "storage/sql_table.h"
#include "storage/storage_defs.h"
#include "transaction/transaction_manager.h"
#include "util/tpcc/database.h"
#include "util/tpcc/tpcc_defs.h"
#include "util/tpcc/util.h"
#include "util/tpcc/worker.h"
#include "util/transaction_benchmark_util.h"

namespace terrier::tpcc {

/**
 * Order-Status transaction according to section 2.6.2 of the specification
 */
class OrderStatus {
 private:
  // Customer metadata
  const uint8_t c_id_key_pr_offset_;
  const uint8_t c_d_id_key_pr_offset_;
  const uint8_t c_w_id_key_pr_offset_;
  const uint8_t c_last_name_key_pr_offset_;
  const uint8_t c_d_id_name_key_pr_offset_;
  const uint8_t c_w_id_name_key_pr_offset_;
  const catalog::col_oid_t c_id_oid_;
  const catalog::col_oid_t c_balance_oid_;
  const catalog::col_oid_t c_first_oid_;
  const catalog::col_oid_t c_middle_oid_;
  const catalog::col_oid_t c_last_oid_;
  const storage::ProjectedRowInitializer c_first_pr_initializer_;
  const storage::ProjectedRowInitializer customer_select_pr_initializer_;
  const storage::ProjectionMap customer_select_pr_map_;
  const uint8_t c_id_select_pr_offset_;
  const uint8_t c_balance_select_pr_offset_;
  const uint8_t c_first_select_pr_offset_;
  const uint8_t c_middle_select_pr_offset_;
  const uint8_t c_last_select_pr_offset_;

  // Order metadata
  const uint8_t o_id_secondary_key_pr_offset_;
  const uint8_t o_d_id_secondary_key_pr_offset_;
  const uint8_t o_w_id_secondary_key_pr_offset_;
  const uint8_t o_c_id_secondary_key_pr_offset_;
  const catalog::col_oid_t o_id_oid_;
  const catalog::col_oid_t o_entry_d_oid_;
  const catalog::col_oid_t o_carrier_id_oid_;
  const storage::ProjectedRowInitializer order_select_pr_initializer_;
  const storage::ProjectionMap order_select_pr_map_;
  const uint8_t o_id_select_pr_offset_;

  // Order Line metadata
  const uint8_t ol_o_id_key_pr_offset_;
  const uint8_t ol_d_id_key_pr_offset_;
  const uint8_t ol_w_id_key_pr_offset_;
  const uint8_t ol_number_key_pr_offset_;
  const catalog::col_oid_t ol_i_id_oid_;
  const catalog::col_oid_t ol_supply_w_id_oid_;
  const catalog::col_oid_t ol_quantity_oid_;
  const catalog::col_oid_t ol_amount_oid_;
  const catalog::col_oid_t ol_delivery_d_oid_;
  const storage::ProjectedRowInitializer order_line_select_pr_initializer_;

 public:
  explicit OrderStatus(const Database *const db)
      : c_id_key_pr_offset_(static_cast<uint8_t>(db->customer_primary_index_->GetKeyOidToOffsetMap().at(
            db->customer_primary_index_schema_.GetColumn(2).Oid()))),
        c_d_id_key_pr_offset_(static_cast<uint8_t>(db->customer_primary_index_->GetKeyOidToOffsetMap().at(
            db->customer_primary_index_schema_.GetColumn(1).Oid()))),
        c_w_id_key_pr_offset_(static_cast<uint8_t>(db->customer_primary_index_->GetKeyOidToOffsetMap().at(
            db->customer_primary_index_schema_.GetColumn(0).Oid()))),
        c_last_name_key_pr_offset_(static_cast<uint8_t>(db->customer_secondary_index_->GetKeyOidToOffsetMap().at(
            db->customer_secondary_index_schema_.GetColumn(2).Oid()))),
        c_d_id_name_key_pr_offset_(static_cast<uint8_t>(db->customer_secondary_index_->GetKeyOidToOffsetMap().at(
            db->customer_secondary_index_schema_.GetColumn(1).Oid()))),
        c_w_id_name_key_pr_offset_(static_cast<uint8_t>(db->customer_secondary_index_->GetKeyOidToOffsetMap().at(
            db->customer_secondary_index_schema_.GetColumn(0).Oid()))),

        c_id_oid_(db->customer_schema_.GetColumn(0).Oid()),
        c_balance_oid_(db->customer_schema_.GetColumn(16).Oid()),
        c_first_oid_(db->customer_schema_.GetColumn(3).Oid()),
        c_middle_oid_(db->customer_schema_.GetColumn(4).Oid()),
        c_last_oid_(db->customer_schema_.GetColumn(5).Oid()),

        c_first_pr_initializer_(db->customer_table_->InitializerForProjectedRow({c_first_oid})),
        customer_select_pr_initializer_(db->customer_table_->InitializerForProjectedRow(
            {c_id_oid, c_balance_oid, c_first_oid, c_middle_oid, c_last_oid})),
        customer_select_pr_map_(db->customer_table_->ProjectionMapForOids(
            {c_id_oid, c_balance_oid, c_first_oid, c_middle_oid, c_last_oid})),

        c_id_select_pr_offset_(static_cast<uint8_t>(customer_select_pr_map.at(c_id_oid))),
        c_balance_select_pr_offset_(static_cast<uint8_t>(customer_select_pr_map.at(c_balance_oid))),
        c_first_select_pr_offset_(static_cast<uint8_t>(customer_select_pr_map.at(c_first_oid))),
        c_middle_select_pr_offset_(static_cast<uint8_t>(customer_select_pr_map.at(c_middle_oid))),
        c_last_select_pr_offset_(static_cast<uint8_t>(customer_select_pr_map.at(c_last_oid))),
        o_id_secondary_key_pr_offset_(static_cast<uint8_t>(db->order_secondary_index_->GetKeyOidToOffsetMap().at(
            db->order_secondary_index_schema_.GetColumn(3).Oid()))),
        o_d_id_secondary_key_pr_offset_(static_cast<uint8_t>(db->order_secondary_index_->GetKeyOidToOffsetMap().at(
            db->order_secondary_index_schema_.GetColumn(1).Oid()))),
        o_w_id_secondary_key_pr_offset_(static_cast<uint8_t>(db->order_secondary_index_->GetKeyOidToOffsetMap().at(
            db->order_secondary_index_schema_.GetColumn(0).Oid()))),
        o_c_id_secondary_key_pr_offset_(static_cast<uint8_t>(db->order_secondary_index_->GetKeyOidToOffsetMap().at(
            db->order_secondary_index_schema_.GetColumn(2).Oid()))),
        o_id_oid_(db->order_schema_.GetColumn(0).Oid()),
        o_entry_d_oid_(db->order_schema_.GetColumn(4).Oid()),
        o_carrier_id_oid_(db->order_schema_.GetColumn(5).Oid()),
        order_select_pr_initializer_(
            db->order_table_->InitializerForProjectedRow({o_id_oid, o_entry_d_oid, o_carrier_id_oid})),
        order_select_pr_map_(db->order_table_->ProjectionMapForOids({o_id_oid, o_entry_d_oid, o_carrier_id_oid})),
        o_id_select_pr_offset_(static_cast<uint8_t>(order_select_pr_map.at(o_id_oid))),
        ol_o_id_key_pr_offset_(static_cast<uint8_t>(db->order_line_primary_index_->GetKeyOidToOffsetMap().at(
            db->order_line_primary_index_schema_.GetColumn(2).Oid()))),
        ol_d_id_key_pr_offset_(static_cast<uint8_t>(db->order_line_primary_index_->GetKeyOidToOffsetMap().at(
            db->order_line_primary_index_schema_.GetColumn(1).Oid()))),
        ol_w_id_key_pr_offset_(static_cast<uint8_t>(db->order_line_primary_index_->GetKeyOidToOffsetMap().at(
            db->order_line_primary_index_schema_.GetColumn(0).Oid()))),
        ol_number_key_pr_offset_(static_cast<uint8_t>(db->order_line_primary_index_->GetKeyOidToOffsetMap().at(
            db->order_line_primary_index_schema_.GetColumn(3).Oid()))),

        ol_i_id_oid_(db->order_line_schema_.GetColumn(4).Oid()),
        ol_supply_w_id_oid_(db->order_line_schema_.GetColumn(5).Oid()),
        ol_quantity_oid_(db->order_line_schema_.GetColumn(7).Oid()),
        ol_amount_oid_(db->order_line_schema_.GetColumn(8).Oid()),
        ol_delivery_d_oid_(db->order_line_schema_.GetColumn(6).Oid()),
        order_line_select_pr_initializer_(db->order_line_table_->InitializerForProjectedRow(
            {ol_i_id_oid, ol_supply_w_id_oid, ol_quantity_oid, ol_amount_oid, ol_delivery_d_oid}))

  {}

  bool Execute(transaction::TransactionManager *txn_manager, Database *db, Worker *worker,
               const TransactionArgs &args) const;
};

}  // namespace terrier::tpcc
