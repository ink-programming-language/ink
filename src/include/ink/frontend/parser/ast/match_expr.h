#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/match_arm.h"

namespace ink::frontend::parser
{
  class MatchExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MatchExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MatchExpr(SourceRange Range, Expr *Value, ASTArray<MatchArm> Arms)
          : Expr(Kind, Range),
            Value(Value),
            Arms(Arms)
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
      ASTArray<MatchArm> arms() const noexcept
      {
        return Arms;
      }

    private:
      Expr *Value;
      ASTArray<MatchArm> Arms;
  };
} // namespace ink::frontend::parser
