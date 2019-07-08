#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/managed_pointer.h"
#include "common/sql_node_visitor.h"
#include "parser/parser_defs.h"
#include "parser/select_statement.h"
#include "parser/sql_statement.h"

namespace terrier {
namespace parser {

/**
 * Represents the sql "INSERT ..."
 */
class InsertStatement : public SQLStatement {
 public:
  /**
   * Insert from SELECT
   * @param columns columns to insert into
   * @param table_ref table
   * @param select select statement to insert from
   */
  InsertStatement(std::vector<std::string> columns, common::ManagedPointer<TableRef> table_ref,
                  common::ManagedPointer<SelectStatement> select)
      : SQLStatement(StatementType::INSERT),
        type_(InsertType::SELECT),
        columns_(std::move(columns)),
        table_ref_(table_ref),
        select_(select) {}

  /**
   * Insert from VALUES
   * @param columns columns to insert into
   * @param table_ref table
   * @param insert_values values to be inserted
   */
  InsertStatement(std::vector<std::string> columns, common::ManagedPointer<TableRef> table_ref,
                  std::vector<std::vector<common::ManagedPointer<AbstractExpression>>> insert_values)
      : SQLStatement(StatementType::INSERT),
        type_(InsertType::VALUES),
        columns_(std::move(columns)),
        table_ref_(table_ref),
        select_(common::ManagedPointer<SelectStatement>(nullptr)),
        insert_values_(std::move(insert_values)) {}

  /**
   * @param type insert type (SELECT or VALUES)
   */
  explicit InsertStatement(InsertType type)
      : SQLStatement(StatementType::INSERT),
        type_(type),
        table_ref_(common::ManagedPointer<TableRef>(nullptr)),
        select_(common::ManagedPointer<SelectStatement>(nullptr)) {}

  void Accept(SqlNodeVisitor *v) override { v->Visit(this); }

  /**
   * @return type of insertion
   */
  InsertType GetInsertType() { return type_; }

  /**
   * @return columns to insert into
   */
  std::vector<std::string> GetInsertColumns() { return columns_; }

  /**
   * @return table to insert into
   */
  common::ManagedPointer<TableRef> GetInsertionTable() const { return table_ref_; }

  /**
   * @return select statement we're inserting from
   */
  common::ManagedPointer<SelectStatement> GetSelect() const { return select_; }

  /**
   * @return number of tuples being inserted
   */
  size_t GetBulkInsertSize() const { return insert_values_.size(); }

  /**
   * @return number of attributes to insert on for each row
   */
  size_t GetAttributesSize() const { return GetBulkInsertSize() > 0 ? insert_values_[0].size() : 0; }

  /**
   * @param tuple_idx index of tuple containing value
   * @param attr_idx index of attribute in the tuple
   * @return value to insert
   */
  common::ManagedPointer<AbstractExpression> GetValue(size_t tuple_idx, size_t attr_idx) const {
    TERRIER_ASSERT(tuple_idx < GetBulkInsertSize(), "Tuple index must be less than number of tuples");
    TERRIER_ASSERT(attr_idx < GetAttributesSize(), "Attribute index must be less than number of attributes");
    return insert_values_[tuple_idx][attr_idx];
  }

 private:
  const InsertType type_;
  const std::vector<std::string> columns_;
  const common::ManagedPointer<TableRef> table_ref_;
  const common::ManagedPointer<SelectStatement> select_;
  // TODO(WAN): unsure about this one.
  const std::vector<std::vector<common::ManagedPointer<AbstractExpression>>> insert_values_;
};

}  // namespace parser
}  // namespace terrier
