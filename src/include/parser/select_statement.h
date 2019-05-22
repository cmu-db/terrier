#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "common/json.h"
#include "common/sql_node_visitor.h"
#include "parser/sql_statement.h"
#include "parser/table_ref.h"

namespace terrier {
namespace parser {

enum OrderType { kOrderAsc, kOrderDesc };
using terrier::parser::OrderType;

/**
 * Describes OrderBy clause in a select statement.
 */
class OrderByDescription {
  // TODO(WAN): hold multiple expressions to be sorted by

 public:
  /**
   * @param types order by types
   * @param exprs order by expressions
   */
  OrderByDescription(std::vector<OrderType> types, std::vector<AbstractExpression *> exprs)
      : types_(std::move(types)), exprs_(std::move(exprs)) {}

  /**
   * Default constructor for deserialization
   */
  OrderByDescription() = default;

  virtual ~OrderByDescription() = default;

  // TODO(WAN): no SQLStatement? maybe a Description base class?
  /**
   * @param v visitor
   */
  void Accept(SqlNodeVisitor *v) { v->Visit(this); }

  /**
   * @return order by types
   */
  std::vector<OrderType> GetOrderByTypes() { return types_; }

  /**
   * @return order by expressions
   */
  std::vector<AbstractExpression *> GetOrderByExpressions() { return exprs_; }

  /**
   * @return OrderByDescription serialized to json
   */
  nlohmann::json ToJson() const {
    nlohmann::json j;
    j["types"] = types_;
    j["exprs"] = exprs_;
    return j;
  }

  /**
   * @param j json to deserialize
   */
  void FromJson(const nlohmann::json &j) {
    // Deserialize types
    types_ = j.at("types").get<std::vector<OrderType>>();

    // Deserialize exprs
    auto expressions = j.at("exprs").get<std::vector<nlohmann::json>>();
    for (const auto &expr : expressions) {
      exprs_.emplace_back(DeserializeExpression(expr));
    }
  }

 private:
  std::vector<OrderType> types_;
  std::vector<AbstractExpression *> exprs_;
};

DEFINE_JSON_DECLARATIONS(OrderByDescription);

/**
 * Describes the limit clause in a SELECT statement.
 */
class LimitDescription {
 public:
  /**
   * Denotes that there is no limit.
   */
  static constexpr int NO_LIMIT = -1;
  /**
   * Denotes that there is no offset.
   */
  static constexpr int NO_OFFSET = -1;

  /**
   * @param limit limit
   * @param offset offset
   */
  LimitDescription(int64_t limit, int64_t offset) : limit_(limit), offset_(offset) {}

  /**
   * Default constructor for deserialization
   */
  LimitDescription() = default;

  ~LimitDescription() = default;

  // TODO(WAN): not SQL statement?
  /**
   * @param v visitor
   */
  void Accept(SqlNodeVisitor *v) { v->Visit(this); }

  /**
   * @return limit
   */
  int64_t GetLimit() { return limit_; }

  /**
   * @return offset
   */
  int64_t GetOffset() { return offset_; }

  /**
   * @return LimitDescription serialized to json
   */
  nlohmann::json ToJson() const {
    nlohmann::json j;
    j["limit"] = limit_;
    j["offset"] = offset_;
    return j;
  }

  /**
   * @param j json to deserialize
   */
  void FromJson(const nlohmann::json &j) {
    limit_ = j.at("limit").get<int64_t>();
    offset_ = j.at("offset").get<int64_t>();
  }

 private:
  int64_t limit_;
  int64_t offset_;
};

DEFINE_JSON_DECLARATIONS(LimitDescription);

/**
 * Represents the sql "GROUP BY".
 */
class GroupByDescription {
 public:
  /**
   * @param columns group by columns
   * @param having having clause
   */
  GroupByDescription(std::vector<AbstractExpression *> columns, AbstractExpression *having)
      : columns_(std::move(columns)), having_(having) {}

  /**
   * Default constructor for deserialization
   */
  GroupByDescription() = default;

  // TODO(WAN): not a SQLStatement?
  /**
   * Visitor pattern for GroupByDescription.
   * @param v visitor
   */
  void Accept(SqlNodeVisitor *v) { v->Visit(this); }

