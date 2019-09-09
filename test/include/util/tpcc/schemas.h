#pragma once

#include <utility>
#include <vector>
#include "catalog/catalog_defs.h"
#include "catalog/index_schema.h"
#include "catalog/schema.h"
#include "common/macros.h"
#include "parser/expression/column_value_expression.h"
#include "parser/expression/constant_value_expression.h"
#include "storage/index/index_defs.h"
#include "type/transient_value_factory.h"

namespace terrier::tpcc {

/**
 * Schemas is a utility class that defines all 9 table schemas and 8 index schemas according to section 1.3 of the TPC-C
 * specification. It also defines 2 secondary indexes that improve query performance.
 */
class Schemas {
 public:
  Schemas() = delete;

  /**
   * @param oid_counter global OID counter to be incremented
   * @return Warehouse table schema
   */
  static catalog::Schema BuildWarehouseTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> warehouse_columns;
    warehouse_columns.reserve(NUM_WAREHOUSE_TABLE_COLS);

    // 2*W unique IDs
    warehouse_columns.emplace_back(
        "w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 10
    warehouse_columns.emplace_back(
        "w_name", type::TypeId::VARCHAR, 10, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    warehouse_columns.emplace_back(
        "w_street_1", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    warehouse_columns.emplace_back(
        "w_street_2", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    warehouse_columns.emplace_back(
        "w_city", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 2
    warehouse_columns.emplace_back(
        "w_state", type::TypeId::VARCHAR, 2, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 9
    warehouse_columns.emplace_back(
        "w_zip", type::TypeId::VARCHAR, 9, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(4,4)
    warehouse_columns.emplace_back(
        "w_tax", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(12,2)
    warehouse_columns.emplace_back(
        "w_ytd", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    warehouse_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(warehouse_columns.size() == NUM_WAREHOUSE_TABLE_COLS,
                   "Wrong number of columns for Warehouse table schema.");

    return catalog::Schema(warehouse_columns);
  }

  /**
   * @param schema Warehouse table schema
   * @param oid_counter global OID counter to be incremented
   * @return Warehouse primary index schema
   */
  static catalog::IndexSchema BuildWarehousePrimaryIndexSchema(const catalog::Schema &schema,
                                                               uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> warehouse_key_schema;
    warehouse_key_schema.reserve(NUM_WAREHOUSE_PRIMARY_INDEX_COLS);

    // Primary Key: W_ID
    warehouse_key_schema.emplace_back(
        "w_id", schema.GetColumn("w_id").Type(), schema.GetColumn("w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("w_id").Oid()));
    warehouse_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(warehouse_key_schema.size() == NUM_WAREHOUSE_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for Warehouse primary index schema.");

    return catalog::IndexSchema(warehouse_key_schema, true, true, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return District table schema
   */
  static catalog::Schema BuildDistrictTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> district_columns;
    district_columns.reserve(NUM_DISTRICT_TABLE_COLS);

    // 20 unique IDs
    district_columns.emplace_back(
        "d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    district_columns.emplace_back(
        "d_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 10
    district_columns.emplace_back(
        "d_name", type::TypeId::VARCHAR, 10, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    district_columns.emplace_back(
        "d_street_1", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    district_columns.emplace_back(
        "d_street_2", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    district_columns.emplace_back(
        "d_city", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 2
    district_columns.emplace_back(
        "d_state", type::TypeId::VARCHAR, 2, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 9
    district_columns.emplace_back(
        "d_zip", type::TypeId::VARCHAR, 9, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(4,4)
    district_columns.emplace_back(
        "d_tax", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(12,2)
    district_columns.emplace_back(
        "d_ytd", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 10,000,000 unique IDs
    district_columns.emplace_back(
        "d_next_o_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    district_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(district_columns.size() == NUM_DISTRICT_TABLE_COLS,
                   "Wrong number of columns for District table schema.");

    return catalog::Schema(district_columns);
  }

  /**
   * @param schema District table schema
   * @param oid_counter global OID counter to be incremented
   * @return District primary index schema
   */
  static catalog::IndexSchema BuildDistrictPrimaryIndexSchema(const catalog::Schema &schema,
                                                              uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> district_key_schema;
    district_key_schema.reserve(NUM_DISTRICT_PRIMARY_INDEX_COLS);

    // Primary Key: (D_W_ID, D_ID)
    district_key_schema.emplace_back(
        "d_w_id", schema.GetColumn("d_w_id").Type(), schema.GetColumn("d_w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("d_w_id").Oid()));
    district_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    district_key_schema.emplace_back(
        "d_id", schema.GetColumn("d_id").Type(), schema.GetColumn("d_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("d_id").Oid()));
    district_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(district_key_schema.size() == NUM_DISTRICT_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for District primary index schema.");

    return catalog::IndexSchema(district_key_schema, true, true, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return Customer table schema
   */
  static catalog::Schema BuildCustomerTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> customer_columns;
    customer_columns.reserve(NUM_CUSTOMER_TABLE_COLS);

    // 96,000 unique IDs
    customer_columns.emplace_back(
        "c_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 20 unique IDs
    customer_columns.emplace_back(
        "c_d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    customer_columns.emplace_back(
        "c_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 16
    customer_columns.emplace_back(
        "c_first", type::TypeId::VARCHAR, 16, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 2
    customer_columns.emplace_back(
        "c_middle", type::TypeId::VARCHAR, 2, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 16
    customer_columns.emplace_back(
        "c_last", type::TypeId::VARCHAR, 16, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    customer_columns.emplace_back(
        "c_street_1", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    customer_columns.emplace_back(
        "c_street_2", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 20
    customer_columns.emplace_back(
        "c_city", type::TypeId::VARCHAR, 20, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 2
    customer_columns.emplace_back(
        "c_state", type::TypeId::VARCHAR, 2, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 9
    customer_columns.emplace_back(
        "c_zip", type::TypeId::VARCHAR, 9, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 16
    customer_columns.emplace_back(
        "c_phone", type::TypeId::VARCHAR, 16, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // date and time
    customer_columns.emplace_back(
        "c_since", type::TypeId::TIMESTAMP, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TIMESTAMP)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 2
    customer_columns.emplace_back(
        "c_credit", type::TypeId::VARCHAR, 2, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(12,2)
    customer_columns.emplace_back(
        "c_credit_lim", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(4,4)
    customer_columns.emplace_back(
        "c_discount", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(12,2)
    customer_columns.emplace_back(
        "c_balance", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(12,2)
    customer_columns.emplace_back(
        "c_ytd_payment", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(4)
    customer_columns.emplace_back(
        "c_payment_cnt", type::TypeId::SMALLINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::SMALLINT)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(4)
    customer_columns.emplace_back(
        "c_delivery_cnt", type::TypeId::SMALLINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::SMALLINT)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 500
    customer_columns.emplace_back(
        "c_data", type::TypeId::VARCHAR, 500, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    customer_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(customer_columns.size() == NUM_CUSTOMER_TABLE_COLS,
                   "Wrong number of columns for Customer table schema.");

    return catalog::Schema(customer_columns);
  }

  /**
   * @param schema Customer table schema
   * @param oid_counter global OID counter to be incremented
   * @return Customer primary index schema
   */
  static catalog::IndexSchema BuildCustomerPrimaryIndexSchema(const catalog::Schema &schema,
                                                              uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> customer_key_schema;
    customer_key_schema.reserve(NUM_CUSTOMER_PRIMARY_INDEX_COLS);

    // Primary Key: (C_W_ID, C_D_ID, C_ID)
    customer_key_schema.emplace_back(
        "c_w_id", schema.GetColumn("c_w_id").Type(), schema.GetColumn("c_w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("c_w_id").Oid()));
    customer_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    customer_key_schema.emplace_back(
        "c_d_id", schema.GetColumn("c_d_id").Type(), schema.GetColumn("c_d_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("c_d_id").Oid()));
    customer_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    customer_key_schema.emplace_back(
        "c_id", schema.GetColumn("c_id").Type(), schema.GetColumn("c_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("c_id").Oid()));
    customer_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(customer_key_schema.size() == NUM_CUSTOMER_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for Customer primary index schema.");

    return catalog::IndexSchema(customer_key_schema, true, true, false, true);
  }

  /**
   * @param schema Customer table schema
   * @param oid_counter global OID counter to be incremented
   * @return Customer secondary index schema
   */
  static catalog::IndexSchema BuildCustomerSecondaryIndexSchema(const catalog::Schema &schema,
                                                                uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> customer_secondary_key_schema;
    customer_secondary_key_schema.reserve(NUM_CUSTOMER_SECONDARY_INDEX_COLS);

    // C_W_ID, C_D_ID, C_LAST for Order Status and Payment transactions
    customer_secondary_key_schema.emplace_back(
        "c_w_id", schema.GetColumn("c_w_id").Type(), schema.GetColumn("c_w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("c_w_id").Oid()));
    customer_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    customer_secondary_key_schema.emplace_back(
        "c_d_id", schema.GetColumn("c_d_id").Type(), schema.GetColumn("c_d_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("c_d_id").Oid()));
    customer_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    customer_secondary_key_schema.emplace_back(
        "c_last", schema.GetColumn("c_last").Type(), schema.GetColumn("c_last").MaxVarlenSize(),
        schema.GetColumn("c_last").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("c_last").Oid()));
    customer_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(customer_secondary_key_schema.size() == NUM_CUSTOMER_SECONDARY_INDEX_COLS,
                   "Wrong number of columns for Customer secondary index schema.");

    return catalog::IndexSchema(customer_secondary_key_schema, false, false, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return History table schema
   */
  static catalog::Schema BuildHistoryTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> history_columns;
    history_columns.reserve(NUM_HISTORY_TABLE_COLS);

    // 96,000 unique IDs
    history_columns.emplace_back(
        "h_c_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 20 unique IDs
    history_columns.emplace_back(
        "h_c_d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    history_columns.emplace_back(
        "h_c_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 20 unique IDs
    history_columns.emplace_back(
        "h_d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    history_columns.emplace_back(
        "h_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // date and time
    history_columns.emplace_back(
        "h_date", type::TypeId::TIMESTAMP, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TIMESTAMP)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(6,2)
    history_columns.emplace_back(
        "h_amount", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 24
    history_columns.emplace_back(
        "h_data", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    history_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(history_columns.size() == NUM_HISTORY_TABLE_COLS,
                   "Wrong number of columns for History table schema.");

    return catalog::Schema(history_columns);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return New Order table schema
   */
  static catalog::Schema BuildNewOrderTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> new_order_columns;
    new_order_columns.reserve(NUM_NEW_ORDER_TABLE_COLS);

    // 10,000,000 unique IDs
    new_order_columns.emplace_back(
        "no_o_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    new_order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 20 unique IDs
    new_order_columns.emplace_back(
        "no_d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    new_order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    new_order_columns.emplace_back(
        "no_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    new_order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(new_order_columns.size() == NUM_NEW_ORDER_TABLE_COLS,
                   "Wrong number of columns for New Order table schema.");

    return catalog::Schema(new_order_columns);
  }

  /**
   * @param schema New Order table schema
   * @param oid_counter global OID counter to be incremented
   * @return New Order primary index schema
   */
  static catalog::IndexSchema BuildNewOrderPrimaryIndexSchema(const catalog::Schema &schema,
                                                              uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> new_order_key_schema;
    new_order_key_schema.reserve(NUM_NEW_ORDER_PRIMARY_INDEX_COLS);

    // Primary Key: (NO_W_ID, NO_D_ID, NO_O_ID)
    new_order_key_schema.emplace_back(
        "no_w_id", schema.GetColumn("no_w_id").Type(), schema.GetColumn("no_w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("no_w_id").Oid()));
    new_order_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    new_order_key_schema.emplace_back(
        "no_d_id", schema.GetColumn("no_d_id").Type(), schema.GetColumn("no_d_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("no_d_id").Oid()));
    new_order_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    new_order_key_schema.emplace_back(
        "no_o_id", schema.GetColumn("no_o_id").Type(), schema.GetColumn("no_o_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("no_o_id").Oid()));
    new_order_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(new_order_key_schema.size() == NUM_NEW_ORDER_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for New Order primary index schema.");

    return catalog::IndexSchema(new_order_key_schema, true, true, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return Order table schema
   */
  static catalog::Schema BuildOrderTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> order_columns;
    order_columns.reserve(NUM_ORDER_TABLE_COLS);

    // 10,000,000 unique IDs
    order_columns.emplace_back(
        "o_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 20 unique IDs
    order_columns.emplace_back(
        "o_d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    order_columns.emplace_back(
        "o_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 96,000 unique IDs
    order_columns.emplace_back(
        "o_c_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // date and time
    order_columns.emplace_back(
        "o_entry_d", type::TypeId::TIMESTAMP, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TIMESTAMP)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 10 unique IDs, or null
    order_columns.emplace_back(
        "o_carrier_id", type::TypeId::TINYINT, true,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(2)
    order_columns.emplace_back(
        "o_ol_cnt", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(1)
    order_columns.emplace_back(
        "o_all_local", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(order_columns.size() == NUM_ORDER_TABLE_COLS, "Wrong number of columns for Order table schema.");

    return catalog::Schema(order_columns);
  }

  /**
   * @param schema Order table schema
   * @param oid_counter global OID counter to be incremented
   * @return Order primary index schema
   */
  static catalog::IndexSchema BuildOrderPrimaryIndexSchema(const catalog::Schema &schema, uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> order_key_schema;
    order_key_schema.reserve(NUM_ORDER_PRIMARY_INDEX_COLS);

    // Primary Key: (O_W_ID, O_D_ID, O_ID)
    order_key_schema.emplace_back("o_w_id", schema.GetColumn("o_w_id").Type(), schema.GetColumn("o_w_id").Nullable(),
                                  parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                                                schema.GetColumn("o_w_id").Oid()));
    order_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_key_schema.emplace_back("o_d_id", schema.GetColumn("o_d_id").Type(), schema.GetColumn("o_d_id").Nullable(),
                                  parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                                                schema.GetColumn("o_d_id").Oid()));
    order_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_key_schema.emplace_back("o_id", schema.GetColumn("o_id").Type(), schema.GetColumn("o_id").Nullable(),
                                  parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                                                schema.GetColumn("o_id").Oid()));
    order_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(order_key_schema.size() == NUM_ORDER_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for Order primary index schema.");

    return catalog::IndexSchema(order_key_schema, true, true, false, true);
  }

  /**
   * @param schema Order table schema
   * @param oid_counter global OID counter to be incremented
   * @return Order secondary index schema
   */
  static catalog::IndexSchema BuildOrderSecondaryIndexSchema(const catalog::Schema &schema,
                                                             uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> order_secondary_key_schema;
    order_secondary_key_schema.reserve(NUM_ORDER_SECONDARY_INDEX_COLS);

    // O_W_ID, O_D_ID, O_C_ID, O_ID for Order Status transaction
    order_secondary_key_schema.emplace_back(
        "o_w_id", schema.GetColumn("o_w_id").Type(), schema.GetColumn("o_w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("o_w_id").Oid()));
    order_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_secondary_key_schema.emplace_back(
        "o_d_id", schema.GetColumn("o_d_id").Type(), schema.GetColumn("o_d_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("o_d_id").Oid()));
    order_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_secondary_key_schema.emplace_back(
        "o_c_id", schema.GetColumn("o_c_id").Type(), schema.GetColumn("o_c_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("o_c_id").Oid()));
    order_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_secondary_key_schema.emplace_back(
        "o_id", schema.GetColumn("o_id").Type(), schema.GetColumn("o_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("o_id").Oid()));
    order_secondary_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(order_secondary_key_schema.size() == NUM_ORDER_SECONDARY_INDEX_COLS,
                   "Wrong number of columns for Order secondary index schema.");

    return catalog::IndexSchema(order_secondary_key_schema, true, false, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return Order Line table schema
   */
  static catalog::Schema BuildOrderLineTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> order_line_columns;
    order_line_columns.reserve(NUM_ORDER_LINE_TABLE_COLS);

    // 10,000,000 unique IDs
    order_line_columns.emplace_back(
        "ol_o_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 20 unique IDs
    order_line_columns.emplace_back(
        "ol_d_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    order_line_columns.emplace_back(
        "ol_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 15 unique IDs
    order_line_columns.emplace_back(
        "ol_number", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 200,000 unique IDs
    order_line_columns.emplace_back(
        "ol_i_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    order_line_columns.emplace_back(
        "ol_supply_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // date and time, or null
    order_line_columns.emplace_back(
        "ol_delivery_d", type::TypeId::TIMESTAMP, true,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TIMESTAMP)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(2)
    order_line_columns.emplace_back(
        "ol_quantity", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(6,2)
    order_line_columns.emplace_back(
        "ol_amount", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    order_line_columns.emplace_back(
        "ol_dist_info", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    order_line_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(order_line_columns.size() == NUM_ORDER_LINE_TABLE_COLS,
                   "Wrong number of columns for Order Line table schema.");

    return catalog::Schema(order_line_columns);
  }

  /**
   * @param schema Order Line table schema
   * @param oid_counter global OID counter to be incremented
   * @return Order Line primary index schema
   */
  static catalog::IndexSchema BuildOrderLinePrimaryIndexSchema(const catalog::Schema &schema,
                                                               uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> order_line_key_schema;
    order_line_key_schema.reserve(NUM_ORDER_LINE_PRIMARY_INDEX_COLS);

    // Primary Key: (OL_W_ID, OL_D_ID, OL_O_ID, OL_NUMBER)
    order_line_key_schema.emplace_back(
        "ol_w_id", schema.GetColumn("ol_w_id").Type(), schema.GetColumn("ol_w_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("ol_w_id").Oid()));
    order_line_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_line_key_schema.emplace_back(
        "ol_d_id", schema.GetColumn("ol_d_id").Type(), schema.GetColumn("ol_d_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("ol_d_id").Oid()));
    order_line_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_line_key_schema.emplace_back(
        "ol_o_id", schema.GetColumn("ol_o_id").Type(), schema.GetColumn("ol_o_id").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("ol_o_id").Oid()));
    order_line_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    order_line_key_schema.emplace_back(
        "ol_number", schema.GetColumn("ol_number").Type(), schema.GetColumn("ol_number").Nullable(),
        parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                      schema.GetColumn("ol_number").Oid()));
    order_line_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(order_line_key_schema.size() == NUM_ORDER_LINE_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for Order Line key schema.");

    return catalog::IndexSchema(order_line_key_schema, true, true, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return Item table schema
   */
  static catalog::Schema BuildItemTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> item_columns;
    item_columns.reserve(NUM_ITEM_TABLE_COLS);

    // 200,000 unique IDs
    item_columns.emplace_back(
        "i_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    item_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 200,000 unique IDs
    item_columns.emplace_back(
        "i_im_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    item_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 24
    item_columns.emplace_back(
        "i_name", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    item_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(5,2)
    item_columns.emplace_back(
        "i_price", type::TypeId::DECIMAL, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::DECIMAL)));
    item_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 50
    item_columns.emplace_back(
        "i_data", type::TypeId::VARCHAR, 50, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    item_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(item_columns.size() == NUM_ITEM_TABLE_COLS, "Wrong number of columns for Item table schema.");

    return catalog::Schema(item_columns);
  }

  /**
   * @param schema Item table schema
   * @param oid_counter global OID counter to be incremented
   * @return Item primary index schema
   */
  static catalog::IndexSchema BuildItemPrimaryIndexSchema(const catalog::Schema &schema, uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> item_key_schema;
    item_key_schema.reserve(NUM_ITEM_PRIMARY_INDEX_COLS);

    // Primary Key: I_ID
    item_key_schema.emplace_back("i_id", schema.GetColumn("i_id").Type(), schema.GetColumn("i_id").Nullable(),
                                 parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                                               schema.GetColumn("i_id").Oid()));
    item_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(item_key_schema.size() == NUM_ITEM_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for Item primary index schema.");

    return catalog::IndexSchema(item_key_schema, true, true, false, true);
  }

  /**
   * @param oid_counter global OID counter to be incremented
   * @return Stock table schema
   */
  static catalog::Schema BuildStockTableSchema(uint32_t *const oid_counter) {
    std::vector<catalog::Schema::Column> stock_columns;
    stock_columns.reserve(NUM_STOCK_TABLE_COLS);

    // 200,000 unique IDs
    stock_columns.emplace_back(
        "s_i_id", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // 2*W unique IDs
    stock_columns.emplace_back(
        "s_w_id", type::TypeId::TINYINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::TINYINT)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // signed numeric(4)
    stock_columns.emplace_back(
        "s_quantity", type::TypeId::SMALLINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::SMALLINT)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_01", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_02", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_03", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_04", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_05", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_06", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_07", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_08", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_09", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // fixed text, size 24
    stock_columns.emplace_back(
        "s_dist_10", type::TypeId::VARCHAR, 24, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(8)
    stock_columns.emplace_back(
        "s_ytd", type::TypeId::INTEGER, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::INTEGER)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(4)
    stock_columns.emplace_back(
        "s_order_cnt", type::TypeId::SMALLINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::SMALLINT)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // numeric(4)
    stock_columns.emplace_back(
        "s_remote_cnt", type::TypeId::SMALLINT, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::SMALLINT)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));
    // variable text, size 50
    stock_columns.emplace_back(
        "s_data", type::TypeId::VARCHAR, 50, false,
        parser::ConstantValueExpression(type::TransientValueFactory::GetNull(type::TypeId::VARCHAR)));
    stock_columns.back().SetOid(static_cast<catalog::col_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(stock_columns.size() == NUM_STOCK_TABLE_COLS, "Wrong number of columns for Stock table schema.");

    return catalog::Schema(stock_columns);
  }

  /**
   * @param schema Stock table schema
   * @param oid_counter global OID counter to be incremented
   * @return Stock primary index schema
   */
  static catalog::IndexSchema BuildStockPrimaryIndexSchema(const catalog::Schema &schema, uint32_t *const oid_counter) {
    std::vector<catalog::IndexSchema::Column> stock_key_schema;
    stock_key_schema.reserve(NUM_STOCK_PRIMARY_INDEX_COLS);

    // Primary Key: (S_W_ID, S_I_ID)
    stock_key_schema.emplace_back("s_w_id", schema.GetColumn("s_w_id").Type(), schema.GetColumn("s_w_id").Nullable(),
                                  parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                                                schema.GetColumn("s_w_id").Oid()));
    stock_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));
    stock_key_schema.emplace_back("s_i_id", schema.GetColumn("s_i_id").Type(), schema.GetColumn("s_i_id").Nullable(),
                                  parser::ColumnValueExpression(catalog::INVALID_DATABASE_OID, catalog::table_oid_t(0),
                                                                schema.GetColumn("s_i_id").Oid()));
    stock_key_schema.back().SetOid(static_cast<catalog::indexkeycol_oid_t>(++(*oid_counter)));

    TERRIER_ASSERT(stock_key_schema.size() == NUM_STOCK_PRIMARY_INDEX_COLS,
                   "Wrong number of columns for Stock primary index schema.");

    return catalog::IndexSchema(stock_key_schema, true, true, false, true);
  }

 private:
  // The values below are just to sanity check the schema functions
  static constexpr uint8_t NUM_WAREHOUSE_TABLE_COLS = 9;
  static constexpr uint8_t NUM_DISTRICT_TABLE_COLS = 11;
  static constexpr uint8_t NUM_CUSTOMER_TABLE_COLS = 21;
  static constexpr uint8_t NUM_HISTORY_TABLE_COLS = 8;
  static constexpr uint8_t NUM_NEW_ORDER_TABLE_COLS = 3;
  static constexpr uint8_t NUM_ORDER_TABLE_COLS = 8;
  static constexpr uint8_t NUM_ORDER_LINE_TABLE_COLS = 10;
  static constexpr uint8_t NUM_ITEM_TABLE_COLS = 5;
  static constexpr uint8_t NUM_STOCK_TABLE_COLS = 17;

  static constexpr uint8_t NUM_WAREHOUSE_PRIMARY_INDEX_COLS = 1;
  static constexpr uint8_t NUM_DISTRICT_PRIMARY_INDEX_COLS = 2;
  static constexpr uint8_t NUM_CUSTOMER_PRIMARY_INDEX_COLS = 3;
  static constexpr uint8_t NUM_CUSTOMER_SECONDARY_INDEX_COLS = 3;
  static constexpr uint8_t NUM_NEW_ORDER_PRIMARY_INDEX_COLS = 3;
  static constexpr uint8_t NUM_ORDER_PRIMARY_INDEX_COLS = 3;
  static constexpr uint8_t NUM_ORDER_SECONDARY_INDEX_COLS = 4;
  static constexpr uint8_t NUM_ORDER_LINE_PRIMARY_INDEX_COLS = 4;
  static constexpr uint8_t NUM_ITEM_PRIMARY_INDEX_COLS = 1;
  static constexpr uint8_t NUM_STOCK_PRIMARY_INDEX_COLS = 2;
};

}  // namespace terrier::tpcc
