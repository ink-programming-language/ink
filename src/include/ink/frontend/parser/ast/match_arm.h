#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class MatchArm
  {
    public:
      MatchArm(MatchPattern *Pattern, Expr *Guard, Expr *Value, SourceRange Range)
          : Pattern(Pattern),
            Guard(Guard),
            Value(Value),
            Range(Range)
      {
      }
      MatchPattern *pattern() noexcept
      {
        return Pattern;
      }
      const MatchPattern *pattern() const noexcept
      {
        return Pattern;
      }
      Expr *guard() noexcept
      {
        return Guard;
      }
      const Expr *guard() const noexcept
      {
        return Guard;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      MatchPattern *Pattern;
      Expr *Guard;
      Expr *Value;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
