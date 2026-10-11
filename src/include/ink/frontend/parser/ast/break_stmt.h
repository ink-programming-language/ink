#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class BreakStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BreakStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BreakStmt(SourceRange Range)
          : Stmt(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
