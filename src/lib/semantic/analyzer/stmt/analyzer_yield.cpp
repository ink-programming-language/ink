#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeYieldStmt(AnalysisState &State, const parser::YieldStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
