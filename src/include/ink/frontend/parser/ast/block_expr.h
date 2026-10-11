#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class BlockExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BlockExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BlockExpr(SourceRange Range, BlockStmt *Body)
          : Expr(Kind, Range),
            Body(Body)
      {
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }

    private:
      BlockStmt *Body;
  };
} // namespace ink::frontend::parser
