#include "execution/codegen/compilation_context.h"

#include <algorithm>
#include <atomic>
#include <sstream>

#include "common/exception.h"
#include "common/macros.h"
#include "execution/ast/context.h"
#include "execution/codegen/codegen.h"
#include "execution/codegen/executable_query.h"
#include "execution/codegen/executable_query_builder.h"
#include "execution/codegen/expression//derived_value_translator.h"
#include "execution/codegen/expression/arithmetic_translator.h"
#include "execution/codegen/expression/builtin_function_translator.h"
#include "execution/codegen/expression/column_value_translator.h"
#include "execution/codegen/expression/comparison_translator.h"
#include "execution/codegen/expression/conjunction_translator.h"
#include "execution/codegen/expression/constant_translator.h"
#include "execution/codegen/expression/null_check_translator.h"
#include "execution/codegen/expression/unary_translator.h"
#include "execution/codegen/function_builder.h"
#include "execution/codegen/operators/csv_scan_translator.h"
#include "execution/codegen/operators/hash_aggregation_translator.h"
#include "execution/codegen/operators/hash_join_translator.h"
#include "execution/codegen/operators/limit_translator.h"
#include "execution/codegen/operators/nested_loop_join_translator.h"
#include "execution/codegen/operators/operator_translator.h"
#include "execution/codegen/operators/output_translator.h"
#include "execution/codegen/operators/projection_translator.h"
#include "execution/codegen/operators/seq_scan_translator.h"
#include "execution/codegen/operators/sort_translator.h"
#include "execution/codegen/operators/static_aggregation_translator.h"
#include "execution/codegen/pipeline.h"
#include "parser/expression/abstract_expression.h"
#include "parser/expression/column_value_expression.h"
#include "parser/expression/comparison_expression.h"
#include "parser/expression/conjunction_expression.h"
#include "parser/expression/derived_value_expression.h"
#include "parser/expression/operator_expression.h"
#include "planner/plannodes/abstract_plan_node.h"
#include "planner/plannodes/aggregate_plan_node.h"
#include "planner/plannodes/csv_scan_plan_node.h"
#include "planner/plannodes/hash_join_plan_node.h"
#include "planner/plannodes/limit_plan_node.h"
#include "planner/plannodes/nested_loop_join_plan_node.h"
#include "planner/plannodes/order_by_plan_node.h"
#include "planner/plannodes/projection_plan_node.h"
#include "planner/plannodes/seq_scan_plan_node.h"
#include "planner/plannodes/set_op_plan_node.h"
#include "spdlog/fmt/fmt.h"

