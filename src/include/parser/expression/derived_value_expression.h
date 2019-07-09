#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "parser/expression/abstract_expression.h"

namespace terrier::parser {

/**
 * Represents a logical tuple value.
 */
class DerivedValueExpression : public AbstractExpression {
 public:
  /**
   * This constructor is called by the optimizer
   * @param type type of the return value of the expression
   * @param tuple_idx index of the tuple
   * @param value_idx index of the value
   */
  DerivedValueExpression(type::TypeId type, int tuple_idx, int value_idx)
      : AbstractExpression(ExpressionType::VALUE_TUPLE, type, {}), tuple_idx_(tuple_idx), value_idx_(value_idx) {}

  /**
   * Default constructor for deserialization
   */
  DerivedValueExpression() = default;

  std::shared_ptr<AbstractExpression> Copy() const override { return std::make_shared<DerivedValueExpression>(*this); }

  /**
   * @return tuple index
   */
  int GetTupleIdx() const { return tuple_idx_; }

  /**
   * @return value index
   */
  int GetValueIdx() const { return value_idx_; }

  common::hash_t Hash() const override {
    common::hash_t hash = AbstractExpression::Hash();
    hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(tuple_idx_));
    hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(value_idx_));
    return hash;
  }

  bool operator==(const AbstractExpression &rhs) const override {
    if (!AbstractExpression::operator==(rhs)) return false;
    auto const &other = dynamic_cast<const DerivedValueExpression &>(rhs);
    return GetTupleIdx() == other.GetTupleIdx() && GetValueIdx() == other.GetValueIdx();
  }

  void Accept(SqlNodeVisitor *v) override { v->Visit(this); }

  /**
   * @return expression serialized to json
   */
  nlohmann::json ToJson() const override {
    nlohmann::json j = AbstractExpression::ToJson();
    j["tuple_idx"] = tuple_idx_;
    j["value_idx"] = value_idx_;
    return j;
  }

  /**
   * @param j json to deserialize
   */
  void FromJson(const nlohmann::json &j) override {
    AbstractExpression::FromJson(j);
    tuple_idx_ = j.at("tuple_idx").get<int>();
    value_idx_ = j.at("value_idx").get<int>();
  }

 private:
  int tuple_idx_;
  int value_idx_;
};

DEFINE_JSON_DECLARATIONS(DerivedValueExpression);

}  // namespace terrier::parser
