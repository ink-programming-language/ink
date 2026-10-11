#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class UnaryExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::UnaryExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      UnaryExpr(SourceRange Range, TokenKind Op, SourceRange OperatorRange, Expr *Operand)
          : Expr(Kind, Range),
            Op(Op),
            OperatorRange(OperatorRange),
            Operand(Operand)
      {
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      Expr *operand() noexcept
      {
        return Operand;
      }
      const Expr *operand() const noexcept
      {
        return Operand;
      }

    private:
      TokenKind Op;
      SourceRange OperatorRange;
      Expr *Operand;
  };
} // namespace ink::frontend::parser
