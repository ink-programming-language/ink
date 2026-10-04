#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/semantic/operator_method.h"

namespace ink::semantic
{
  using namespace ir;
  using tokenizer::TokenKind;

  Analyzer::ExpressionResult Analyzer::analyzeBinaryExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth)
  {
    if (Node.op() == TokenKind::AmpAmp || Node.op() == TokenKind::PipePipe)
    {
      return analyzeLogicalBinaryExpr(State, Node, Depth);
    }
    if (Node.op() == TokenKind::EqualEqual || Node.op() == TokenKind::BangEqual || Node.op() == TokenKind::Less || Node.op() == TokenKind::LessEqual || Node.op() == TokenKind::Greater || Node.op() == TokenKind::GreaterEqual)
    {
      return analyzeComparisonExpr(State, Node, Depth);
    }
    const ExpressionResult Left = analyzeExpr(State, *Node.left(), Depth + 1);
    if (!Left)
    {
      return {};
    }
    if (Left.ValueObject && ClassType::classof(&Left.ValueObject->type()))
    {
      const bool Deferred = State.Evaluating && findDeferredIntegerLiteral(*Node.right(), Depth + 1, State.ExpressionDepthLimit);
      if (Deferred && !prepareIntegerLiteral(State, *Node.right(), Depth + 1))
      {
        return {};
      }
      AnalysisState::EvaluationGuard OperandExpected(State, State.Evaluating && !Deferred, nullptr);
      const ExpressionResult Right = analyzeExpr(State, *Node.right(), Depth + 1);
      if (!Right)
      {
        return {};
      }
      AnalysisState::EvaluationGuard CallMode(State, State.Evaluating || Deferred, nullptr);
      return callClassOperator(State, Left, operatorMethodName(Node.op(), OperatorMethodKind::Binary), std::span<const ExpressionResult>(&Right, 1), Node);
    }
    if (!State.Evaluating && Node.op() != TokenKind::Plus)
    {
      reportUnsupported(State, Node);
      return {};
    }
    const Type *Expected = Left.ValueObject ? &Left.ValueObject->type() : (State.ExpectedType && IntegerType::classof(State.ExpectedType) ? State.ExpectedType : nullptr);
    AnalysisState::EvaluationGuard Guard(State, State.Evaluating, Expected);
    const ExpressionResult Right = analyzeExpr(State, *Node.right(), Depth + 1);
    if (!Right)
    {
      return {};
    }
    const Type *Common = Expected ? Expected : (Right.ValueObject ? &Right.ValueObject->type() : State.Context.typePool().getType<TypeKind::Integer>(32, true));
    const Value *First = convertExpression(State, Left, *Common, *Node.left());
    const Value *Second = convertExpression(State, Right, *Common, *Node.right());
    if (!First || !Second)
    {
      return {};
    }
    if (State.Evaluating)
    {
      if (!Constant::classof(First) || !Constant::classof(Second))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const auto Result = State.Context.comptimeState().Engine.evaluateBinary(Node.op(), static_cast<const Constant &>(*First), static_cast<const Constant &>(*Second));
      return reportExecution(State, Result.Status, Node) ? ExpressionResult{Result.Value} : ExpressionResult{};
    }
    const Value *Result = State.Builder.createAddInstruction(*First, *Second);
    if (!Result)
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "matching integer operands", describeType(*Common));
    }
    return {Result};
  }

  Analyzer::ExpressionResult Analyzer::analyzeLogicalBinaryExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth)
  {
    if (!State.Evaluating && !State.CurrentFunction)
    {
      reportUnsupported(State, Node);
      return {};
    }
    // A boolean result does not impose bool on arithmetic nested inside either operand.
    AnalysisState::EvaluationGuard Guard(State, State.Evaluating, nullptr);
    const ExpressionResult Left = analyzeExpr(State, *Node.left(), Depth + 1);
    if (!Left)
    {
      return {};
    }
    const Type &Boolean = State.Context.typePool().getType<TypeKind::Bool>();
    const Value *First = convertExpression(State, Left, Boolean, *Node.left());
    if (!First)
    {
      return {};
    }
    const bool IsAnd = Node.op() == TokenKind::AmpAmp;
    if (State.Evaluating)
    {
      if (!BoolConstant::classof(First))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const bool LeftValue = static_cast<const BoolConstant &>(*First).value();
      if ((IsAnd && !LeftValue) || (!IsAnd && LeftValue))
      {
        return {First};
      }
      const ExpressionResult Right = analyzeExpr(State, *Node.right(), Depth + 1);
      if (!Right)
      {
        return {};
      }
      const Value *Second = convertExpression(State, Right, Boolean, *Node.right());
      if (!Second)
      {
        return {};
      }
      if (!BoolConstant::classof(Second))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const auto Result = State.Context.comptimeState().Engine.evaluateBinary(Node.op(), static_cast<const BoolConstant &>(*First), static_cast<const BoolConstant &>(*Second));
      return reportExecution(State, Result.Status, Node) ? ExpressionResult{Result.Value} : ExpressionResult{};
    }

    AllocaInstruction *Storage = State.Builder.createAllocaInstruction(Boolean);
    BasicBlock *RightBlock = State.Builder.createBasicBlock(*State.CurrentFunction);
    BasicBlock *MergeBlock = State.Builder.createBasicBlock(*State.CurrentFunction);
    if (!Storage || !RightBlock || !MergeBlock || !State.Builder.createStoreInstruction(*Storage, *First) || !State.Builder.createConditionalBranchInstruction(*First, IsAnd ? *RightBlock : *MergeBlock, IsAnd ? *MergeBlock : *RightBlock) || !State.Builder.setInsertPoint(*RightBlock))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return {};
    }
    // Even a statically skipped runtime operand must type-check. Its instructions
    // live only on the RHS path, including any nested short-circuit blocks.
    const ExpressionResult Right = analyzeExpr(State, *Node.right(), Depth + 1);
    if (!Right)
    {
      return {};
    }
    const Value *Second = convertExpression(State, Right, Boolean, *Node.right());
    if (!Second)
    {
      return {};
    }
    if (!State.Builder.createStoreInstruction(*Storage, *Second) || !State.Builder.createBranchInstruction(*MergeBlock) || !State.Builder.setInsertPoint(*MergeBlock))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return {};
    }
    const Value *Result = State.Builder.createLoadInstruction(*Storage);
    if (!Result)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
    }
    return {Result};
  }

  Analyzer::ExpressionResult Analyzer::analyzeComparisonExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth)
  {
    // Comparisons produce bool, but their operand type comes from the operands.
    AnalysisState::EvaluationGuard Guard(State, State.Evaluating, nullptr);
    auto AnalyzeOperand = [&](const parser::Expr &Operand)
    {
      if (const auto *Literal = State.Evaluating ? findDeferredIntegerLiteral(Operand, Depth + 1, State.ExpressionDepthLimit) : nullptr)
      {
        // Type selection may wait for the other operand, but traversal and its
        // failure checks happen here, before any later operand can have effects.
        return prepareIntegerLiteral(State, Operand, Depth + 1) ? ExpressionResult{nullptr, Literal} : ExpressionResult{};
      }
      return analyzeExpr(State, Operand, Depth + 1);
    };
    ExpressionResult Left = AnalyzeOperand(*Node.left());
    if (!Left)
    {
      return {};
    }
    if (Left.ValueObject && ClassType::classof(&Left.ValueObject->type()))
    {
      const ExpressionResult Right = AnalyzeOperand(*Node.right());
      if (!Right)
      {
        return {};
      }
      const ExpressionResult Result = callClassOperator(State, Left, operatorMethodName(Node.op(), OperatorMethodKind::Binary), std::span<const ExpressionResult>(&Right, 1), Node);
      if (Result && (!Result.ValueObject || Result.ValueObject->type().typeKind() != TypeKind::Bool))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "bool comparison result", Result.ValueObject ? describeType(Result.ValueObject->type()) : "void");
        return {};
      }
      return Result;
    }
    const Type *Expected = Left.ValueObject ? &Left.ValueObject->type() : nullptr;
    AnalysisState::EvaluationGuard ExpectedGuard(State, State.Evaluating, Expected);
    ExpressionResult Right = AnalyzeOperand(*Node.right());
    if (!Right)
    {
      return {};
    }
    const Type *Common = Expected ? Expected : (Right.ValueObject ? &Right.ValueObject->type() : State.Context.typePool().getType<TypeKind::Integer>(32, true));
    const bool Equality = Node.op() == TokenKind::EqualEqual || Node.op() == TokenKind::BangEqual;
    if (Common->typeKind() != TypeKind::Integer && !(Equality && Common->typeKind() == TypeKind::Bool))
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), Equality ? "matching integer or bool operands" : "matching integer operands", describeType(*Common));
      return {};
    }
    if (State.Evaluating)
    {
      // Traversal was already charged at each operand's source position. Materialize
      // its target-width value without replaying or charging the operand again.
      if (Left.IntegerLiteral)
      {
        Left = materializeIntegerLiteral(State, *Node.left(), *Common);
        if (!Left)
        {
          return {};
        }
      }
      if (Right.IntegerLiteral)
      {
        Right = materializeIntegerLiteral(State, *Node.right(), *Common);
        if (!Right)
        {
          return {};
        }
      }
    }
    const Value *First = convertExpression(State, Left, *Common, *Node.left());
    const Value *Second = convertExpression(State, Right, *Common, *Node.right());
    if (!First || !Second)
    {
      return {};
    }
    if (State.Evaluating)
    {
      if (!Constant::classof(First) || !Constant::classof(Second))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const auto Result = State.Context.comptimeState().Engine.evaluateBinary(Node.op(), static_cast<const Constant &>(*First), static_cast<const Constant &>(*Second));
      return reportExecution(State, Result.Status, Node) ? ExpressionResult{Result.Value} : ExpressionResult{};
    }
    core::ComparisonPredicate Predicate = core::ComparisonPredicate::Equal;
    switch (Node.op())
    {
    case TokenKind::BangEqual: Predicate = core::ComparisonPredicate::NotEqual; break;
    case TokenKind::Less: Predicate = core::ComparisonPredicate::Less; break;
    case TokenKind::LessEqual: Predicate = core::ComparisonPredicate::LessEqual; break;
    case TokenKind::Greater: Predicate = core::ComparisonPredicate::Greater; break;
    case TokenKind::GreaterEqual: Predicate = core::ComparisonPredicate::GreaterEqual; break;
    default: break;
    }
    const Value *Result = State.Builder.createCompareInstruction(Predicate, *First, *Second);
    if (!Result)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
    }
    return {Result};
  }
} // namespace ink::semantic
