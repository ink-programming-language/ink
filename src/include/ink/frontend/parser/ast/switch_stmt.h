#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/switch_clause.h"

namespace ink::frontend::parser
{
  class SwitchStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::SwitchStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      SwitchStmt(SourceRange Range, Expr *Value, ASTArray<SwitchClause> Clauses)
          : Stmt(Kind, Range),
            Value(Value),
            Clauses(Clauses)
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
      ASTArray<SwitchClause> clauses() const noexcept
      {
        return Clauses;
      }

    private:
      Expr *Value;
      ASTArray<SwitchClause> Clauses;
  };
} // namespace ink::frontend::parser
