#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *pattern(this auto &&Self) noexcept
      {
        return Self.Pattern ? &std::forward_like<decltype(Self) &>(*Self.Pattern) : nullptr;
      }
      auto *guard(this auto &&Self) noexcept
      {
        return Self.Guard ? &std::forward_like<decltype(Self) &>(*Self.Guard) : nullptr;
      }
      auto *value(this auto &&Self) noexcept
      {
        return Self.Value ? &std::forward_like<decltype(Self) &>(*Self.Value) : nullptr;
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
