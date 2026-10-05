#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeReturnStmt(AnalysisState &State, const parser::ReturnStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
