#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class DeferStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DeferStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DeferStmt(SourceRange Range, Stmt *Body)
          : Stmt(Kind, Range),
            Body(Body)
      {
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
      Stmt *Body;
  };
} // namespace ink::frontend::parser
