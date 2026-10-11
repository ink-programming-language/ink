#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDeferStmt(AnalysisState &State, const frontend::parser::DeferStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
