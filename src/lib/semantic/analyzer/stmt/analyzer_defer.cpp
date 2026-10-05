#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDeferStmt(AnalysisState &, const parser::DeferStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
