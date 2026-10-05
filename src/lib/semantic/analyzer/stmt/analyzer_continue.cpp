#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeContinueStmt(AnalysisState &State, const parser::ContinueStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
