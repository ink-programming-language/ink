#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeComptimeStmt(AnalysisState &State, const parser::ComptimeStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
