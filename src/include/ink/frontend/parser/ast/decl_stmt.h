#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *declaration(this auto &&Self) noexcept
      {
        return Self.Declaration ? &std::forward_like<decltype(Self) &>(*Self.Declaration) : nullptr;
      }

    private:
      Decl *Declaration;
  };
} // namespace ink::frontend::parser
