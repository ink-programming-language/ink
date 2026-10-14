#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *expression(this auto &&Self) noexcept
      {
        return Self.Expression ? &std::forward_like<decltype(Self) &>(*Self.Expression) : nullptr;
      }

    private:
      Expr *Expression;
  };
} // namespace ink::frontend::parser
