#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
