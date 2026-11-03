#pragma once

#include <cstdint>
#include <limits>
#include <optional>

namespace ink::ir
{
  // Describes a memory operand as a base-table slot and a signed byte offset.
  // Its bytecode encoding is a 32-bit slot followed by a signed 64-bit byte offset, without padding.
  // The VM maintains BP, SP, and PC in reserved slots 0, 1, and 2. Symbol/data slots start at 3 and are assigned by the caller or loader.
  class AddressOperand
  {
    public:
      static constexpr std::uint32_t BasePointerSlot = 0;
      static constexpr std::uint32_t StackPointerSlot = 1;
      static constexpr std::uint32_t ProgramCounterSlot = 2;
      static constexpr std::uint32_t ReservedSlotCount = ProgramCounterSlot + 1;

      constexpr explicit AddressOperand(std::uint32_t BaseIndex, std::int64_t ByteOffset = 0) noexcept
          : BaseIndex(BaseIndex),
            ByteOffset(ByteOffset)
      {
      }

      static constexpr AddressOperand bp(std::int64_t ByteOffset = 0) noexcept
      {
        return AddressOperand(BasePointerSlot, ByteOffset);
      }

      static constexpr AddressOperand sp(std::int64_t ByteOffset = 0) noexcept
      {
        return AddressOperand(StackPointerSlot, ByteOffset);
      }

      static constexpr AddressOperand pc(std::int64_t ByteOffset = 0) noexcept
      {
        return AddressOperand(ProgramCounterSlot, ByteOffset);
      }

      static constexpr AddressOperand data(std::uint32_t BaseIndex, std::int64_t ByteOffset = 0) noexcept
      {
        return AddressOperand(BaseIndex, ByteOffset);
      }

      constexpr std::uint32_t baseIndex() const noexcept
      {
        return BaseIndex;
      }

      constexpr std::int64_t byteOffset() const noexcept
      {
        return ByteOffset;
      }

      constexpr std::optional<AddressOperand> operator+(std::int64_t Amount) const noexcept
      {
        if ((Amount > 0 && ByteOffset > std::numeric_limits<std::int64_t>::max() - Amount) || (Amount < 0 && ByteOffset < std::numeric_limits<std::int64_t>::min() - Amount))
        {
          return std::nullopt;
        }
        return AddressOperand(BaseIndex, ByteOffset + Amount);
      }

      constexpr std::optional<AddressOperand> operator-(std::int64_t Amount) const noexcept
      {
        if ((Amount > 0 && ByteOffset < std::numeric_limits<std::int64_t>::min() + Amount) || (Amount < 0 && ByteOffset > std::numeric_limits<std::int64_t>::max() + Amount))
        {
          return std::nullopt;
        }
        return AddressOperand(BaseIndex, ByteOffset - Amount);
      }

      constexpr bool operator==(const AddressOperand &) const noexcept = default;

    private:
      std::uint32_t BaseIndex;
      std::int64_t ByteOffset;
  };
} // namespace ink::ir
