#pragma once

#include <cstdint>
#include <limits>
#include <optional>

namespace ink::ir
{
  // Describes a memory operand as a base-table slot and a signed byte offset.
  // The VM updates the BP slot when entering/leaving a frame. Symbol/data slots are assigned by the caller or loader.
  class AddressOperand
  {
    public:
      static constexpr std::uint32_t BasePointerSlot = 0;

      constexpr explicit AddressOperand(std::uint32_t BaseIndex, std::int64_t ByteOffset = 0) noexcept
          : BaseIndex(BaseIndex),
            ByteOffset(ByteOffset)
      {
      }

      static constexpr AddressOperand bp(std::int64_t ByteOffset = 0) noexcept
      {
        return AddressOperand(BasePointerSlot, ByteOffset);
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
