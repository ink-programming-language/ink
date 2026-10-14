#pragma once

#include "ink/frontend/parser/ast/simple_item.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class ExprItem final : public SimpleItem
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ExprItem;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ExprItem(SourceRange Range, Expr *Expression)
          : SimpleItem(Kind, Range),
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
