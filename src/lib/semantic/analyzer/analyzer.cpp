#include "analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeMissingStmt(AnalysisState &, const parser::MissingStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }

  bool Analyzer::analyzeErrorStmt(AnalysisState &, const parser::ErrorStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }

  bool Analyzer::analyzeMissingDecl(AnalysisState &, const parser::MissingDecl &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }

  bool Analyzer::analyzeErrorDecl(AnalysisState &, const parser::ErrorDecl &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