namespace terrier::execution::codegen {

namespace {
// A unique ID generator used to generate globally unique TPL function names.
std::atomic<uint64_t> kUniqueIds{0};
}  // namespace

CompilationContext::CompilationContext(ExecutableQuery *query, const CompilationMode mode)
    : unique_id_(kUniqueIds++),
      query_(query),
      mode_(mode),
      codegen_(query_->GetContext()),
      query_state_var_(codegen_.MakeIdentifier("queryState")),
      query_state_type_(codegen_.MakeIdentifier("QueryState")),
      query_state_(query_state_type_, [this](CodeGen *codegen) { return codegen->MakeExpr(query_state_var_); }) {}

ast::FunctionDecl *CompilationContext::GenerateInitFunction() {
  const auto name = codegen_.MakeIdentifier(GetFunctionPrefix() + "_Init");
  FunctionBuilder builder(&codegen_, name, QueryParams(), codegen_.Nil());
  {
    // Request new scope for the function.
    CodeGen::CodeScope code_scope(&codegen_);
    for (auto &[_, op] : ops_) {
      (void)_;
      op->InitializeQueryState(&builder);
    }
  }
  return builder.Finish();
}

ast::FunctionDecl *CompilationContext::GenerateTearDownFunction() {
  const auto name = codegen_.MakeIdentifier(GetFunctionPrefix() + "_TearDown");
  FunctionBuilder builder(&codegen_, name, QueryParams(), codegen_.Nil());
  {
    // Request new scope for the function.
    CodeGen::CodeScope code_scope(&codegen_);
    for (auto &[_, op] : ops_) {
      (void)_;
      op->TearDownQueryState(&builder);
    }
  }
  return builder.Finish();
}

void CompilationContext::GeneratePlan(const planner::AbstractPlanNode &plan) {
  exec_ctx_ =
      query_state_.DeclareStateEntry(GetCodeGen(), "execCtx", codegen_.PointerType(ast::BuiltinType::ExecutionContext));

  // Recursively prepare all translators for the query.
  Pipeline main_pipeline(this);
  if (plan.GetOutputSchema()->NumColumns() != 0) {
    PrepareOut(plan, &main_pipeline);
  } else {
    Prepare(plan, &main_pipeline);
  }
  query_state_.ConstructFinalType(&codegen_);

  // Collect top-level structures and declarations.
  util::RegionVector<ast::StructDecl *> top_level_structs(query_->GetContext()->GetRegion());
  util::RegionVector<ast::FunctionDecl *> top_level_funcs(query_->GetContext()->GetRegion());
  for (auto &[_, op] : ops_) {
    (void)_;
    op->DefineHelperStructs(&top_level_structs);
    op->DefineHelperFunctions(&top_level_funcs);
  }
  top_level_structs.push_back(query_state_.GetType());

  // All fragments.
  std::vector<std::unique_ptr<ExecutableQuery::Fragment>> fragments;

  // The main builder. The initialization and tear-down code go here. In
  // one-shot compilation, all query code goes here, too.
  ExecutableQueryFragmentBuilder main_builder(query_->GetContext());
  main_builder.DeclareAll(top_level_structs);
  main_builder.DeclareAll(top_level_funcs);
  main_builder.RegisterStep(GenerateInitFunction());

  // Generate each pipeline.
  std::vector<Pipeline *> execution_order;
  main_pipeline.CollectDependencies(&execution_order);
  for (auto *pipeline : execution_order) {
    pipeline->Prepare();
    pipeline->GeneratePipeline(&main_builder);
  }

  // Register the tear-down function.
  main_builder.RegisterStep(GenerateTearDownFunction());

  // Compile and finish.
  fragments.emplace_back(main_builder.Compile());
  query_->Setup(std::move(fragments), query_state_.GetSize());
}

// static
std::unique_ptr<ExecutableQuery> CompilationContext::Compile(const planner::AbstractPlanNode &plan,
                                                             const CompilationMode mode) {
  // The query we're generating code for.
  auto query = std::make_unique<ExecutableQuery>(plan);

  // Generate the plan for the query
  CompilationContext ctx(query.get(), mode);
  ctx.GeneratePlan(plan);

  // Done
  return query;
}

uint32_t CompilationContext::RegisterPipeline(Pipeline *pipeline) {
  TERRIER_ASSERT(std::find(pipelines_.begin(), pipelines_.end(), pipeline) == pipelines_.end(),
                 "Duplicate pipeline in context");
  pipelines_.push_back(pipeline);
  return pipelines_.size();
}

void CompilationContext::PrepareOut(const planner::AbstractPlanNode &plan, Pipeline *pipeline) {
  auto translator = std::make_unique<OutputTranslator>(plan, this, pipeline);
  ops_[nullptr] = std::move(translator);
}

void CompilationContext::Prepare(const planner::AbstractPlanNode &plan, Pipeline *pipeline) {
  std::unique_ptr<OperatorTranslator> translator;

  switch (plan.GetPlanNodeType()) {
    case planner::PlanNodeType::AGGREGATE: {
      const auto &aggregation = static_cast<const planner::AggregatePlanNode &>(plan);
      if (aggregation.GetAggregateStrategyType() == planner::AggregateStrategyType::SORTED) {
        throw NOT_IMPLEMENTED_EXCEPTION("Code generation for sort-based aggregations.");
      }
      if (aggregation.GetGroupByTerms().empty()) {
        translator = std::make_unique<StaticAggregationTranslator>(aggregation, this, pipeline);
      } else {
        translator = std::make_unique<HashAggregationTranslator>(aggregation, this, pipeline);
      }
      break;
    }
    case planner::PlanNodeType::CSVSCAN: {
      const auto &scan_plan = static_cast<const planner::CSVScanPlanNode &>(plan);
      translator = std::make_unique<CSVScanTranslator>(scan_plan, this, pipeline);
      break;
    }
    case planner::PlanNodeType::HASHJOIN: {
      const auto &hash_join = static_cast<const planner::HashJoinPlanNode &>(plan);
      translator = std::make_unique<HashJoinTranslator>(hash_join, this, pipeline);
      break;
    }
    case planner::PlanNodeType::LIMIT: {
      const auto &limit = static_cast<const planner::LimitPlanNode &>(plan);
      translator = std::make_unique<LimitTranslator>(limit, this, pipeline);
      break;
    }
    case planner::PlanNodeType::NESTLOOP: {
      const auto &nested_loop = static_cast<const planner::NestedLoopJoinPlanNode &>(plan);
      translator = std::make_unique<NestedLoopJoinTranslator>(nested_loop, this, pipeline);
      break;
    }
    case planner::PlanNodeType::ORDERBY: {
      const auto &sort = static_cast<const planner::OrderByPlanNode &>(plan);
      translator = std::make_unique<SortTranslator>(sort, this, pipeline);
      break;
    }
    case planner::PlanNodeType::PROJECTION: {
      const auto &projection = static_cast<const planner::ProjectionPlanNode &>(plan);
      translator = std::make_unique<ProjectionTranslator>(projection, this, pipeline);
      break;
    }
    case planner::PlanNodeType::SEQSCAN: {
      const auto &seq_scan = static_cast<const planner::SeqScanPlanNode &>(plan);
      translator = std::make_unique<SeqScanTranslator>(seq_scan, this, pipeline);
      break;
    }
    default: {
      throw NOT_IMPLEMENTED_EXCEPTION(fmt::format("code generation for plan node type '{}'",
                                                  planner::PlanNodeTypeToString(plan.GetPlanNodeType())));
    }
  }

  ops_[&plan] = std::move(translator);
}

void CompilationContext::Prepare(const parser::AbstractExpression &expression) {
  std::unique_ptr<ExpressionTranslator> translator;

  switch (expression.GetExpressionType()) {
    case parser::ExpressionType::COLUMN_VALUE: {
      const auto &column_value = static_cast<const parser::ColumnValueExpression &>(expression);
      translator = std::make_unique<ColumnValueTranslator>(column_value, this);
      break;
    }
    case parser::ExpressionType::COMPARE_EQUAL:
    case parser::ExpressionType::COMPARE_GREATER_THAN:
    case parser::ExpressionType::COMPARE_GREATER_THAN_OR_EQUAL_TO:
    case parser::ExpressionType::COMPARE_LESS_THAN:
    case parser::ExpressionType::COMPARE_LESS_THAN_OR_EQUAL_TO:
    case parser::ExpressionType::COMPARE_NOT_EQUAL:
    case parser::ExpressionType::COMPARE_LIKE:
    case parser::ExpressionType::COMPARE_NOT_LIKE: {
      const auto &comparison = static_cast<const parser::ComparisonExpression &>(expression);
      translator = std::make_unique<ComparisonTranslator>(comparison, this);
      break;
    }
    case parser::ExpressionType::CONJUNCTION_AND:
    case parser::ExpressionType::CONJUNCTION_OR: {
      const auto &conjunction = static_cast<const parser::ConjunctionExpression &>(expression);
      translator = std::make_unique<ConjunctionTranslator>(conjunction, this);
      break;
    }
    case parser::ExpressionType::OPERATOR_PLUS:
    case parser::ExpressionType::OPERATOR_MINUS:
    case parser::ExpressionType::OPERATOR_MULTIPLY:
    case parser::ExpressionType::OPERATOR_DIVIDE:
    case parser::ExpressionType::OPERATOR_MOD: {
      const auto &operator_expr = static_cast<const parser::OperatorExpression &>(expression);
      translator = std::make_unique<ArithmeticTranslator>(operator_expr, this);
      break;
    }
    case parser::ExpressionType::OPERATOR_NOT:
    case parser::ExpressionType::OPERATOR_UNARY_MINUS: {
      const auto &operator_expr = static_cast<const parser::OperatorExpression &>(expression);
      translator = std::make_unique<UnaryTranslator>(operator_expr, this);
      break;
    }
    case parser::ExpressionType::OPERATOR_IS_NULL:
    case parser::ExpressionType::OPERATOR_IS_NOT_NULL: {
      const auto &operator_expr = static_cast<const parser::OperatorExpression &>(expression);
      translator = std::make_unique<NullCheckTranslator>(operator_expr, this);
      break;
    }
    case parser::ExpressionType::VALUE_CONSTANT: {
      const auto &constant = static_cast<const parser::ConstantValueExpression &>(expression);
      translator = std::make_unique<ConstantTranslator>(constant, this);
      break;
    }
    case parser::ExpressionType::VALUE_TUPLE: {
      const auto &derived_value = static_cast<const parser::DerivedValueExpression &>(expression);
      translator = std::make_unique<DerivedValueTranslator>(derived_value, this);
      break;
    }
    // TODO(WAN): ??
#if 0
    case parser::ExpressionType::BUILTIN_FUNCTION: {
      const auto &builtin_func = static_cast<const parser::BuiltinFunctionExpression &>(expression);
      translator = std::make_unique<BuiltinFunctionTranslator>(builtin_func, this);
      break;
    }
#endif
    default: {
      throw NOT_IMPLEMENTED_EXCEPTION(
          fmt::format("Code generation for expression type '{}' not supported.",
                      parser::ExpressionTypeToString(expression.GetExpressionType(), false)));
    }
  }

  expressions_[&expression] = std::move(translator);
}

OperatorTranslator *CompilationContext::LookupTranslator(const planner::AbstractPlanNode &node) const {
  if (auto iter = ops_.find(&node); iter != ops_.end()) {
    return iter->second.get();
  }
  return nullptr;
}

ExpressionTranslator *CompilationContext::LookupTranslator(const parser::AbstractExpression &expr) const {
  if (auto iter = expressions_.find(&expr); iter != expressions_.end()) {
    return iter->second.get();
  }
  return nullptr;
}

std::string CompilationContext::GetFunctionPrefix() const { return "Query" + std::to_string(unique_id_); }

util::RegionVector<ast::FieldDecl *> CompilationContext::QueryParams() const {
  ast::Expr *state_type = codegen_.PointerType(codegen_.MakeExpr(query_state_type_));
  ast::FieldDecl *field = codegen_.MakeField(query_state_var_, state_type);
  return codegen_.MakeFieldList({field});
}

ast::Expr *CompilationContext::GetExecutionContextPtrFromQueryState() { return exec_ctx_.Get(&codegen_); }

}  // namespace terrier::execution::codegen
