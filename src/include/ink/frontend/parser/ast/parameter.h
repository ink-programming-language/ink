#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *type(this auto &&Self) noexcept
      {
        return Self.Type ? &std::forward_like<decltype(Self) &>(*Self.Type) : nullptr;
      }
      auto *defaultValue(this auto &&Self) noexcept
      {
        return Self.DefaultValue ? &std::forward_like<decltype(Self) &>(*Self.DefaultValue) : nullptr;
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
