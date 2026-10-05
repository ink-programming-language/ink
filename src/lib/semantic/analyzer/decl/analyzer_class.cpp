#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
