#pragma once

#include "ink/ir/type/type.h"

#include <cstdint>

namespace ink::ir
{
  class IntegerType final : public Type
  {
    public:
      enum class Width : std::uint8_t
      {
        Bits8 = 8,
        Bits16 = 16,
        Bits32 = 32,
        Bits64 = 64,
      };

      enum class Signedness : std::uint8_t
      {
        Signed,
        Unsigned,
      };

      static constexpr TypeKind Kind = TypeKind::Integer;

      constexpr IntegerType(Width BitWidth, Signedness Sign) noexcept
          : Type(Kind),
            BitWidth(BitWidth),
            Sign(Sign)
      {
      }

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return static_cast<std::uint32_t>(BitWidth);
      }

      constexpr bool isSigned() const noexcept
      {
        return Sign == Signedness::Signed;
      }

      constexpr Signedness signedness() const noexcept
      {
        return Sign;
      }

    private:
      Width BitWidth;
      Signedness Sign;
  };
} // namespace ink::ir
