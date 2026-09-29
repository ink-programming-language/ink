#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
