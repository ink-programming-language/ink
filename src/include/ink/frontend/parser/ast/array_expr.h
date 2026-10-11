#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class ArrayExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayExpr(SourceRange Range, ASTArray<Expr *> Elements)
          : Expr(Kind, Range),
            Elements(Elements)
      {
      }
      ASTArray<Expr *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<Expr> elements() const noexcept
      {
        return ConstNodeArray<Expr>(Elements);
      }

    private:
      ASTArray<Expr *> Elements;
  };
} // namespace ink::frontend::parser
