#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
