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
    const auto Site = State.at(Node);
    auto &Builder = State.irBuilder();
    const ir::Function *Function = Builder.insertFunction();
    RETURN_REPORT_IF_FALSE(Site, Function, core::DiagnosticKind::SemanticReturnOutsideFunction, false);
    const ir::Type *ReturnType = Function->functionType()->returnType();
    const bool ReturnsVoid = ir::VoidType::classof(ReturnType);
    if (!Node.value())
    {
      RETURN_REPORT_IF_FALSE(Site, ReturnsVoid, core::DiagnosticKind::SemanticMissingReturn, false, core::NamePool::instance().text(Function->name()));
      CHECK_ICE(Site, Builder.createReturn(), core::DiagnosticKind::SemanticConstructionFailed);
      return true;
    }
    const auto ValueSite = State.at(*Node.value());
    RETURN_REPORT_IF(ValueSite, ReturnsVoid, core::DiagnosticKind::SemanticTypeMismatch, false, "no return value", "a return value");
    const ir::Value *Value = requireRuntimeValue(ValueSite, emitValue(State, *Node.value(), ExprHints{ReturnType}));
    RETURN_IF(!Value, false);
    CHECK_ICE(Site, Builder.createReturn(*Value), core::DiagnosticKind::SemanticConstructionFailed);
    return true;
  }
} // namespace ink::semantic
