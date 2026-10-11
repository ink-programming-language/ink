#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class ContinueStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ContinueStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ContinueStmt(SourceRange Range)
          : Stmt(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
