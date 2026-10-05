#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
