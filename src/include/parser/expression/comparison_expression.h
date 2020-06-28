#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "parser/expression/abstract_expression.h"
#include "parser/expression/constant_value_expression.h"
#include "parser/postgresparser.h"

#include "type/type_id.h"

namespace terrier::parser {

/**
 * ComparisonExpression represents comparisons between multiple expressions like < and >.
 */
class ComparisonExpression : public AbstractExpression {
 public:
  /**
   * Instantiates a new comparison expression.
   * @param cmp_type type of comparison
   * @param children vector containing exactly two children, left then right
   */
  ComparisonExpression(const ExpressionType cmp_type, std::vector<std::unique_ptr<AbstractExpression>> &&children)
      : AbstractExpression(cmp_type, type::TypeId::BOOLEAN, std::move(children)) {}

  /** Default constructor for deserialization. */
  ComparisonExpression() = default;

  /**
   * Copies the ComparisonExpression
   * @returns copy of this
   */
  std::unique_ptr<AbstractExpression> Copy() const override;

  /**
   * Creates a copy of the current AbstractExpression with new children implanted.
   * The children should not be owned by any other AbstractExpression.
   * @param children New children to be owned by the copy
   * @returns copy of this with new children
   */
  std::unique_ptr<AbstractExpression> CopyWithChildren(
      std::vector<std::unique_ptr<AbstractExpression>> &&children) const override;

  void Accept(common::ManagedPointer<binder::SqlNodeVisitor> v) override { v->Visit(common::ManagedPointer(this)); }
};

DEFINE_JSON_HEADER_DECLARATIONS(ComparisonExpression);

}  // namespace terrier::parser
