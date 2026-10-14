#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *object(this auto &&Self) noexcept
      {
        return Self.Object ? &std::forward_like<decltype(Self) &>(*Self.Object) : nullptr;
      }
      auto *index(this auto &&Self) noexcept
      {
        return Self.Index ? &std::forward_like<decltype(Self) &>(*Self.Index) : nullptr;
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
