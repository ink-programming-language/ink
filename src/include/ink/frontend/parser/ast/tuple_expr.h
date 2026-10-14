#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class TupleExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TupleExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TupleExpr(SourceRange Range, ASTArray<Expr *> Elements)
          : Expr(Kind, Range),
            Elements(Elements)
      {
      }
      auto elements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), Expr>(Self.Elements);
      }

    private:
      ASTArray<Expr *> Elements;
  };
} // namespace ink::frontend::parser
