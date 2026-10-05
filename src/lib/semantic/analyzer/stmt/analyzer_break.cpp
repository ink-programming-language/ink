#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeBreakStmt(AnalysisState &, const parser::BreakStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
