#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class IfStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::IfStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      IfStmt(SourceRange Range, Expr *Condition, Stmt *ThenBranch, Stmt *ElseBranch)
          : Stmt(Kind, Range),
            Condition(Condition),
            ThenBranch(ThenBranch),
            ElseBranch(ElseBranch)
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
      Stmt *thenBranch() noexcept
      {
        return ThenBranch;
      }
      const Stmt *thenBranch() const noexcept
      {
        return ThenBranch;
      }
      Stmt *elseBranch() noexcept
      {
        return ElseBranch;
      }
      const Stmt *elseBranch() const noexcept
      {
        return ElseBranch;
      }

    private:
      Expr *Condition;
      Stmt *ThenBranch;
      Stmt *ElseBranch;
  };
} // namespace ink::frontend::parser
