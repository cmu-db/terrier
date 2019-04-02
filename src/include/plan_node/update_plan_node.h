#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "parser/parameter.h"
#include "parser/update_statement.h"
#include "plan_node/abstract_plan_node.h"
#include "plan_node/output_schema.h"

namespace terrier {

namespace storage {
class SqlTable;
}
// TODO(Gus,Wen) Add back VisitParameters, SetParamaterValues, and PerformBinding

namespace plan_node {

class UpdatePlanNode : public AbstractPlanNode {
 public:
  /**
   * Builder for an delete plan node
   */
  class Builder : public AbstractPlanNode::Builder<Builder> {
   public:
    /**
     * Don't allow builder to be copied or moved
     */
    DISALLOW_COPY_AND_MOVE(Builder);

    /**
     * @param table_oid the OID of the target SQL table
     * @return builder object
     */
    Builder &SetTableOid(catalog::table_oid_t table_oid) {
      table_oid_ = table_oid;
      return *this;
    }

    /**
     * @param table_name name of the target table
     * @return builder object
     */
    Builder &SetTableName(std::string table_name) {
      table_name_ = std::move(table_name);
      return *this;
    }

    /**
     * @param update_primary_key whether to update primary key
     * @return builder object
     */
    Builder &SetUpdatePrimaryKey(bool update_primary_key) {
      update_primary_key_ = update_primary_key;
      return *this;
    }

    /**
     * Build the delete plan node
     * @return plan node
     */
    std::shared_ptr<UpdatePlanNode> Build() {
      return std::shared_ptr<UpdatePlanNode>(new UpdatePlanNode(
          std::move(children_), std::move(output_schema_), table_oid_, std::move(table_name_), update_primary_key_));
    }

   protected:
    /**
     * OID of the table to update
     */
    catalog::table_oid_t table_oid_;

    /**
     * Name of the table to update
     */
    std::string table_name_;

    /**
     * Whether to update primary key
     */
    bool update_primary_key_;
  };

 private:
  /**
   * @param children child plan nodes
   * @param output_schema Schema representing the structure of the output of this plan node
   * @param table_oid the OID of the target SQL table
   * @param table_name name of the target table
   * @param update_primary_key whether to update primary key
   */
  UpdatePlanNode(std::vector<std::unique_ptr<AbstractPlanNode>> &&children, std::shared_ptr<OutputSchema> output_schema,
                 catalog::table_oid_t table_oid, std::string table_name, bool update_primary_key)
      : AbstractPlanNode(std::move(children), std::move(output_schema)),
        table_oid_(table_oid),
        table_name_(std::move(table_name)),
        update_primary_key_(update_primary_key) {}

 public:
  UpdatePlanNode() = delete;

  /**
   * @return the OID of the target table to operate on
   */
  catalog::table_oid_t GetTableOid() const { return table_oid_; }

  /**
   * @return the name of the target table
   */
  const std::string &GetTableName() const { return table_name_; }

  /**
   * @return whether to update primary key
   */
  bool GetUpdatePrimaryKey() const { return update_primary_key_; }

  /**
   * @return the type of this plan node
   */
  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::UPDATE; }

  /**
   * @return the hashed value of this plan node
   */
  common::hash_t Hash() const override;

  bool operator==(const AbstractPlanNode &rhs) const override;

 private:
  /**
   * OID of the table to update
   */
  catalog::table_oid_t table_oid_;

  /**
   * Name of the table to update
   */
  std::string table_name_;

  /**
   * Whether to update primary key
   */
  bool update_primary_key_;

 public:
  /**
   * Don't allow plan to be copied or moved
   */
  DISALLOW_COPY_AND_MOVE(UpdatePlanNode);
};

}  // namespace plan_node
}  // namespace terrier
