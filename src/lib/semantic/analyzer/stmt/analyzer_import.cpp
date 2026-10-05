#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDirectImportStmt(AnalysisState &, const parser::DirectImportStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }

  bool Analyzer::analyzeFromImportStmt(AnalysisState &, const parser::FromImportStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
