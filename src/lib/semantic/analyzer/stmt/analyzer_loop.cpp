#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeLoopCondition(AnalysisState &State, const parser::Expr &Node)
  {
    AnalysisState::EvaluationGuard Expected(State, State.Evaluating, nullptr);
    const ExpressionResult Result = analyzeExpr(State, Node);
    if (!Result)
    {
      return nullptr;
    }
    if (!Result.ValueObject || Result.ValueObject->type().typeKind() != ir::TypeKind::Bool)
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "bool", Result.IntegerLiteral ? "integer literal" : (Result.ValueObject ? describeType(Result.ValueObject->type()) : "void"));
      return nullptr;
    }
    return cleanupObjects(State, 0, true, Node) ? Result.ValueObject : nullptr;
  }

  bool Analyzer::analyzeRuntimeLoop(AnalysisState &State, const parser::Stmt &Node, const std::function<const ir::Value *()> &Condition, const std::function<bool()> &Body, const std::function<bool()> &Step)
  {
    auto *Header = State.Builder.createBasicBlock(*State.CurrentFunction);
    auto *BodyBlock = State.Builder.createBasicBlock(*State.CurrentFunction);
    auto *StepBlock = State.Builder.createBasicBlock(*State.CurrentFunction);
    auto *Exit = State.Builder.createBasicBlock(*State.CurrentFunction);
    if (!Header || !BodyBlock || !StepBlock || !Exit || !State.Builder.createBranchInstruction(*Header) || !State.Builder.setInsertPoint(*Header))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    const ir::Value *Test = Condition();
    if (!Test)
    {
      return false;
    }
    const bool Always = ir::BoolConstant::classof(Test) && static_cast<const ir::BoolConstant &>(*Test).value();
    if (!(Always ? State.Builder.createBranchInstruction(*BodyBlock) != nullptr : State.Builder.createConditionalBranchInstruction(*Test, *BodyBlock, *Exit) != nullptr) || !State.Builder.setInsertPoint(*BodyBlock))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    const auto Incoming = State.initializationState();
    AnalysisState::LoopGuard Loop(State, Exit, StepBlock);
    const std::size_t LoopIndex = State.Loops.size() - 1;
    if (!Always)
    {
      State.Loops[LoopIndex].Breaks.push_back(Incoming);
    }
    bool Succeeded;
    {
      NameResolver::ScopeGuard Scope(State.Resolver);
      AnalysisState::FrameGuard Frame(State, execution::ExecutionFrameKind::Block);
      const std::size_t LifetimeBegin = State.Lifetimes.size();
      Succeeded = Frame ? Body() : reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
      Succeeded = finishObjectScope(State, LifetimeBegin, Node, Succeeded);
    }
    if (!State.Terminated && Succeeded)
    {
      State.Loops[LoopIndex].Continues.push_back(State.initializationState());
      Succeeded = State.Builder.createBranchInstruction(*StepBlock) != nullptr;
    }
    State.mergeInitialization(Incoming, State.Loops[LoopIndex].Continues);
    State.Terminated = false;
    if (!Succeeded || !State.Builder.setInsertPoint(*StepBlock) || !Step() || !State.Builder.createBranchInstruction(*Header) || !State.Builder.setInsertPoint(*Exit))
    {
      State.restoreInitialization(Incoming);
      return false;
    }
    State.mergeInitialization(Incoming, State.Loops[LoopIndex].Breaks);
    if (State.Loops[LoopIndex].Breaks.empty())
    {
      // Keep the unused exit well-formed without inventing a reachable return path.
      if (!State.Builder.createBranchInstruction(*Exit))
      {
        return false;
      }
      State.Terminated = true;
      State.Builder.clearInsertPoint();
    }
    return true;
  }
} // namespace ink::semantic
