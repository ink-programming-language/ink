#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class BinaryExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BinaryExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BinaryExpr(SourceRange Range, Expr *Left, TokenKind Op, SourceRange OperatorRange, Expr *Right)
          : Expr(Kind, Range),
            Left(Left),
            Op(Op),
            OperatorRange(OperatorRange),
            Right(Right)
      {
      }
      auto *left(this auto &&Self) noexcept
      {
        return Self.Left ? &std::forward_like<decltype(Self) &>(*Self.Left) : nullptr;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      auto *right(this auto &&Self) noexcept
      {
        return Self.Right ? &std::forward_like<decltype(Self) &>(*Self.Right) : nullptr;
      }

    private:
      Expr *Left;
      TokenKind Op;
      SourceRange OperatorRange;
      Expr *Right;
  };
} // namespace ink::frontend::parser
