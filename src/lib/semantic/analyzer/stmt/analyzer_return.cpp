#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeReturnStmt(AnalysisState &, const parser::ReturnStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
