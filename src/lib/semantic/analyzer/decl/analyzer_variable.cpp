#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeVarDecl(AnalysisState &, const parser::VarDecl &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
