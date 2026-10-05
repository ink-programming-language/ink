#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node)
  {
    if (!State.Evaluating && !Node.isComptime())
    {
      if (!State.CurrentFunction)
      {
        return reportUnsupported(State, Node);
      }
      const auto Condition = [&]()
      {
        return analyzeLoopCondition(State, *Node.condition());
      };
      const auto Body = [&]()
      {
        return analyzeStmt(State, *Node.body());
      };
      const auto Step = []()
      {
        return true;
      };
      return analyzeRuntimeLoop(State, Node, Condition, Body, Step);
    }
    AnalysisState::LoopGuard Loop(State);
    bool Succeeded = true;
    while (Succeeded)
    {
      ExpressionResult Condition;
      {
        AnalysisState::EvaluationGuard Evaluate(State);
        Condition = analyzeExpr(State, *Node.condition());
        if (Condition && !cleanupObjects(State, 0, true, Node))
        {
          return false;
        }
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
      // Each iteration has its own lexical scope and local storage.
      NameResolver::ScopeGuard Scope(State.Resolver);
      AnalysisState::FrameGuard Iteration(State, execution::ExecutionFrameKind::Block);
      if (!Iteration)
      {
        Succeeded = reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
        break;
      }
      const std::size_t Begin = State.Lifetimes.size();
      Succeeded = analyzeStmt(State, *Node.body());
      Succeeded = finishObjectScope(State, Begin, Node, Succeeded);
      State.Continuing = false;
      if (State.Terminated || State.Breaking)
      {
        break;
      }
    }
    State.Breaking = false;
    return Succeeded;
  }
} // namespace ink::semantic
