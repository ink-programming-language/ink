#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class WhileStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::WhileStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      WhileStmt(SourceRange Range, Expr *Condition, Stmt *Body)
          : Stmt(Kind, Range),
            Condition(Condition),
            Body(Body)
      {
      }
      auto *condition(this auto &&Self) noexcept
      {
        return Self.Condition ? &std::forward_like<decltype(Self) &>(*Self.Condition) : nullptr;
      }
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      Expr *Condition;
      Stmt *Body;
  };
} // namespace ink::frontend::parser
