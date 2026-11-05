#pragma once

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  // Borrows both operands and their type; they must outlive this instruction.
  // The caller must provide operands of the same IntegerType or FloatType; construction does not validate types.
  class AddInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Add;

      constexpr AddInstruction(ValueRef LHS, ValueRef RHS) noexcept
          : Instruction(Kind, LHS.get().getType()),
            LHS(LHS.get()),
            RHS(RHS.get())
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value &getLHS() const noexcept
      {
        return LHS;
      }

      constexpr const Value &getRHS() const noexcept
      {
        return RHS;
      }

    private:
      const Value &LHS;
      const Value &RHS;
  };
} // namespace ink::ir
