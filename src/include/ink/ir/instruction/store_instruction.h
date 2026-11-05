#pragma once

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/void_type.h"

namespace ink::ir
{
  // Produces no value and borrows both operands and the void type; all must outlive this instruction.
  // The destination must reference the source's exact type object; construction does not validate types.
  class StoreInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Store;

      constexpr StoreInstruction(const VoidType &ValueType, ValueRef Destination, ValueRef Source) noexcept
          : Instruction(Kind, ValueType),
            Destination(Destination.get()),
            Source(Source.get())
      {
      }

      StoreInstruction(const VoidType &&, ValueRef, ValueRef) = delete;

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value &getDestination() const noexcept
      {
        return Destination;
      }

      constexpr const Value &getSource() const noexcept
      {
        return Source;
      }

    private:
      const Value &Destination;
      const Value &Source;
  };
} // namespace ink::ir
