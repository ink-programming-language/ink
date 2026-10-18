#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/unary_expr.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node, ExprHints Hints)
  {
    using namespace frontend::parser;
    UNSUPPORT_IF(State, Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp && Node.op() != TokenKind::Minus, ExprResult{}, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    auto &Context = State.semanticContext();
    if (Node.op() == TokenKind::Minus)
    {
      // TODO: Preserve untyped integer values through negation before materialization; signed-minimum literals such as -128 for i8 currently fail while typing their positive operand.
      Hints.IsCallCallee = false;
      ExprResult Operand = analyzeExpr(State, *Node.operand(), Hints);
      RETURN_IF(!Operand, ExprResult{});
      UNSUPPORT_IF(State, ir::GlobalVariable::classof(Operand.Value) || (Operand.Kind != ExprResultKind::RValue && Operand.Kind != ExprResultKind::LValue) || (!ir::IntegerType::classof(Operand.Type) && !ir::FloatType::classof(Operand.Type)), ExprResult{}, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
      Operand = toRValue(State, Node.operand()->getSourceRange(), Operand);
      RETURN_IF(!Operand, ExprResult{});
      const auto *Result = State.irBuilder().createNeg(*Operand.Value);
      RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticConstructionFailed, ExprResult{}, Node.getSourceRange());
      return ExprResult::fromValue(Result);
    }

    const ExprResult Operand = analyzeExpr(State, *Node.operand());
    RETURN_IF(!Operand, ExprResult{});
    if (Operand.Kind == ExprResultKind::Type)
    {
      // analyzeExpr has already validated this type's membership in the current IR context.
      const auto *Type = static_cast<const ir::Type *>(Operand.Value);
      RETURN_REPORT_IF(Context, ir::MetaType::classof(Type), core::DiagnosticKind::SemanticTypeMismatch, ExprResult{}, Node.operand()->getSourceRange(), "a type with runtime storage", Type->toString());
      auto &Types = Context.typePool();
      return ExprResult::fromValue(Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.getPointerType(Type)) : Types.getReferenceType(Type));
    }
    RETURN_REPORT_IF(Context, Operand.Kind == ExprResultKind::Module, core::DiagnosticKind::SemanticTypeMismatch, ExprResult{}, Node.operand()->getSourceRange(), "a type or runtime operand", ir::valueKindName(Operand.Value->kind()));
    // TODO: Emit runtime dereference and address operations when their IR is supported.
    reportUnsupported(State, Node);
    return {};
  }
} // namespace ink::semantic
