#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class YieldStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::YieldStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      YieldStmt(SourceRange Range, bool Return, Expr *Value)
          : Stmt(Kind, Range),
            Return(Return),
            Value(Value)
      {
      }
      bool isReturn() const noexcept
      {
        return Return;
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
      bool Return;
      Expr *Value;
  };
} // namespace ink::frontend::parser
