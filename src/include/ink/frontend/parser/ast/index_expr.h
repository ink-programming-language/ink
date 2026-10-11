#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class IndexExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::IndexExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      IndexExpr(SourceRange Range, Expr *Object, Expr *Index, bool Optional)
          : Expr(Kind, Range),
            Object(Object),
            Index(Index),
            Optional(Optional)
      {
      }
      Expr *object() noexcept
      {
        return Object;
      }
      const Expr *object() const noexcept
      {
        return Object;
      }
      Expr *index() noexcept
      {
        return Index;
      }
      const Expr *index() const noexcept
      {
        return Index;
      }
      bool optional() const noexcept
      {
        return Optional;
      }

    private:
      Expr *Object;
      Expr *Index;
      bool Optional;
  };
} // namespace ink::frontend::parser
