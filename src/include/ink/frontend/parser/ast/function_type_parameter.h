#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <optional>
#include <utility>

namespace ink::frontend::parser
{
  class FunctionTypeParameter
  {
    public:
      FunctionTypeParameter(std::optional<NameToken> Name, TypeSyntax *Type, bool Variadic, SourceRange Range)
          : Name(Name),
            Type(Type),
            Variadic(Variadic),
            Range(Range)
      {
      }
      std::optional<NameToken> name() const noexcept
      {
        return Name;
      }
      auto *type(this auto &&Self) noexcept
      {
        return Self.Type ? &std::forward_like<decltype(Self) &>(*Self.Type) : nullptr;
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
      std::optional<NameToken> Name;
      TypeSyntax *Type;
      bool Variadic;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
