#pragma once

#include "ink/frontend/parser/ast/ast_node_base.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *expression(this auto &&Self) noexcept
      {
        return Self.Expression ? &std::forward_like<decltype(Self) &>(*Self.Expression) : nullptr;
      }

    private:
      Expr *Expression;
  };
} // namespace ink::frontend::parser
