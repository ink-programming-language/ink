#include "../analyzer_internal.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDirectImportStmt(AnalysisState &State, const frontend::parser::DirectImportStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeFromImportStmt(AnalysisState &State, const frontend::parser::FromImportStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
