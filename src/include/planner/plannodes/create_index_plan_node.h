#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "catalog/index_schema.h"
#include "parser/create_statement.h"
#include "parser/expression/column_value_expression.h"
#include "planner/plannodes/abstract_plan_node.h"

namespace terrier::planner {

/**
 * Plan node for creating indexes
 */
class CreateIndexPlanNode : public AbstractPlanNode {
 public:
  /**
   * Builder for a create index plan node
   */
  class Builder : public AbstractPlanNode::Builder<Builder> {
   public:
    Builder() = default;

    /**
     * Don't allow builder to be copied or moved
     */
    DISALLOW_COPY_AND_MOVE(Builder);

    /**
     * @param database_oid  OID of the database
     * @return builder object
     */
    Builder &SetDatabaseOid(catalog::db_oid_t database_oid) {
      database_oid_ = database_oid;
      return *this;
    }

    /**
     * @param namespace_oid OID of the namespace
     * @return builder object
     */
    Builder &SetNamespaceOid(catalog::namespace_oid_t namespace_oid) {
      namespace_oid_ = namespace_oid;
      return *this;
    }

    /**
     * @param table_oid OID of the table to create index on
     * @return builder object
     */
    Builder &SetTableOid(catalog::table_oid_t table_oid) {
      table_oid_ = table_oid;
      return *this;
    }

    /**
     * @param index_name the name of the index
     * @return builder object
     */
    Builder &SetIndexName(std::string index_name) {
      index_name_ = std::move(index_name);
      return *this;
    }

    Builder &SetSchema(std::unique_ptr<catalog::IndexSchema> schema) {
      schema_ = std::move(schema);
      return *this;
    }

    /**
     * Build the create index plan node
     * @return plan node
     */
    std::unique_ptr<CreateIndexPlanNode> Build() {
      return std::unique_ptr<CreateIndexPlanNode>(
          new CreateIndexPlanNode(std::move(children_), std::move(output_schema_), database_oid_, namespace_oid_,
                                  table_oid_, std::move(index_name_)));
    }

   protected:
    /**
     * OID of the database
     */
    catalog::db_oid_t database_oid_;

    /**
     * OID of namespace
     */
    catalog::namespace_oid_t namespace_oid_;

    /**
     * OID of the table to create index on
     */
    catalog::table_oid_t table_oid_;

    /**
     * Name of the Index
     */
    std::string index_name_;

    std::unique_ptr<catalog::IndexSchema> schema_;
  };

 private:
  /**
   * @param children child plan nodes
   * @param output_schema Schema representing the structure of the output of this plan node
   * @param database_oid OID of the database
   * @param namespace_oid OID of the namespace
   * @param table_oid OID of the table to create index on
   * @param table_schema schema of the table to create
   * @param index_type type of index to create
   * @param unique_index true if index should be unique
   * @param index_name name of index to be created
   */
  CreateIndexPlanNode(std::vector<std::unique_ptr<AbstractPlanNode>> &&children,
                      std::unique_ptr<OutputSchema> output_schema, catalog::db_oid_t database_oid,
                      catalog::namespace_oid_t namespace_oid, catalog::table_oid_t table_oid, std::string index_name)
      : AbstractPlanNode(std::move(children), std::move(output_schema)),
        database_oid_(database_oid),
        namespace_oid_(namespace_oid),
        table_oid_(table_oid),
        index_name_(std::move(index_name)) {}

 public:
  /**
   * Default constructor for deserialization
   */
  CreateIndexPlanNode() = default;

  DISALLOW_COPY_AND_MOVE(CreateIndexPlanNode)

  /**
   * @return the type of this plan node
   */
  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::CREATE_INDEX; }

  /**
   * @return OID of the database
   */
  catalog::db_oid_t GetDatabaseOid() const { return database_oid_; }

  /**
   * @return OID of the namespace
   */
  catalog::namespace_oid_t GetNamespaceOid() const { return namespace_oid_; }

  /**
   * @return OID of the table to create index on
   */
  catalog::table_oid_t GetTableOid() const { return table_oid_; }

  /**
   * @return name of the index
   */
  const std::string &GetIndexName() const { return index_name_; }

  /**
   * @return pointer to the schema
   */
  common::ManagedPointer<catalog::IndexSchema> GetSchema() const { return common::ManagedPointer(schema_); }

  /**
   * @return the hashed value of this plan node
   */
  common::hash_t Hash() const override;

  bool operator==(const AbstractPlanNode &rhs) const override;

  nlohmann::json ToJson() const override;
  std::vector<std::unique_ptr<parser::AbstractExpression>> FromJson(const nlohmann::json &j) override;

 private:
  catalog::db_oid_t database_oid_;
  catalog::namespace_oid_t namespace_oid_;
  catalog::table_oid_t table_oid_;
  std::string index_name_;
  std::unique_ptr<catalog::IndexSchema> schema_;
};

DEFINE_JSON_DECLARATIONS(CreateIndexPlanNode);

}  // namespace terrier::planner
