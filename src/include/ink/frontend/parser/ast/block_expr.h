#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      BlockStmt *Body;
  };
} // namespace ink::frontend::parser
