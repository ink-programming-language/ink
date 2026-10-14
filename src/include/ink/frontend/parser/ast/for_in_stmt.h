#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class ForInStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ForInStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ForInStmt(SourceRange Range, BindingPattern *Binding, Expr *Iterable, Stmt *Body)
          : Stmt(Kind, Range),
            Binding(Binding),
            Iterable(Iterable),
            Body(Body)
      {
      }
      auto *binding(this auto &&Self) noexcept
      {
        return Self.Binding ? &std::forward_like<decltype(Self) &>(*Self.Binding) : nullptr;
      }
      auto *iterable(this auto &&Self) noexcept
      {
        return Self.Iterable ? &std::forward_like<decltype(Self) &>(*Self.Iterable) : nullptr;
      }
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      BindingPattern *Binding;
      Expr *Iterable;
      Stmt *Body;
  };
} // namespace ink::frontend::parser
