#pragma once

#include "ink/backend/vm/instruction/operand.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/core/name/name.h"

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
            Register(Register),
            ByteOffset(ByteOffset)
      {
      }

      constexpr explicit AddressOperand(core::Name Symbol, std::int64_t ByteOffset = 0) noexcept
          : Operand(OperandKind::Address),
            BaseKind(AddressBaseKind::Symbol),
            Symbol(Symbol),
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
        return Register;
      }

      constexpr std::optional<core::Name> getSymbol() const noexcept
      {
        if (BaseKind != AddressBaseKind::Symbol)
        {
          return std::nullopt;
        }
        return Symbol;
      }

      constexpr std::int64_t getByteOffset() const noexcept
      {
        return ByteOffset;
      }

    private:
      AddressBaseKind BaseKind;
      RegisterId Register = RegisterId::BP;
      core::Name Symbol;
      std::int64_t ByteOffset;
  };
} // namespace ink::vm
