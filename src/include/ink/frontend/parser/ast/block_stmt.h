#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class BlockStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BlockStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BlockStmt(SourceRange Range, ASTArray<Stmt *> Statements, bool Synthetic)
          : Stmt(Kind, Range),
            Statements(Statements),
            Synthetic(Synthetic)
      {
      }
      auto statements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), Stmt>(Self.Statements);
      }
      bool synthetic() const noexcept
      {
        return Synthetic;
      }

    private:
      ASTArray<Stmt *> Statements;
      bool Synthetic;
  };
} // namespace ink::frontend::parser
