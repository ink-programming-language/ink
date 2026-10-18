#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/return_stmt.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  bool Analyzer::analyzeReturnStmt(AnalyzerState &State, const frontend::parser::ReturnStmt &Node)
  {
    auto &Context = State.semanticContext();
    auto &Builder = State.irBuilder();
    const ir::Function *Function = Builder.insertFunction();
    RETURN_REPORT_IF_FALSE(Context, Function, core::DiagnosticKind::SemanticReturnOutsideFunction, false, Node.getSourceRange());
    const ir::Type *ReturnType = Function->functionType()->returnType();
    const bool ReturnsVoid = ReturnType == Context.typePool().getVoidType();
    if (!Node.value())
    {
      RETURN_REPORT_IF_FALSE(Context, ReturnsVoid, core::DiagnosticKind::SemanticMissingReturn, false, Node.getSourceRange(), core::NamePool::instance().text(Function->name()));
      RETURN_REPORT_IF_FALSE(Context, Builder.createReturn(), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
      return true;
    }
    RETURN_REPORT_IF(Context, ReturnsVoid, core::DiagnosticKind::SemanticTypeMismatch, false, Node.value()->getSourceRange(), "no return value", "a return value");
    const ExprResult Value = toRValue(State, Node.value()->getSourceRange(), analyzeExpr(State, *Node.value(), ExprHints{ReturnType}));
    RETURN_IF(!Value, false);
    RETURN_REPORT_IF_FALSE(Context, Builder.createReturn(*Value.Value), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    return true;
  }
} // namespace ink::semantic
