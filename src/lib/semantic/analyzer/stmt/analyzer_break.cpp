#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node)
  {
    if (!State.Loops.empty() && State.Loops.back().BreakTarget && !State.Evaluating)
    {
      const auto Target = State.Loops.back().BreakTarget;
      const auto Begin = State.Loops.back().LifetimeBegin;
      if (!cleanupObjects(State, Begin, false, Node) || !State.Builder.createBranchInstruction(*Target))
      {
        return false;
      }
      State.Loops.back().Breaks.push_back(State.initializationState());
      State.Terminated = true;
      return true;
    }
    if (State.LoopDepth && !State.Loops.empty() && !State.Loops.back().BreakTarget)
    {
      State.Breaking = true;
      return true;
    }
    State.report<core::DiagnosticKind::SemanticInvalidLoopControl>(Node.getSourceRange(), "break");
    return false;
  }
} // namespace ink::semantic
