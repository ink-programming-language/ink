#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeYieldStmt(AnalysisState &, const parser::YieldStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
