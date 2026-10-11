#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class LiteralMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::LiteralMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      LiteralMatchPattern(SourceRange Range, LiteralExpr *Value, bool Negative)
          : MatchPattern(Kind, Range),
            Value(Value),
            Negative(Negative)
      {
      }
      LiteralExpr *value() noexcept
      {
        return Value;
      }
      const LiteralExpr *value() const noexcept
      {
        return Value;
      }
      bool negative() const noexcept
      {
        return Negative;
      }

    private:
      LiteralExpr *Value;
      bool Negative;
  };
} // namespace ink::frontend::parser
