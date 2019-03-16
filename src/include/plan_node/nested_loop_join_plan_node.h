#pragma once

#include <memory>
#include <string>
#include <utility>
#include "plan_node/abstract_join_plan_node.h"

namespace terrier::plan_node {

class NestedLoopJoinPlanNode : public AbstractJoinPlanNode {
 public:
  NestedLoopJoinPlanNode(std::shared_ptr<OutputSchema> output_schema, LogicalJoinType join_type,
                         parser::AbstractExpression *predicate)
      : AbstractJoinPlanNode(std::move(output_schema), join_type, predicate) {}

  common::hash_t Hash() const override;

  bool operator==(const AbstractPlanNode &rhs) const override;

  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::NESTLOOP; }

  /**
   * @return debug info
   */
  const std::string GetInfo() const override { return "NestedLoopJoinPlanNode"; }

  std::unique_ptr<AbstractPlanNode> Copy() const override;

 private:
  DISALLOW_COPY_AND_MOVE(NestedLoopJoinPlanNode);
};

}  // namespace terrier::plan_node
