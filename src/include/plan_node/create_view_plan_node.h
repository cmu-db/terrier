#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "parser/create_statement.h"
#include "parser/select_statement.h"
#include "plan_node/abstract_plan_node.h"

namespace terrier::plan_node {

/**
 * Plan node for creating views
 */
class CreateViewPlanNode : public AbstractPlanNode {
 public:
  /**
   * Builder for a create view plan node
   */
  class Builder : public AbstractPlanNode::Builder<Builder> {
   public:
    Builder() = default;

    /**
     * Don't allow builder to be copied or moved
     */
    DISALLOW_COPY_AND_MOVE(Builder);

    /**
     * @param view_name  view name
     * @return builder object
     */
    Builder &SetViewName(std::string view_name) {
      view_name_ = std::move(view_name);
      return *this;
    }

    /**
     * @param view_query view query
     * @return builder object
     */
    Builder &SetViewQuery(std::shared_ptr<parser::SelectStatement> view_query) {
      view_query_ = std::move(view_query);
      return *this;
    }

    /**
     * @param create_stmt the SQL CREATE statement
     * @return builder object
     */
    Builder &SetFromCreateStatement(parser::CreateStatement *create_stmt) {
      // TODO(Gus,Wen) Need to implement
      // if (create_stmt->GetCreateType() == parser::CreateStatement::kView) {
      //}
      return *this;
    }

    /**
     * Build the create view plan node
     * @return plan node
     */
    std::unique_ptr<CreateViewPlanNode> Build() {
      return std::unique_ptr<CreateViewPlanNode>(new CreateViewPlanNode(std::move(children_), std::move(output_schema_),
                                                                        std::move(view_name_), std::move(view_query_)));
    }

   protected:
    /**
     * Name of the view
     */
    std::string view_name_;

    /**
     * View query
     */
    std::shared_ptr<parser::SelectStatement> view_query_;
  };

 private:
  /**
   * @param children child plan nodes
   * @param output_schema Schema representing the structure of the output of this plan node
   * @param view_name  view name
   * @param view_query view query
   */
  CreateViewPlanNode(std::vector<std::unique_ptr<AbstractPlanNode>> &&children,
                     std::shared_ptr<OutputSchema> output_schema, std::string view_name,
                     std::shared_ptr<parser::SelectStatement> view_query)
      : AbstractPlanNode(std::move(children), std::move(output_schema)),
        view_name_(std::move(view_name)),
        view_query_(std::move(view_query)) {}

 public:
  CreateViewPlanNode() = delete;
  /**
   * @return the type of this plan node
   */
  PlanNodeType GetPlanNodeType() const override { return PlanNodeType::CREATE_VIEW; }

  /**
   * @return view name
   */
  const std::string &GetViewName() const { return view_name_; }

  /**
   * @return view query
   */
  std::shared_ptr<parser::SelectStatement> GetViewQuery() { return view_query_; }

  /**
   * @return the hashed value of this plan node
   */
  common::hash_t Hash() const override;

  bool operator==(const AbstractPlanNode &rhs) const override;

 private:
  /**
   * Name of the view
   */
  std::string view_name_;

  /**
   * View query
   */
  std::shared_ptr<parser::SelectStatement> view_query_;

 public:
  /**
   * Don't allow plan to be copied or moved
   */
  DISALLOW_COPY_AND_MOVE(CreateViewPlanNode);
};

}  // namespace terrier::plan_node
