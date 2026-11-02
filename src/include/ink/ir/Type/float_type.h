#pragma once

#include "ink/ir/Type/type.h"

#include <cstdint>

namespace ink::ir
{
  class FloatType final : public Type
  {
    public:
      enum class Width : std::uint8_t
      {
        Bits32 = 32,
        Bits64 = 64,
      };

      static constexpr TypeKind Kind = TypeKind::Float;

      constexpr explicit FloatType(Width BitWidth) noexcept
          : Type(Kind),
            BitWidth(BitWidth)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return static_cast<std::uint32_t>(BitWidth);
      }

    private:
      Width BitWidth;
  };
} // namespace ink::ir
