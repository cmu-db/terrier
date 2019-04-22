#include "planner/plannodes/csv_scan_plan_node.h"
#include <memory>
#include <string>

namespace terrier::planner {

common::hash_t CSVScanPlanNode::Hash() const {
  common::hash_t hash = AbstractScanPlanNode::Hash();
  hash = common::HashUtil::CombineHashes(hash, std::hash<std::string>{}(file_name_));
  hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(&delimiter_));
  hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(&quote_));
  hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(&escape_));
  hash = common::HashUtil::CombineHashes(hash, std::hash<std::string>{}(null_string_));

  return hash;
}

// TODO(Gus): Are file names case sensitive?
bool CSVScanPlanNode::operator==(const AbstractPlanNode &rhs) const {
  if (!AbstractScanPlanNode::operator==(rhs)) return false;

  const auto &other = static_cast<const CSVScanPlanNode &>(rhs);
  return file_name_ == other.file_name_ && delimiter_ == other.delimiter_ && quote_ == other.quote_ &&
          escape_ == other.escape_ && null_string_ == other.null_string_;
}

}  // namespace terrier::planner
