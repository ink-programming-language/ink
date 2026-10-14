#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *value(this auto &&Self) noexcept
      {
        return Self.Value ? &std::forward_like<decltype(Self) &>(*Self.Value) : nullptr;
      }
      auto *count(this auto &&Self) noexcept
      {
        return Self.Count ? &std::forward_like<decltype(Self) &>(*Self.Count) : nullptr;
      }

    private:
      Expr *Value;
      Expr *Count;
  };
} // namespace ink::frontend::parser
