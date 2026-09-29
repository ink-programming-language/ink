#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
