#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeInterfaceDecl(AnalysisState &State, const parser::InterfaceDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
