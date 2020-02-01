#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "parser/expression/abstract_expression.h"
#include "parser/expression/constant_value_expression.h"
#include "parser/expression/type_cast_expression.h"
#include "parser/expression_defs.h"
#include "type/transient_value_factory.h"
#include "type/transient_value_peeker.h"
#include "util/time_util.h"

namespace terrier::binder {
/** Utility functions for the binder's role in semantically validating and transforming input. */
class BinderUtil {
 public:
  /**
   * Create the real expression represented by @p expr. Does not modify the existing expression.
   *
   * This currently handles the following conversions:
   *   - Dates/timestamps come in as strings. This parses the strings into a Date/Timestamp TransientValue.
   *   - Type-cast expressions are unwrapped and follow the above rules for conversion.
   *
   * @param expr The expression to convert, this is not modified.
   * @param expected_ret_type The expected return type of this expression.
   * @return A pointer to the new converted expression.
   */
  static std::unique_ptr<parser::AbstractExpression> Convert(common::ManagedPointer<parser::AbstractExpression> expr,
                                                             type::TypeId expected_ret_type) {
    auto expr_type = expr->GetExpressionType();
    auto expr_ret_type = expr->GetReturnValueType();

    TERRIER_ASSERT(expr_type == parser::ExpressionType::OPERATOR_CAST || expr_ret_type != expected_ret_type,
                   "Types already compatible!");

    switch (expr_type) {
      case parser::ExpressionType::VALUE_CONSTANT: {
        auto cexpr = expr.CastManagedPointerTo<parser::ConstantValueExpression>();
        std::string str{type::TransientValuePeeker::PeekVarChar(cexpr->GetValue())};

        if (expected_ret_type == type::TypeId::DATE) {
          auto parsed = util::TimeConvertor::ParseDate(str);
          if (!parsed.first) {
            throw BINDER_EXCEPTION("Unable to parse the date.");
          }
          return std::make_unique<parser::ConstantValueExpression>(type::TransientValueFactory::GetDate(parsed.second));
        }

        if (expected_ret_type == type::TypeId::TIMESTAMP) {
          auto parsed = util::TimeConvertor::ParseTimestamp(str);
          if (!parsed.first) {
            throw BINDER_EXCEPTION("Unable to parse the timestamp.");
          }
          return std::make_unique<parser::ConstantValueExpression>(
              type::TransientValueFactory::GetTimestamp(parsed.second));
        }
      }
      case parser::ExpressionType::OPERATOR_CAST: {
        auto cast_expr = expr.CastManagedPointerTo<parser::TypeCastExpression>();
        TERRIER_ASSERT(cast_expr->GetChildrenSize() == 1, "Cannot type-cast multiple children.");
        auto child = cast_expr->GetChild(0);
        return Convert(child, expected_ret_type);
      }
      default:
        throw BINDER_EXCEPTION("Mismatch in expected return type and expression return type.");
    }
  }
};

}  // namespace terrier::binder
