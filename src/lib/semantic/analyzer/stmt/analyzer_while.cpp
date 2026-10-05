#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
