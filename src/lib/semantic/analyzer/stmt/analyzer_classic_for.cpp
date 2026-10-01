#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node)
  {
    if (!State.Evaluating && !Node.isComptime())
    {
      return reportUnsupported(State, Node);
    }
    NameResolver::ScopeGuard Scope(State.Resolver);
    AnalysisState::FrameGuard Frame(State, execution::ExecutionFrameKind::Block);
    if (!Frame)
    {
      return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
    }
    if (Node.initializer())
    {
      AnalysisState::EvaluationGuard Evaluate(State);
      if (!analyzeStmt(State, *Node.initializer()))
      {
        return false;
      }
    }
    ++State.LoopDepth;
    bool Succeeded = true;
    while (Succeeded)
    {
      if (!reportExecution(State, State.Context.comptimeState().Engine.consumeStep(), Node))
      {
        Succeeded = false;
        break;
      }
      if (Node.condition())
      {
        AnalysisState::EvaluationGuard Evaluate(State);
        const ExpressionResult Condition = analyzeExpr(State, *Node.condition());
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
      }
      {
        NameResolver::ScopeGuard IterationScope(State.Resolver);
        AnalysisState::FrameGuard Iteration(State, execution::ExecutionFrameKind::Block);
        if (!Iteration)
        {
          Succeeded = reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
          break;
        }
        Succeeded = analyzeStmt(State, *Node.body());
      }
      State.Continuing = false;
      if (!Succeeded || State.Terminated || State.Breaking)
      {
        break;
      }
      AnalysisState::EvaluationGuard Evaluate(State);
      for (const parser::SimpleItem *Step : Node.step())
      {
        if (!analyzeSimpleItem(State, *Step))
        {
          Succeeded = false;
          break;
        }
      }
    }
    State.Breaking = false;
    --State.LoopDepth;
    return Succeeded;
  }
} // namespace ink::semantic
