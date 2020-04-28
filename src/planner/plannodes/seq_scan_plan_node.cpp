#include <iterator>
#include <memory>
#include <vector>

#include "catalog/catalog_defs.h"
#include "common/hash_util.h"
#include "nlohmann/json.hpp"
#include "parser/expression/abstract_expression.h"
#include "planner/plannodes/abstract_scan_plan_node.h"
#include "planner/plannodes/seq_scan_plan_node.h"

namespace terrier {
namespace planner {
class AbstractPlanNode;
}  // namespace planner
}  // namespace terrier

namespace terrier::planner {

common::hash_t SeqScanPlanNode::Hash() const {
  common::hash_t hash = AbstractScanPlanNode::Hash();
  hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(table_oid_));
  hash = common::HashUtil::CombineHashInRange(hash, column_oids_.begin(), column_oids_.end());
  return hash;
}

bool SeqScanPlanNode::operator==(const AbstractPlanNode &rhs) const {
  auto &other = static_cast<const SeqScanPlanNode &>(rhs);
  if (!AbstractScanPlanNode::operator==(rhs)) return false;
  if (table_oid_ != other.table_oid_) return false;
  return column_oids_ == other.column_oids_;
}

nlohmann::json SeqScanPlanNode::ToJson() const {
  nlohmann::json j = AbstractScanPlanNode::ToJson();
  j["column_oids"] = column_oids_;
  j["table_oid"] = table_oid_;
  return j;
}

std::vector<std::unique_ptr<parser::AbstractExpression>> SeqScanPlanNode::FromJson(const nlohmann::json &j) {
  std::vector<std::unique_ptr<parser::AbstractExpression>> exprs;
  auto e1 = AbstractScanPlanNode::FromJson(j);
  exprs.insert(exprs.end(), std::make_move_iterator(e1.begin()), std::make_move_iterator(e1.end()));
  column_oids_ = j.at("column_oids").get<std::vector<catalog::col_oid_t>>();
  table_oid_ = j.at("table_oid").get<catalog::table_oid_t>();
  return exprs;
}

}  // namespace terrier::planner
