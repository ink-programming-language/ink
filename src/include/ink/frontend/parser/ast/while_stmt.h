#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

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
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
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
      Expr *Condition;
      Stmt *Body;
  };
} // namespace ink::frontend::parser
