#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class ConditionalExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ConditionalExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ConditionalExpr(SourceRange Range, Expr *Condition, Expr *Then, Expr *Else)
          : Expr(Kind, Range),
            Condition(Condition),
            Then(Then),
            Else(Else)
      {
      }
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
      }
      Expr *then() noexcept
      {
        return Then;
      }
      const Expr *then() const noexcept
      {
        return Then;
      }
      Expr *elseValue() noexcept
      {
        return Else;
      }
      const Expr *elseValue() const noexcept
      {
        return Else;
      }

    private:
      Expr *Condition;
      Expr *Then;
      Expr *Else;
  };
} // namespace ink::frontend::parser
