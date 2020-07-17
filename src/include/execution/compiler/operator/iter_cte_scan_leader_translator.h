#pragma once

#include <utility>
#include <vector>
#include "execution/compiler/operator/operator_translator.h"
#include "execution/compiler/translator_factory.h"
#include "planner/plannodes/cte_scan_plan_node.h"

namespace terrier::execution::compiler {

/**
 * CteScanLeader Translator
 */
class IterCteScanLeaderTranslator : public OperatorTranslator {
 public:
  /**
   * Constructor
   * @param op The plan node
   * @param codegen The code generator
   */
  IterCteScanLeaderTranslator(const terrier::planner::CteScanPlanNode *op, CodeGen *codegen,
                              OperatorTranslator *base_case, int index);

  // Pass through
  void Produce(FunctionBuilder *builder) override;

  // Pass through
  void Abort(FunctionBuilder *builder) override { child_translator_->Abort(builder); }

  // Pass through
  void Consume(FunctionBuilder *builder) override;

  // Does nothing
  void InitializeStateFields(util::RegionVector<ast::FieldDecl *> *state_fields) override {
    if(base_translator_ != nullptr){
      return;
    }
    ast::Expr *cte_scan_type = codegen_->BuiltinType(ast::BuiltinType::Kind::CteScanIterator);
    ast::Expr *cte_scan_ptr_type = codegen_->PointerType(cte_scan_type);
    state_fields->emplace_back(codegen_->MakeField(codegen_->GetIdentifier(op_->GetCTETableName()),
                                                   cte_scan_ptr_type));

    ast::Expr *iter_cte_scan_type = codegen_->BuiltinType(ast::BuiltinType::Kind::IterCteScanIterator);
    state_fields->emplace_back(codegen_->MakeField(iter_cte_scan_, iter_cte_scan_type));
  }

  // Does nothing
  void InitializeStructs(util::RegionVector<ast::Decl *> *decls) override {}

  // Does nothing
  void InitializeHelperFunctions(util::RegionVector<ast::Decl *> *decls) override {}

  // Does nothing
  void InitializeSetup(util::RegionVector<ast::Stmt *> *setup_stmts) override {}

  // Does nothing
  void InitializeTeardown(util::RegionVector<ast::Stmt *> *teardown_stmts) override {
    if(base_translator_ != nullptr){
      return;
    }
    ast::Expr *cte_free_call =
        codegen_->OneArgCall(ast::Builtin::IterCteScanFree, codegen_->GetStateMemberPtr(iter_cte_scan_));
    teardown_stmts->emplace_back(codegen_->MakeStmt(cte_free_call));
  }

  ast::Expr *GetOutput(uint32_t attr_idx) override {
    // should never reach here
    TERRIER_ASSERT(false, "We should never reach here");
    return nullptr;
  }

  ast::Expr *GetChildOutput(uint32_t child_idx, uint32_t attr_idx, terrier::type::TypeId type) override {
    return child_translator_->GetOutput(attr_idx);
  }

  // Is always vectorizable.
  bool IsVectorizable() override { return true; }

  // Should not be called here
  ast::Expr *GetTableColumn(const catalog::col_oid_t &col_oid) override {
    UNREACHABLE("Projection nodes should not use column value expressions");
  }

  const planner::AbstractPlanNode *Op() override { return op_; }

 private:
  const planner::CteScanPlanNode *op_;

  ast::Expr *GetReadCteScanIterator();
  void PopulateReadCteScanIterator(FunctionBuilder *builder);
  void FinalizeReadCteScanIterator(FunctionBuilder *builder);
  // Declare Cte Scan Itarator
  void DeclareIterCteScanIterator(FunctionBuilder *builder);
  // Set Column Types for insertion
  void SetColumnTypes(FunctionBuilder *builder);
  // Declare the insert PR
  void DeclareInsertPR(FunctionBuilder *builder);
  // Get the pr to insert
  void GetInsertPR(FunctionBuilder *builder);
  // Fill the insert PR from the child's output
  void FillPRFromChild(FunctionBuilder *builder);
  // Insert into table.
  void GenTableInsert(FunctionBuilder *builder);
  ast::Identifier iter_cte_scan_;
  ast::Identifier col_types_;
  std::vector<int> all_types_;
  ast::Identifier insert_pr_;
  std::vector<catalog::col_oid_t> col_oids_;
  storage::ProjectionMap projection_map_;
  ast::Identifier read_col_oids_;
  ast::Identifier read_tvi_;
  ast::Identifier read_pci_;
  ast::Identifier accumulate_checker_;

//  OperatorTranslator *base_case_translator_{nullptr};
//  OperatorTranslator *inductive_case_translator_{nullptr};
  void SetReadOids(FunctionBuilder *builder);
  void DeclareReadTVI(FunctionBuilder *builder);
  void GenReadTVIClose(FunctionBuilder *builder);
  void DoTableScan(FunctionBuilder *builder);

  // for (@tableIterInit(&tvi, ...); @tableIterAdvance(&tvi);) {...}
  void GenTVILoop(FunctionBuilder *builder);

  void DeclarePCI(FunctionBuilder *builder);
  void DeclareSlot(FunctionBuilder *builder);

  // var pci = @tableIterGetPCI(&tvi)
  // for (; @pciHasNext(pci); @pciAdvance(pci)) {...}
  void GenPCILoop(FunctionBuilder *builder);
  void DeclareAccumulateChecker(FunctionBuilder *builder);
  void GenInductiveLoop(FunctionBuilder *builder);
  int current_index_;
  parser::ConstantValueExpression DummyLeaderCVE();
  ast::Expr *GetIterCteScanIterator();
  IterCteScanLeaderTranslator *base_translator_{nullptr};
};

}  // namespace terrier::execution::compiler
