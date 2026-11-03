#pragma once

#include "ink/ir/type/type_kind.h"

#include <cstdint>
#include <string_view>

namespace ink::ir
{
  enum class TypeSemantics : std::uint8_t
  {
    Value,
    Reference,
  };

  class Type
  {
    public:
      constexpr TypeKind getKind() const noexcept
      {
        return Kind;
      }

      std::string_view getName() const noexcept
      {
        return typeKindName(Kind);
      }

      virtual constexpr TypeSemantics getSemantics() const noexcept
      {
        return TypeSemantics::Value;
      }

      Type(const Type &) = delete;
      Type &operator=(const Type &) = delete;

    protected:
      constexpr explicit Type(TypeKind Kind) noexcept
          : Kind(Kind)
      {
      }

      // Types are destroyed through their concrete class, never through Type.
      ~Type() = default;

    private:
      TypeKind Kind;
  };
} // namespace ink::ir
