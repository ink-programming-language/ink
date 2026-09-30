#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeForInStmt(AnalysisState &State, const parser::ForInStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
