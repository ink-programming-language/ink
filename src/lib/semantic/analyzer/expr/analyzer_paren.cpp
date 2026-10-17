#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/paren_expr.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeParenExpr(AnalyzerState &State, const frontend::parser::ParenExpr &Node, ExprHints Hints)
  {
    if (!Node.expression())
    {
      State.semanticContext().report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "ParenExpr requires an expression");
      return nullptr;
    }
    return analyzeExpr(State, *Node.expression(), Hints);
  }
} // namespace ink::semantic
