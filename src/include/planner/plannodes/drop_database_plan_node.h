#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "catalog/catalog_defs.h"
#include "common/hash_util.h"
#include "common/json.h"
#include "common/macros.h"
#include "common/managed_pointer.h"
#include "nlohmann/json.hpp"
#include "parser/drop_statement.h"
#include "planner/plannodes/abstract_plan_node.h"
#include "planner/plannodes/output_schema.h"
#include "planner/plannodes/plan_node_defs.h"
#include "planner/plannodes/plan_visitor.h"

namespace terrier {
namespace parser {
class AbstractExpression;
}  // namespace parser
}  // namespace terrier

namespace terrier::planner {
/**
 *  The plan node for dropping databases
 */
class DropDatabasePlanNode : public AbstractPlanNode {
 public:
  /**
   * Builder for a drop database plan node
   */
  class Builder : public AbstractPlanNode::Builder<Builder> {
   public:
    Builder() = default;

    /**
     * Don't allow builder to be copied or moved
     */
    DISALLOW_COPY_AND_MOVE(Builder);

    /**
     * @param database_oid the OID of the database to drop
     * @return builder object
     */
    Builder &SetDatabaseOid(catalog::db_oid_t database_oid) {
      database_oid_ = database_oid;
      return *this;
    }

    /**
     * Build the drop database plan node
     * @return plan node
     */
    std::unique_ptr<DropDatabasePlanNode> Build() {
      return std::unique_ptr<DropDatabasePlanNode>(
          new DropDatabasePlanNode(std::move(children_), std::move(output_schema_), database_oid_));
    }

   protected:
    /**
     * OID of the database to drop
     */
    catalog::db_oid_t database_oid_;
  };

 private:
  /**
   * @param children child plan nodes
   * @param output_schema Schema representing the structure of the output of this plan node
   * @param database_oid OID of the database to drop
   */
  DropDatabasePlanNode(std::vector<std::unique_ptr<AbstractPlanNode>> &&children,
                       std::unique_ptr<OutputSchema> output_schema, catalog::db_oid_t database_oid)
      : AbstractPlanNode(std::move(children), std::move(output_schema)), database_oid_(database_oid) {}

 public:
  /**
   * Default constructor for deserialization
   */
  DropDatabasePlanNode() = default;

  DISALLOW_COPY_AND_MOVE(DropDatabasePlanNode)

  /**
   * @return the type of this plan node
   */
  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::DROP_DATABASE; }

  /**
   * @return OID of the database to drop
   */
  catalog::db_oid_t GetDatabaseOid() const { return database_oid_; }

  /**
   * @return the hashed value of this plan node
   */
  common::hash_t Hash() const override;

  bool operator==(const AbstractPlanNode &rhs) const override;

  void Accept(common::ManagedPointer<PlanVisitor> v) const override { v->Visit(this); }

  nlohmann::json ToJson() const override;
  std::vector<std::unique_ptr<parser::AbstractExpression>> FromJson(const nlohmann::json &j) override;

 private:
  catalog::db_oid_t database_oid_;
};

DEFINE_JSON_DECLARATIONS(DropDatabasePlanNode);

}  // namespace terrier::planner
