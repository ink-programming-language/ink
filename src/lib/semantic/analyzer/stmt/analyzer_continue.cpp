#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeContinueStmt(AnalysisState &State, const parser::ContinueStmt &Node)
  {
    if (State.LoopDepth)
    {
      State.Continuing = true;
      return true;
    }
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
