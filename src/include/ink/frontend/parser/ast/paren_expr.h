#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class ParenExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ParenExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ParenExpr(SourceRange Range, Expr *Expression)
          : Expr(Kind, Range),
            Expression(Expression)
      {
      }
      Expr *expression() noexcept
      {
        return Expression;
      }
      const Expr *expression() const noexcept
      {
        return Expression;
      }

    private:
      Expr *Expression;
  };
} // namespace ink::frontend::parser
