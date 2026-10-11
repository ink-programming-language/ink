#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class PostfixUpdateExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::PostfixUpdateExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      PostfixUpdateExpr(SourceRange Range, Expr *Operand, TokenKind Op, SourceRange OperatorRange)
          : Expr(Kind, Range),
            Operand(Operand),
            Op(Op),
            OperatorRange(OperatorRange)
      {
      }
      Expr *operand() noexcept
      {
        return Operand;
      }
      const Expr *operand() const noexcept
      {
        return Operand;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }

    private:
      Expr *Operand;
      TokenKind Op;
      SourceRange OperatorRange;
  };
} // namespace ink::frontend::parser
