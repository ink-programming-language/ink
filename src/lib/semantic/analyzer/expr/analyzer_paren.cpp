#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/paren_expr.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeParenExpr(AnalyzerState &State, const frontend::parser::ParenExpr &Node, ExprHints Hints)
  {
    return analyzeExpr(State, *Node.expression(), Hints);
  }
} // namespace ink::semantic
