#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeSwitchStmt(AnalysisState &State, const parser::SwitchStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
