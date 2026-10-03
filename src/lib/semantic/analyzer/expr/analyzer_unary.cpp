#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  namespace
  {
    const parser::LiteralExpr *negativeLiteral(const parser::UnaryExpr &Node)
    {
      if (Node.op() != tokenizer::TokenKind::Minus)
      {
        return nullptr;
      }
      const parser::Expr *Operand = Node.operand();
      while (parser::ParenExpr::classof(Operand))
      {
        Operand = static_cast<const parser::ParenExpr *>(Operand)->expression();
      }
      return parser::LiteralExpr::classof(Operand) && static_cast<const parser::LiteralExpr *>(Operand)->literalKind() == tokenizer::TokenKind::IntegerLiteral ? static_cast<const parser::LiteralExpr *>(Operand) : nullptr;
    }

    bool immediateNegativeLiteral(const parser::UnaryExpr &Node)
    {
      return Node.op() == tokenizer::TokenKind::Minus && parser::LiteralExpr::classof(Node.operand()) && static_cast<const parser::LiteralExpr *>(Node.operand())->literalKind() == tokenizer::TokenKind::IntegerLiteral;
    }
  } // namespace

  const parser::LiteralExpr *findDeferredIntegerLiteral(const parser::Expr &Node, std::size_t Depth, std::size_t Limit)
  {
    const parser::Expr *Current = &Node;
    while (!Current->isComptime() && Depth < Limit)
    {
      if (parser::ParenExpr::classof(Current))
      {
        Current = static_cast<const parser::ParenExpr *>(Current)->expression();
      }
      else if (parser::UnaryExpr::classof(Current))
      {
        const auto &Unary = static_cast<const parser::UnaryExpr &>(*Current);
        if (Unary.op() != tokenizer::TokenKind::Plus && Unary.op() != tokenizer::TokenKind::Minus)
        {
          return nullptr;
        }
        Current = Unary.operand();
      }
      else
      {
        return parser::LiteralExpr::classof(Current) && static_cast<const parser::LiteralExpr *>(Current)->literalKind() == tokenizer::TokenKind::IntegerLiteral ? static_cast<const parser::LiteralExpr *>(Current) : nullptr;
      }
      ++Depth;
    }
    return nullptr;
  }

  bool Analyzer::prepareIntegerLiteral(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    AnalysisState::TraversalGuard Traversal(State);
    if (!reportExecution(State, Traversal.status(), Node))
    {
      return false;
    }
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return false;
    }
    if (parser::ParenExpr::classof(&Node))
    {
      return prepareIntegerLiteral(State, *static_cast<const parser::ParenExpr &>(Node).expression(), Depth + 1);
    }
    if (parser::UnaryExpr::classof(&Node))
    {
      const auto &Unary = static_cast<const parser::UnaryExpr &>(Node);
      // The existing direct signed-literal path parses the sign and digits together,
      // without entering the unsigned literal child or performing a separate negate.
      if (immediateNegativeLiteral(Unary))
      {
        return true;
      }
      return prepareIntegerLiteral(State, *Unary.operand(), Depth + 1) && reportExecution(State, State.Context.comptimeState().Engine.consumeStep(), Node);
    }
    return true;
  }

  Analyzer::ExpressionResult Analyzer::materializeIntegerLiteral(AnalysisState &State, const parser::Expr &Node, const ir::Type &Target)
  {
    if (parser::ParenExpr::classof(&Node))
    {
      return materializeIntegerLiteral(State, *static_cast<const parser::ParenExpr &>(Node).expression(), Target);
    }
    if (parser::LiteralExpr::classof(&Node))
    {
      return {convertExpression(State, {nullptr, static_cast<const parser::LiteralExpr *>(&Node)}, Target, Node)};
    }
    const auto &Unary = static_cast<const parser::UnaryExpr &>(Node);
    if (const auto *Literal = negativeLiteral(Unary))
    {
      return {convertExpression(State, {nullptr, Literal, true}, Target, Node)};
    }
    const ExpressionResult Operand = materializeIntegerLiteral(State, *Unary.operand(), Target);
    if (!Operand || Unary.op() == tokenizer::TokenKind::Plus)
    {
      return Operand;
    }
    const auto &Integer = static_cast<const ir::IntegerConstant &>(*Operand.ValueObject);
    const execution::ExecutionInteger Bits(Integer.value());
    execution::ExecutionInteger Negated(Bits.bitWidth());
    if (!reportExecution(State, Bits.negate(Negated), Node))
    {
      return {};
    }
    return {State.Context.constantPool().getIntegerConstant(static_cast<const ir::IntegerType &>(Target), Negated.bits())};
  }

  Analyzer::ExpressionResult Analyzer::analyzeUnaryExpr(AnalysisState &State, const parser::UnaryExpr &Node, std::size_t Depth)
  {
    if (Node.op() == tokenizer::TokenKind::Amp)
    {
      return {resolveAddress(State, *Node.operand(), Depth + 1)};
    }
    if (Node.op() == tokenizer::TokenKind::Star)
    {
      AnalysisState::EvaluationGuard Expected(State, State.Evaluating, nullptr);
      const ExpressionResult Operand = analyzeExpr(State, *Node.operand(), Depth + 1);
      if (!Operand)
      {
        return {};
      }
      if (!Operand.ValueObject || !ir::PointerType::classof(&Operand.ValueObject->type()) || static_cast<const ir::PointerType &>(Operand.ValueObject->type()).pointeeType().typeKind() == ir::TypeKind::Void)
      {
        State.report<core::DiagnosticKind::SemanticInvalidDereference>(Node.getSourceRange());
        return {};
      }
      if (State.Evaluating)
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const ir::Value *Result = State.Builder.createLoadInstruction(*Operand.ValueObject);
      if (!Result)
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      }
      return {Result};
    }
    if (Node.op() == tokenizer::TokenKind::PlusPlus || Node.op() == tokenizer::TokenKind::MinusMinus)
    {
      return analyzeUpdateExpr(State, *Node.operand(), Node.op(), false, Node, Depth);
    }
    if (Node.op() == tokenizer::TokenKind::Bang)
    {
      AnalysisState::EvaluationGuard Guard(State, State.Evaluating, nullptr);
      const ExpressionResult Operand = analyzeExpr(State, *Node.operand(), Depth + 1);
      if (!Operand)
      {
        return {};
      }
      const ir::Value *Value = convertExpression(State, Operand, State.Context.typePool().getType<ir::TypeKind::Bool>(), *Node.operand());
      if (!Value)
      {
        return {};
      }
      if (State.Evaluating)
      {
        if (!ir::BoolConstant::classof(Value))
        {
          reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
          return {};
        }
        const auto Result = State.Context.comptimeState().Engine.evaluateUnary(Node.op(), static_cast<const ir::BoolConstant &>(*Value));
        return reportExecution(State, Result.Status, Node) ? ExpressionResult{Result.Value} : ExpressionResult{};
      }
      const ir::Value *Result = State.Builder.createLogicalNotInstruction(*Value);
      if (!Result)
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      }
      return {Result};
    }
    if (State.Evaluating)
    {
      // Parentheses do not force the positive magnitude of a negative literal to fit
      // first. Other unary operators still execute in order at the selected width.
      if ((Node.op() == tokenizer::TokenKind::Plus || Node.op() == tokenizer::TokenKind::Minus) && (immediateNegativeLiteral(Node) || findDeferredIntegerLiteral(*Node.operand(), Depth + 1, State.ExpressionDepthLimit)))
      {
        if (!immediateNegativeLiteral(Node) && (!prepareIntegerLiteral(State, *Node.operand(), Depth + 1) || !reportExecution(State, State.Context.comptimeState().Engine.consumeStep(), Node)))
        {
          return {};
        }
        const auto *Target = State.ExpectedType && ir::IntegerType::classof(State.ExpectedType) ? State.ExpectedType : State.Context.typePool().getType<ir::TypeKind::Integer>(32, true);
        return materializeIntegerLiteral(State, Node, *Target);
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
