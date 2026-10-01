#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ir;
  using tokenizer::TokenKind;

  Analyzer::ExpressionResult Analyzer::analyzeBinaryExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth)
  {
    if (!State.Evaluating && Node.op() != TokenKind::Plus)
    {
      reportUnsupported(State, Node);
      return {};
    }
    const ExpressionResult Left = analyzeExpr(State, *Node.left(), Depth + 1);
    if (!Left)
    {
      return {};
    }
    if (State.Evaluating && (Node.op() == TokenKind::AmpAmp || Node.op() == TokenKind::PipePipe))
    {
      if (!Left.ValueObject || !BoolConstant::classof(Left.ValueObject))
      {
        reportExecution(State, execution::ExecutionStatus::TypeMismatch, Node);
        return {};
      }
      const bool Value = static_cast<const BoolConstant &>(*Left.ValueObject).value();
      if ((Node.op() == TokenKind::AmpAmp && !Value) || (Node.op() == TokenKind::PipePipe && Value))
      {
        return Left;
      }
    }
    const Type *Expected = Left.ValueObject ? &Left.ValueObject->type() : State.ExpectedType;
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
} // namespace ink::semantic
