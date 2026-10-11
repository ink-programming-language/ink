#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class ReturnStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ReturnStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ReturnStmt(SourceRange Range, Expr *Value)
          : Stmt(Kind, Range),
            Value(Value)
      {
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }

    private:
      Expr *Value;
  };
} // namespace ink::frontend::parser
