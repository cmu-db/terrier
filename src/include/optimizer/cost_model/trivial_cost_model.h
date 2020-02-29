#pragma once

#include "optimizer/cost_model/abstract_cost_model.h"
#include "optimizer/group_expression.h"
#include "optimizer/physical_operators.h"
#include "transaction/transaction_context.h"
#include <unordered_map>

namespace terrier {
namespace optimizer {

class Memo;
class GroupExpression;

/**
 * This cost model is meant to just be a trivial cost model. The decisions it makes are as follows
 * Always choose index scan (cost of 0) over sequential scan (cost of 1)
 * Choose NL if left rows is a single record (for single record lookup queries), else choose hash join
 * Choose hash group by over sort group by
 */
class TrivialCostModel : public AbstractCostModel {
 public:
  /**
   * Default constructor
   */
  TrivialCostModel() = default;

  /**
   * Costs a GroupExpression
   * @param txn TransactionContext that query is generated under
   * @param memo Memo object containing all relevant groups
   * @param gexpr GroupExpression to calculate cost for
   */
  CostModelEstimate CalculateCost(transaction::TransactionContext *txn, Memo *memo, GroupExpression *gexpr) override {
    gexpr_ = gexpr;
    memo_ = memo;
    txn_ = txn;
    gexpr_->Op().Accept(common::ManagedPointer<OperatorVisitor>(this));
    return CostModelEstimate(output_cost_, output_cardinalities_);
  };

  /**
   * Visit a SeqScan operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const SeqScan *op) override {
    output_cost_ = 1.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a IndexScan operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const IndexScan *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a QueryDerivedScan operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const QueryDerivedScan *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a OrderBy operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const OrderBy *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a Limit operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const Limit *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a InnerNLJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const InnerNLJoin *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a LeftNLJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const LeftNLJoin *op) override {}

  /**
   * Visit a RightNLJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const RightNLJoin *op) override {}

  /**
   * Visit a OuterNLJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const OuterNLJoin *op) override {}

  /**
   * Visit a InnerHashJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const InnerHashJoin *op) override {
    output_cost_ = 1.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a LeftHashJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const LeftHashJoin *op) override {}

  /**
   * Visit a RightHashJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const RightHashJoin *op) override {}

  /**
   * Visit a OuterHashJoin operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const OuterHashJoin *op) override {}

  /**
   * Visit a Insert operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const Insert *op) override {}

  /**
   * Visit a InsertSelect operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const InsertSelect *op) override {}

  /**
   * Visit a Delete operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const Delete *op) override {}

  /**
   * Visit a Update operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const Update *op) override {}

  /**
   * Visit a HashGroupBy operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const HashGroupBy *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a SortGroupBy operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const SortGroupBy *op) override {
    output_cost_ = 1.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

  /**
   * Visit a Aggregate operator
   * @param op operator
   */
  void Visit(UNUSED_ATTRIBUTE const Aggregate *op) override {
    output_cost_ = 0.f;
    output_cardinalities_[op->GetPlanNodeId()] = output_cost_;
  }

 private:
  /**
   * GroupExpression to cost
   */
  GroupExpression *gexpr_;

  /**
   * Memo table to use
   */
  Memo *memo_;

  /**
   * Transaction Context
   */
  transaction::TransactionContext *txn_;

  /**
   * Computed output cost
   */
  double output_cost_ = 0;

  std::unordered_map<plan_node_id_t, double> output_cardinalities_;
};

}  // namespace optimizer
}  // namespace terrier
