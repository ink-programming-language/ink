#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
