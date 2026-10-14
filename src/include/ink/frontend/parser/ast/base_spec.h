#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class BaseSpec
  {
    public:
      BaseSpec(bool Implements, TypeSyntax *Type, SourceRange Range)
          : Implements(Implements),
            Type(Type),
            Range(Range)
      {
      }
      bool implements() const noexcept
      {
        return Implements;
      }
      auto *type(this auto &&Self) noexcept
      {
        return Self.Type ? &std::forward_like<decltype(Self) &>(*Self.Type) : nullptr;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      bool Implements;
      TypeSyntax *Type;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
