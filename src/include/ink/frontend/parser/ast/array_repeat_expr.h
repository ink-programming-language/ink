#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class ArrayRepeatExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayRepeatExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayRepeatExpr(SourceRange Range, Expr *Value, Expr *Count)
          : Expr(Kind, Range),
            Value(Value),
            Count(Count)
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
      Expr *count() noexcept
      {
        return Count;
      }
      const Expr *count() const noexcept
      {
        return Count;
      }

    private:
      Expr *Value;
      Expr *Count;
  };
} // namespace ink::frontend::parser
