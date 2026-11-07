#pragma once

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/void_type.h"

namespace ink::ir
{
  // Produces no value itself; the borrowed void type and optional return operand must outlive this instruction.
  // Matching the return operand to the enclosing function signature is the caller's responsibility.
  class ReturnInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Return;

      constexpr explicit ReturnInstruction(ValueRef ValueType) noexcept
          : Instruction(Kind, ValueType)
      {
      }

      constexpr ReturnInstruction(ValueRef ValueType, ValueRef ReturnValue) noexcept
          : Instruction(Kind, ValueType),
            ReturnValue(&ReturnValue.get())
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value *getReturnValue() const noexcept
      {
        return ReturnValue;
      }

    private:
      friend struct detail::InstructionOperands<ReturnInstruction>;

      const Value *ReturnValue = nullptr;
  };
} // namespace ink::ir
