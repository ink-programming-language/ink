#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  Analyzer::ExpressionResult Analyzer::analyzeUnaryExpr(AnalysisState &State, const parser::UnaryExpr &Node, std::size_t Depth)
  {
    if (Node.op() == tokenizer::TokenKind::PlusPlus || Node.op() == tokenizer::TokenKind::MinusMinus)
    {
      return analyzeUpdateExpr(State, *Node.operand(), Node.op(), false, Node, Depth);
    }
    if (State.Evaluating)
    {
      // Parse a signed literal at its target width, including the minimum negative value.
      if (Node.op() == tokenizer::TokenKind::Minus && parser::LiteralExpr::classof(Node.operand()) && static_cast<const parser::LiteralExpr *>(Node.operand())->literalKind() == tokenizer::TokenKind::IntegerLiteral)
      {
        const auto *Target = State.ExpectedType && ir::IntegerType::classof(State.ExpectedType) ? State.ExpectedType : State.Context.typePool().getType<ir::TypeKind::Integer>(32, true);
        return {convertExpression(State, {nullptr, static_cast<const parser::LiteralExpr *>(Node.operand()), true}, *Target, Node)};
      }
      const ExpressionResult Operand = analyzeExpr(State, *Node.operand(), Depth + 1);
      if (!Operand)
      {
        return {};
      }
      if (!Operand.ValueObject || !ir::Constant::classof(Operand.ValueObject))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const auto Result = State.Context.comptimeState().Engine.evaluateUnary(Node.op(), static_cast<const ir::Constant &>(*Operand.ValueObject));
      return reportExecution(State, Result.Status, Node) ? ExpressionResult{Result.Value} : ExpressionResult{};
    }
    if (Node.op() == tokenizer::TokenKind::Plus || Node.op() == tokenizer::TokenKind::Minus)
    {
      ExpressionResult Result = analyzeExpr(State, *Node.operand(), Depth + 1);
      if (!Result)
      {
        return {};
      }
      if (Result.IntegerLiteral)
      {
        Result.Negative = Result.Negative != (Node.op() == tokenizer::TokenKind::Minus);
        return Result;
      }
    }
    reportUnsupported(State, Node);
    return {};
  }
} // namespace ink::semantic
