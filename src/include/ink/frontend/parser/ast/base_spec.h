#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"

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
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
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
