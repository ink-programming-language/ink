#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeYieldStmt(AnalysisState &State, const parser::YieldStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