  /**
   * @return group by columns
   */
  std::vector<AbstractExpression *> GetColumns() { return columns_; }

  /**
   * @return having clause
   */
  AbstractExpression *GetHaving() { return having_; }

  /**
   * @return GroupDescription serialized to json
   */
  nlohmann::json ToJson() const {
    nlohmann::json j;
    j["columns"] = columns_;
    j["having"] = having_;
    return j;
  }

  /**
   * @param j json to deserialize
   */
  void FromJson(const nlohmann::json &j) {
    // Deserialize columns
    auto column_expressions = j.at("columns").get<std::vector<nlohmann::json>>();
    for (const auto &expr : column_expressions) {
      columns_.emplace_back(DeserializeExpression(expr));
    }

    // Deserialize having
    if (!j.at("having").is_null()) {
      having_ = DeserializeExpression(j.at("having"));
    }
  }

 private:
  std::vector<AbstractExpression *> columns_;
  AbstractExpression *having_;
};

DEFINE_JSON_DECLARATIONS(GroupByDescription);

/**
 * Represents the sql "SELECT ..."
 */
class SelectStatement : public SQLStatement {
  /*
   * TODO(WAN): old system says, add union_order and union_limit
   */
 public:
  /**
   * @param select columns being selected
   * @param select_distinct true if "SELECT DISTINCT" was used
   * @param from table to select from
   * @param where select condition
   * @param group_by group by condition
   * @param order_by order by condition
   * @param limit limit condition
   */
  SelectStatement(std::vector<AbstractExpression *> select, const bool &select_distinct, std::shared_ptr<TableRef> from,
                  AbstractExpression *where, std::shared_ptr<GroupByDescription> group_by,
                  std::shared_ptr<OrderByDescription> order_by, std::shared_ptr<LimitDescription> limit)
      : SQLStatement(StatementType::SELECT),
        select_(std::move(select)),
        select_distinct_(select_distinct),
        from_(std::move(from)),
        where_(where),
        group_by_(std::move(group_by)),
        order_by_(std::move(order_by)),
        limit_(std::move(limit)),
        union_select_(nullptr) {}

  ~SelectStatement() override = default;

  /**
   * Default constructor for deserialization
   */
  SelectStatement() = default;

  void Accept(SqlNodeVisitor *v) override { v->Visit(this); }

  /**
   * @return select columns
   */
  std::vector<AbstractExpression *> GetSelectColumns() { return select_; }

  /**
   * @return true if "SELECT DISTINCT", false otherwise
   */
  bool IsSelectDistinct() { return select_distinct_; }

  /**
   * @return table being selected from
   */
  std::shared_ptr<TableRef> GetSelectTable() { return from_; }

  /**
   * @return select condition
   */
  AbstractExpression *GetSelectCondition() { return where_; }

  /**
   * @return select group by
   */
  std::shared_ptr<GroupByDescription> GetSelectGroupBy() { return group_by_; }

  /**
   * @return select order by
   */
  std::shared_ptr<OrderByDescription> GetSelectOrderBy() { return order_by_; }

  /**
   * @return select limit
   */
  std::shared_ptr<LimitDescription> GetSelectLimit() { return limit_; }

  /**
   * Adds a select statement child as a union target.
   * @param select_stmt select statement to union with
   */
  void SetUnionSelect(std::shared_ptr<SelectStatement> select_stmt) { union_select_ = std::move(select_stmt); }

  /**
   * @return statement serialized to json
   */
  nlohmann::json ToJson() const override;

  /**
   * @param j json to deserialize
   */
  void FromJson(const nlohmann::json &j) override;

 private:
  std::vector<AbstractExpression *> select_;
  bool select_distinct_;
  std::shared_ptr<TableRef> from_;
  AbstractExpression *where_;
  std::shared_ptr<GroupByDescription> group_by_;
  std::shared_ptr<OrderByDescription> order_by_;
  std::shared_ptr<LimitDescription> limit_;
  std::shared_ptr<SelectStatement> union_select_;
};

DEFINE_JSON_DECLARATIONS(SelectStatement);

}  // namespace parser
}  // namespace terrier
