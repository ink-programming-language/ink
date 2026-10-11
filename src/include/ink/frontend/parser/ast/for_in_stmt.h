#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

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
      BindingPattern *binding() noexcept
      {
        return Binding;
      }
      const BindingPattern *binding() const noexcept
      {
        return Binding;
      }
      Expr *iterable() noexcept
      {
        return Iterable;
      }
      const Expr *iterable() const noexcept
      {
        return Iterable;
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      BindingPattern *Binding;
      Expr *Iterable;
      Stmt *Body;
  };
} // namespace ink::frontend::parser
