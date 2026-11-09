#pragma once

#include "ink/backend/vm/instruction/operand.h"
#include "ink/backend/vm/instruction/register.h"

#include <cstdint>
#include <optional>

namespace ink::vm
{
  enum class AddressBaseKind : std::uint8_t
  {
    Register,
    Symbol,
  };

  // Describes a relative address; the opcode determines memory access or control transfer.
  // Contains only a logical base and byte displacement, never a runtime address or host pointer.
  class AddressOperand final : public Operand
  {
    public:
      constexpr AddressOperand(RegisterId Register, std::int64_t ByteOffset = 0) noexcept
          : Operand(OperandKind::Address),
            BaseKind(AddressBaseKind::Register),
            BaseIndex(static_cast<std::uint32_t>(Register)),
            ByteOffset(ByteOffset)
      {
      }

      constexpr AddressOperand(std::uint32_t BaseReferenceIndex, std::int64_t ByteOffset = 0) noexcept
          : Operand(OperandKind::Address),
            BaseKind(AddressBaseKind::Symbol),
            BaseIndex(BaseReferenceIndex),
            ByteOffset(ByteOffset)
      {
      }

      constexpr AddressBaseKind getBaseKind() const noexcept
      {
        return BaseKind;
      }

      constexpr std::optional<RegisterId> getRegister() const noexcept
      {
        if (BaseKind != AddressBaseKind::Register)
        {
          return std::nullopt;
        }
        return static_cast<RegisterId>(BaseIndex);
      }

      constexpr std::optional<std::uint32_t> getBaseReferenceIndex() const noexcept
      {
        if (BaseKind != AddressBaseKind::Symbol)
        {
          return std::nullopt;
        }
        return BaseIndex;
      }

      constexpr std::int64_t getByteOffset() const noexcept
      {
        return ByteOffset;
      }

    private:
      AddressBaseKind BaseKind;
      std::uint32_t BaseIndex;
      std::int64_t ByteOffset;
  };
} // namespace ink::vm
