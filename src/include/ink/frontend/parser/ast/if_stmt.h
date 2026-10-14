#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *condition(this auto &&Self) noexcept
      {
        return Self.Condition ? &std::forward_like<decltype(Self) &>(*Self.Condition) : nullptr;
      }
      auto *thenBranch(this auto &&Self) noexcept
      {
        return Self.ThenBranch ? &std::forward_like<decltype(Self) &>(*Self.ThenBranch) : nullptr;
      }
      auto *elseBranch(this auto &&Self) noexcept
      {
        return Self.ElseBranch ? &std::forward_like<decltype(Self) &>(*Self.ElseBranch) : nullptr;
      }

    private:
      Expr *Condition;
      Stmt *ThenBranch;
      Stmt *ElseBranch;
  };
} // namespace ink::frontend::parser
