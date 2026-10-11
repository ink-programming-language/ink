#pragma once

#include "ink/frontend/parser/ast/ast_node_base.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class TypeSyntax final : public ASTNodeBase
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TypeSyntax;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TypeSyntax(SourceRange Range, Expr *Expression)
          : ASTNodeBase(Kind, Range),
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
