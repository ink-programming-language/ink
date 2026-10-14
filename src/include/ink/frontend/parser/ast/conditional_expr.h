#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *condition(this auto &&Self) noexcept
      {
        return Self.Condition ? &std::forward_like<decltype(Self) &>(*Self.Condition) : nullptr;
      }
      auto *then(this auto &&Self) noexcept
      {
        return Self.Then ? &std::forward_like<decltype(Self) &>(*Self.Then) : nullptr;
      }
      auto *elseValue(this auto &&Self) noexcept
      {
        return Self.Else ? &std::forward_like<decltype(Self) &>(*Self.Else) : nullptr;
      }

    private:
      Expr *Condition;
      Expr *Then;
      Expr *Else;
  };
} // namespace ink::frontend::parser
