#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeContinueStmt(AnalysisState &State, const parser::ContinueStmt &Node)
  {
    if (!State.Loops.empty() && State.Loops.back().ContinueTarget && !State.Evaluating)
    {
      const auto Target = State.Loops.back().ContinueTarget;
      const auto Begin = State.Loops.back().LifetimeBegin;
      if (!cleanupObjects(State, Begin, false, Node) || !State.Builder.createBranchInstruction(*Target))
      {
        return false;
      }
      State.Loops.back().Continues.push_back(State.initializationState());
      State.Terminated = true;
      return true;
    }
    if (State.LoopDepth && !State.Loops.empty() && !State.Loops.back().BreakTarget)
    {
      State.Continuing = true;
      return true;
    }
    State.report<core::DiagnosticKind::SemanticInvalidLoopControl>(Node.getSourceRange(), "continue");
    return false;
  }
} // namespace ink::semantic
