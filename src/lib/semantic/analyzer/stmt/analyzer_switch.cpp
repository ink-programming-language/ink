#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeSwitchStmt(AnalysisState &State, const parser::SwitchStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
