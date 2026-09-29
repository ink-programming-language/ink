#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDirectImportStmt(AnalysisState &State, const parser::DirectImportStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeFromImportStmt(AnalysisState &State, const parser::FromImportStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
