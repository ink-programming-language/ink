#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node)
  {
    if (State.LoopDepth)
    {
      State.Breaking = true;
      return true;
    }
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
