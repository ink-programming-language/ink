#pragma once

#include "ink/backend/vm/instruction/operand.h"
#include "ink/core/name/name.h"

#include <cstdint>
#include <optional>

namespace ink::vm
{
  enum class ImmediateValueKind : std::uint8_t
  {
    Numeric,
    Symbol,
  };

  // Stores numeric bits or an unresolved symbol address plus a signed byte offset.
  // Symbol names identify VM link symbols, never host pointers or encoded table indices.
  class ImmediateOperand final : public Operand
  {
    public:
      constexpr explicit ImmediateOperand(std::uint64_t Bits) noexcept
          : Operand(OperandKind::Immediate),
            ValueKind(ImmediateValueKind::Numeric),
            Bits(Bits)
      {
      }

      constexpr explicit ImmediateOperand(core::Name Symbol, std::int64_t ByteOffset = 0) noexcept
          : Operand(OperandKind::Immediate),
            ValueKind(ImmediateValueKind::Symbol),
            Symbol(Symbol),
            ByteOffset(ByteOffset)
      {
      }

      constexpr ImmediateValueKind getValueKind() const noexcept
      {
        return ValueKind;
      }

      constexpr std::optional<std::uint64_t> bits() const noexcept
      {
        if (ValueKind != ImmediateValueKind::Numeric)
        {
          return std::nullopt;
        }
        return Bits;
      }

      constexpr std::optional<core::Name> getSymbol() const noexcept
      {
        if (ValueKind != ImmediateValueKind::Symbol)
        {
          return std::nullopt;
        }
        return Symbol;
      }

      constexpr std::optional<std::int64_t> getByteOffset() const noexcept
      {
        if (ValueKind != ImmediateValueKind::Symbol)
        {
          return std::nullopt;
        }
        return ByteOffset;
      }

    private:
      ImmediateValueKind ValueKind;
      std::uint64_t Bits = 0;
      core::Name Symbol;
      std::int64_t ByteOffset = 0;
  };
} // namespace ink::vm
