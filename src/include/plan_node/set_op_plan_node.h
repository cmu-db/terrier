#pragma once

#include <memory>
#include <string>
#include <utility>
#include "plan_node/abstract_plan_node.h"
#include "plan_node/output_schema.h"

namespace terrier::plan_node {

/**
 * Plan node for set operation:
 * INTERSECT/INTERSECT ALL/EXPECT/EXCEPT ALL
 *
 * @warning UNION (ALL) is handled differently.
 * IMPORTANT: Both children must have the same physical schema.
 */
class SetOpPlanNode : public AbstractPlanNode {
 protected:
  /**
   * Builder for an delete plan node
   */
  class Builder : public AbstractPlanNode::Builder<Builder> {
   public:
    DISALLOW_COPY_AND_MOVE(Builder);

    /**
     * @param set_op set operation of this plan node
     * @return builder object
     */
    Builder &SetSetOp(SetOpType set_op) {
      set_op_ = set_op;
      return *this;
    }

    /**
     * Build the setop plan node
     * @return plan node
     */
    std::shared_ptr<SetOpPlanNode> Build() {
      return std::shared_ptr<SetOpPlanNode>(
          new SetOpPlanNode(std::move(children_), std::move(output_schema_), estimated_cardinality_, set_op_));
    }

   protected:
    SetOpType set_op_;
  };

  /**
   * @param children child plan nodes
   * @param output_schema Schema representing the structure of the output of this plan node
   * @param estimated_cardinality estimated cardinality of output of node
   * @param set_op the set pperation of this node
   */
  SetOpPlanNode(std::vector<std::unique_ptr<AbstractPlanNode>> &&children, std::shared_ptr<OutputSchema> output_schema,
                uint32_t estimated_cardinality, SetOpType set_op)
      : AbstractPlanNode(std::move(children), std::move(output_schema), estimated_cardinality), set_op_(set_op) {}

 public:
  /**
   * @return the set operation of this node
   */
  SetOpType GetSetOp() const { return set_op_; }

  /**
   * @return the type of this plan node
   */
  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::SETOP; }

 private:
  // Set Operation of this node
  SetOpType set_op_;

 public:
  DISALLOW_COPY_AND_MOVE(SetOpPlanNode);
};

}  // namespace terrier::plan_node
