#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeEnumDecl(AnalysisState &State, const parser::EnumDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
