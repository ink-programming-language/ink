#pragma once

#include "ink/backend/vm/instruction/operand.h"
#include "ink/backend/vm/instruction/register.h"

namespace ink::vm
{
  class RegisterOperand final : public Operand
  {
    public:
      constexpr explicit RegisterOperand(RegisterId Register) noexcept
          : Operand(OperandKind::Register),
            Register(Register)
      {
      }

      constexpr RegisterId getRegister() const noexcept
      {
        return Register;
      }

    private:
      RegisterId Register;
  };
} // namespace ink::vm
