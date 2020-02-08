#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "catalog/catalog_defs.h"
#include "parser/drop_statement.h"
#include "planner/plannodes/abstract_plan_node.h"

namespace terrier::planner {
/**
 *  The plan node for dropping indexes
 */
class DropIndexPlanNode : public AbstractPlanNode {
 public:
  /**
   * Builder for a drop index plan node
   */
  class Builder : public AbstractPlanNode::Builder<Builder> {
   public:
    Builder() = default;

    /**
     * Don't allow builder to be copied or moved
     */
    DISALLOW_COPY_AND_MOVE(Builder);

    /**
     * @param index_oid the OID of the index to drop
     * @return builder object
     */
    Builder &SetIndexOid(catalog::index_oid_t index_oid) {
      index_oid_ = index_oid;
      return *this;
    }

    /**
     * Build the drop index plan node
     * @return plan node
     */
    std::unique_ptr<DropIndexPlanNode> Build() {
      return std::unique_ptr<DropIndexPlanNode>(
          new DropIndexPlanNode(std::move(children_), std::move(output_schema_), index_oid_, plan_node_id_));
    }

   protected:
    /**
     * OID of the index to drop
     */
    catalog::index_oid_t index_oid_;
  };

 private:
  /**
   * @param children child plan nodes
   * @param output_schema Schema representing the structure of the output of this plan node
   * @param database_oid OID of the database
   * @param namespace_oid OID of the namespace
   * @param index_oid OID of the index to drop
   */
  DropIndexPlanNode(std::vector<std::unique_ptr<AbstractPlanNode>> &&children,
                    std::unique_ptr<OutputSchema> output_schema, catalog::index_oid_t index_oid,
                    plan_node_id_t plan_node_id)
      : AbstractPlanNode(std::move(children), std::move(output_schema), plan_node_id), index_oid_(index_oid) {}

 public:
  /**
   * Default constructor for deserialization
   */
  DropIndexPlanNode() = default;

  DISALLOW_COPY_AND_MOVE(DropIndexPlanNode)

  /**
   * @return the type of this plan node
   */
  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::DROP_INDEX; }

  /**
   * @return OID of the index to drop
   */
  catalog::index_oid_t GetIndexOid() const { return index_oid_; }

  /**
   * @return the hashed value of this plan node
   */
  common::hash_t Hash() const override;

  bool operator==(const AbstractPlanNode &rhs) const override;

  nlohmann::json ToJson() const override;
  std::vector<std::unique_ptr<parser::AbstractExpression>> FromJson(const nlohmann::json &j) override;

 private:
  /**
   * OID of the index to drop
   */
  catalog::index_oid_t index_oid_;
};

DEFINE_JSON_DECLARATIONS(DropIndexPlanNode);

}  // namespace terrier::planner
