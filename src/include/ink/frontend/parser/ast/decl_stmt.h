#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class DeclStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DeclStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DeclStmt(SourceRange Range, Decl *Declaration)
          : Stmt(Kind, Range),
            Declaration(Declaration)
      {
      }
      Decl *declaration() noexcept
      {
        return Declaration;
      }
      const Decl *declaration() const noexcept
      {
        return Declaration;
      }

    private:
      Decl *Declaration;
  };
} // namespace ink::frontend::parser
