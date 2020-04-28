#pragma once

#include <memory>
#include <vector>

#include "execution/compiler/expression/expression_translator.h"

namespace terrier {
namespace execution {
namespace ast {
class Expr;
}  // namespace ast
namespace compiler {
class CodeGen;
}  // namespace compiler
}  // namespace execution
namespace parser {
class AbstractExpression;
}  // namespace parser
}  // namespace terrier

namespace terrier::execution::compiler {

/**
 * Function Translator
 */
class FunctionTranslator : public ExpressionTranslator {
 public:
  /**
   * Constructor
   * @param expression expression to translate
   * @param codegen code generator to use
   */
  FunctionTranslator(const terrier::parser::AbstractExpression *expression, CodeGen *codegen);

  ast::Expr *DeriveExpr(ExpressionEvaluator *evaluator) override;

 private:
  std::vector<std::unique_ptr<ExpressionTranslator>> params_;
};
}  // namespace terrier::execution::compiler
