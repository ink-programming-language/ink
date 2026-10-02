#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <vector>

namespace ink::semantic
{
  bool Analyzer::analyzeIfStmt(AnalysisState &State, const parser::IfStmt &Node)
  {
    if (State.Evaluating || Node.isComptime())
    {
      const ExpressionResult Condition = State.Evaluating ? analyzeExpr(State, *Node.condition()) : evaluateComptime(State, *Node.condition());
      if (!Condition)
      {
        return false;
      }
      if (!Condition.ValueObject || !ir::BoolConstant::classof(Condition.ValueObject))
      {
        return reportExecution(State, execution::ExecutionStatus::TypeMismatch, Node);
      }
      const parser::Stmt *Selected = static_cast<const ir::BoolConstant &>(*Condition.ValueObject).value() ? Node.thenBranch() : Node.elseBranch();
      return !Selected || analyzeStmt(State, *Selected);
    }
    if (!State.CurrentFunction)
    {
      return reportUnsupported(State, Node);
    }
    const ExpressionResult Condition = analyzeExpr(State, *Node.condition());
    if (!Condition)
    {
      return false;
    }
    if (!Condition.ValueObject || Condition.ValueObject->type().typeKind() != ir::TypeKind::Bool)
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.condition()->getSourceRange(), "bool", Condition.IntegerLiteral ? "integer literal" : (Condition.ValueObject ? describeType(Condition.ValueObject->type()) : "void"));
      return false;
    }

    ir::BasicBlock *ThenBlock = State.Builder.createBasicBlock(*State.CurrentFunction);
    ir::BasicBlock *ElseBlock = State.Builder.createBasicBlock(*State.CurrentFunction);
    if (!ThenBlock || !ElseBlock || !State.Builder.createConditionalBranchInstruction(*Condition.ValueObject, *ThenBlock, *ElseBlock))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }

    struct Initialization
    {
        ComptimeState::Variable *Variable;
        bool Incoming;
        bool Then;
    };
    std::vector<Initialization> Initializations;
    for (auto &Entry : State.Context.comptimeState().Variables)
    {
      auto &Variable = Entry.second;
      if (!Variable.Comptime && Variable.Function == State.CurrentFunction)
      {
        Initializations.push_back({&Variable, Variable.Initialized, Variable.Initialized});
      }
    }

    // A bare declaration is scoped to its branch just like a block's contents.
    // Compile-time loops outside a runtime condition cannot be broken conditionally.
    auto AnalyzeBranch = [&](const parser::Stmt &Branch, ir::BasicBlock &Block)
    {
      State.Terminated = false;
      if (!State.Builder.setInsertPoint(Block))
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
      NameResolver::ScopeGuard Scope(State.Resolver);
      AnalysisState::FrameGuard Frame(State, execution::ExecutionFrameKind::Block);
      if (!Frame)
      {
        return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Branch);
      }
      const std::size_t SavedLoopDepth = State.LoopDepth;
      State.LoopDepth = 0;
      const bool Succeeded = analyzeStmt(State, Branch);
      State.LoopDepth = SavedLoopDepth;
      return Succeeded;
    };

    bool Succeeded = AnalyzeBranch(*Node.thenBranch(), *ThenBlock);
    const bool ThenTerminated = State.Terminated;
    ir::BasicBlock *ThenEnd = State.Builder.insertBlock();
    for (auto &Entry : Initializations)
    {
      Entry.Then = Entry.Variable->Initialized;
      Entry.Variable->Initialized = Entry.Incoming;
    }

    State.Terminated = false;
    bool ElseTerminated = false;
    ir::BasicBlock *ElseEnd = ElseBlock;
    if (Node.elseBranch())
    {
      const bool ElseSucceeded = AnalyzeBranch(*Node.elseBranch(), *ElseBlock);
      Succeeded = Succeeded && ElseSucceeded;
      ElseTerminated = State.Terminated;
      ElseEnd = State.Builder.insertBlock();
    }
    for (auto &Entry : Initializations)
    {
      // Only paths reaching the following statement contribute to definite initialization.
      if (!Succeeded || (ThenTerminated && ElseTerminated))
      {
        Entry.Variable->Initialized = Entry.Incoming;
      }
      else if (ElseTerminated)
      {
        Entry.Variable->Initialized = Entry.Then;
      }
      else if (!ThenTerminated)
      {
        Entry.Variable->Initialized = Entry.Then && Entry.Variable->Initialized;
      }
    }

    State.Terminated = ThenTerminated && ElseTerminated;
    if (State.Terminated)
    {
      State.Builder.clearInsertPoint();
      return Succeeded;
    }
    ir::BasicBlock *MergeBlock = Node.elseBranch() ? State.Builder.createBasicBlock(*State.CurrentFunction) : ElseBlock;
    if (!MergeBlock || (!ThenTerminated && (!ThenEnd || !State.Builder.setInsertPoint(*ThenEnd) || !State.Builder.createBranchInstruction(*MergeBlock))) || (Node.elseBranch() && !ElseTerminated && (!ElseEnd || !State.Builder.setInsertPoint(*ElseEnd) || !State.Builder.createBranchInstruction(*MergeBlock))) || !State.Builder.setInsertPoint(*MergeBlock))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    return Succeeded;
  }
} // namespace ink::semantic
