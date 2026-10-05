#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node)
  {
    if (!State.Evaluating && !Node.isComptime())
    {
      if (!State.CurrentFunction)
      {
        return reportUnsupported(State, Node);
      }
      NameResolver::ScopeGuard Scope(State.Resolver);
      AnalysisState::FrameGuard Frame(State, execution::ExecutionFrameKind::Block);
      if (!Frame)
      {
        return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
      }
      const std::size_t Begin = State.Lifetimes.size();
      if (Node.initializer() && !analyzeStmt(State, *Node.initializer()))
      {
        return finishObjectScope(State, Begin, Node, false);
      }
      const auto Condition = [&]() -> const ir::Value *
      {
        return Node.condition() ? analyzeLoopCondition(State, *Node.condition()) : &State.Context.constantPool().getBoolConstant(true);
      };
      const auto Body = [&]()
      {
        return analyzeStmt(State, *Node.body());
      };
      const auto Step = [&]()
      {
        for (const parser::SimpleItem *Item : Node.step())
        {
          if (!analyzeSimpleItem(State, *Item) || !cleanupObjects(State, 0, true, *Item))
          {
            return false;
          }
        }
        return true;
      };
      const bool Succeeded = analyzeRuntimeLoop(State, Node, Condition, Body, Step);
      return finishObjectScope(State, Begin, Node, Succeeded);
    }
    NameResolver::ScopeGuard Scope(State.Resolver);
    const std::size_t LifetimeBegin = State.Lifetimes.size();
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
    AnalysisState::LoopGuard Loop(State);
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
        if (!cleanupObjects(State, 0, true, Node))
        {
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
        const std::size_t Begin = State.Lifetimes.size();
        Succeeded = analyzeStmt(State, *Node.body());
        Succeeded = finishObjectScope(State, Begin, Node, Succeeded);
      }
      State.Continuing = false;
      if (!Succeeded || State.Terminated || State.Breaking)
      {
        break;
      }
      AnalysisState::EvaluationGuard Evaluate(State);
      for (const parser::SimpleItem *Step : Node.step())
      {
        if (!analyzeSimpleItem(State, *Step) || !cleanupObjects(State, 0, true, *Step))
        {
          Succeeded = false;
          break;
        }
      }
    }
    State.Breaking = false;
    return finishObjectScope(State, LifetimeBegin, Node, Succeeded);
  }
} // namespace ink::semantic
