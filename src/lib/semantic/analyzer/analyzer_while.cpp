#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
