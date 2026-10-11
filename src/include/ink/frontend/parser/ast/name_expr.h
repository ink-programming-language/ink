#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class NameExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::NameExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      NameExpr(SourceRange Range, NameToken Name)
          : Expr(Kind, Range),
            Name(Name)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }

    private:
      NameToken Name;
  };
} // namespace ink::frontend::parser
