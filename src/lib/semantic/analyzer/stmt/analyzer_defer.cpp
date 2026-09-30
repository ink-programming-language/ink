#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDeferStmt(AnalysisState &State, const parser::DeferStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
