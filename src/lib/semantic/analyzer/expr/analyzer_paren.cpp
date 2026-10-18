#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/paren_expr.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::analyzeParenExpr(AnalyzerState &State, const frontend::parser::ParenExpr &Node, ExprHints Hints)
  {
    RETURN_REPORT_IF_FALSE(State.semanticContext(), Node.expression(), core::DiagnosticKind::SemanticInvalidAST, ExprResult{}, Node.getSourceRange(), "ParenExpr requires an expression");
    return analyzeExpr(State, *Node.expression(), Hints);
  }
} // namespace ink::semantic
