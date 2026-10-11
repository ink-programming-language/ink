#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class MissingExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingExpr(SourceRange Range)
          : Expr(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
