#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node)
  {
    if (!State.Evaluating && !Node.isComptime())
    {
      return reportUnsupported(State, Node);
    }
    ++State.LoopDepth;
    bool Succeeded = true;
    while (Succeeded)
    {
      ExpressionResult Condition;
      {
        AnalysisState::EvaluationGuard Evaluate(State);
        Condition = analyzeExpr(State, *Node.condition());
      }
      if (!Condition || !Condition.ValueObject || !ir::BoolConstant::classof(Condition.ValueObject))
      {
        if (Condition)
        {
          reportExecution(State, execution::ExecutionStatus::TypeMismatch, Node);
        }
        Succeeded = false;
        break;
      }
      if (!static_cast<const ir::BoolConstant &>(*Condition.ValueObject).value())
      {
        break;
      }
      // A fresh expansion frame makes each iteration a new semantic event.
      NameResolver::ScopeGuard Scope(State.Resolver);
      AnalysisState::FrameGuard Iteration(State, execution::ExecutionFrameKind::Block);
      if (!Iteration)
      {
        Succeeded = reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
        break;
      }
      Succeeded = analyzeStmt(State, *Node.body());
      State.Continuing = false;
      if (State.Terminated || State.Breaking)
      {
        break;
      }
    }
    State.Breaking = false;
    --State.LoopDepth;
    return Succeeded;
  }
} // namespace ink::semantic
