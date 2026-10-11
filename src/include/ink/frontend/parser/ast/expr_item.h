#pragma once

#include "ink/frontend/parser/ast/simple_item.h"
#include "ink/frontend/parser/ast/fwd.h"

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
