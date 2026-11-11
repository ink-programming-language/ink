#pragma once

#include "ink/backend/vm/instruction/operand.h"
#include "ink/backend/vm/instruction/register.h"

#include <cstdint>

namespace ink::vm
{
  class RegisterOperand final : public Operand
  {
    public:
      constexpr explicit RegisterOperand(RegisterId Register, std::int64_t ByteOffset = 0) noexcept
          : Operand(OperandKind::Register),
            Register(Register),
            ByteOffset(ByteOffset)
      {
      }

      constexpr RegisterId getRegister() const noexcept
      {
        return Register;
      }

      constexpr std::int64_t getByteOffset() const noexcept
      {
        return ByteOffset;
      }

    private:
      RegisterId Register;
      std::int64_t ByteOffset;
  };
} // namespace ink::vm
