#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/paren_expr.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::analyzeParenExpr(AnalyzerState &State, const frontend::parser::ParenExpr &Node, ExprHints Hints)
  {
    CHECK_ICE(State.at(Node), Node.expression(), core::DiagnosticKind::SemanticInvalidAST, "ParenExpr requires an expression");
    return analyzeExpr(State, *Node.expression(), Hints);
  }
} // namespace ink::semantic
