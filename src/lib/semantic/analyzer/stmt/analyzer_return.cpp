#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/return_stmt.h"
#include "ink/ir/module/global_variable.h"
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
    if (!Function)
    {
      Context.report<core::DiagnosticKind::SemanticReturnOutsideFunction>(Node.getSourceRange());
      return false;
    }
    const ir::Type *ReturnType = Function->functionType()->returnType();
    const bool ReturnsVoid = ReturnType == Context.typePool().getVoidType();
    if (!Node.value())
    {
      if (!ReturnsVoid)
      {
        Context.report<core::DiagnosticKind::SemanticMissingReturn>(Node.getSourceRange(), core::NamePool::instance().text(Function->name()));
        return false;
      }
      if (!Builder.createReturn())
      {
        Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
      return true;
    }
    if (ReturnsVoid)
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "no return value", "a return value");
      return false;
    }
    const ir::Value *Value = requireRuntimeValue(Context, Node.value()->getSourceRange(), analyzeExpr(State, *Node.value(), ExprHints{ReturnType}));
    if (!Value)
    {
      return false;
    }
    if (ir::GlobalVariable::classof(Value))
    {
      // TODO: Materialize variable storage reads before returning their values.
      return reportUnsupported(State, *Node.value());
    }
    if (!Builder.createReturn(*Value))
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    return true;
  }
} // namespace ink::semantic
