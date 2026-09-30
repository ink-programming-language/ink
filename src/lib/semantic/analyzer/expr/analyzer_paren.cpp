#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  Analyzer::ExpressionResult Analyzer::analyzeParenExpr(AnalysisState &State, const parser::ParenExpr &Node, std::size_t Depth)
  {
    return analyzeExpr(State, *Node.expression(), Depth + 1);
  }
} // namespace ink::semantic
