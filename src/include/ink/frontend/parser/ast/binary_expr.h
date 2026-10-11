#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

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
      Expr *left() noexcept
      {
        return Left;
      }
      const Expr *left() const noexcept
      {
        return Left;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      Expr *right() noexcept
      {
        return Right;
      }
      const Expr *right() const noexcept
      {
        return Right;
      }

    private:
      Expr *Left;
      TokenKind Op;
      SourceRange OperatorRange;
      Expr *Right;
  };
} // namespace ink::frontend::parser
