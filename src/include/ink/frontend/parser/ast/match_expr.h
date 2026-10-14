#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/match_arm.h"
#include <utility>

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
      auto *value(this auto &&Self) noexcept
      {
        return Self.Value ? &std::forward_like<decltype(Self) &>(*Self.Value) : nullptr;
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
