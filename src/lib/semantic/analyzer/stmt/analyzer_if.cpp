#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeIfStmt(AnalysisState &State, const parser::IfStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
