#include "parser/expression/function_expression.h"

#include "binder/sql_node_visitor.h"
#include "common/hash_util.h"
#include "common/json.h"

namespace terrier::parser {

std::unique_ptr<AbstractExpression> FunctionExpression::Copy() const {
  std::vector<std::unique_ptr<AbstractExpression>> children;
  for (const auto &child : GetChildren()) {
    children.emplace_back(child->Copy());
  }
  return CopyWithChildren(std::move(children));
}

std::unique_ptr<AbstractExpression> FunctionExpression::CopyWithChildren(
    std::vector<std::unique_ptr<AbstractExpression>> &&children) const {
  std::string func_name = GetFuncName();
  auto expr = std::make_unique<FunctionExpression>(std::move(func_name), GetReturnValueType(), std::move(children));
  expr->SetMutableStateForCopy(*this);
  expr->SetProcOid(GetProcOid());
  return expr;
}

hash_t FunctionExpression::Hash() const {
  hash_t hash = AbstractExpression::Hash();
  hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(func_name_));
  return hash;
}

void FunctionExpression::Accept(common::ManagedPointer<binder::SqlNodeVisitor> v) {
  v->Visit(common::ManagedPointer(this));
}

nlohmann::json FunctionExpression::ToJson() const {
  nlohmann::json j = AbstractExpression::ToJson();
  j["func_name"] = func_name_;
  return j;
}

std::vector<std::unique_ptr<AbstractExpression>> FunctionExpression::FromJson(const nlohmann::json &j) {
  std::vector<std::unique_ptr<AbstractExpression>> exprs;
  auto e1 = AbstractExpression::FromJson(j);
  exprs.insert(exprs.end(), std::make_move_iterator(e1.begin()), std::make_move_iterator(e1.end()));
  func_name_ = j.at("func_name").get<std::string>();
  return exprs;
}

DEFINE_JSON_BODY_DECLARATIONS(FunctionExpression);

}  // namespace terrier::parser
