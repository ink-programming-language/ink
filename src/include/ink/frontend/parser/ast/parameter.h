#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class Parameter
  {
    public:
      Parameter(NameToken Name, TypeSyntax *Type, Expr *DefaultValue, bool Variadic, SourceRange Range)
          : Name(Name),
            Type(Type),
            DefaultValue(DefaultValue),
            Variadic(Variadic),
            Range(Range)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      Expr *defaultValue() noexcept
      {
        return DefaultValue;
      }
      const Expr *defaultValue() const noexcept
      {
        return DefaultValue;
      }
      bool variadic() const noexcept
      {
        return Variadic;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      NameToken Name;
      TypeSyntax *Type;
      Expr *DefaultValue;
      bool Variadic;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
