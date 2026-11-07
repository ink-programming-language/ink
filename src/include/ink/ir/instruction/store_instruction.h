#pragma once

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/void_type.h"

namespace ink::ir
{
  // Produces no value and borrows both operands and the void type; all must outlive this instruction.
  // The destination must point to the source's exact type value; construction does not validate types.
  class StoreInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Store;

      constexpr StoreInstruction(ValueRef ValueType, ValueRef Destination, ValueRef Source) noexcept
          : Instruction(Kind, ValueType),
            Destination(Destination),
            Source(Source)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value &getDestination() const noexcept
      {
        return Destination.get();
      }

      constexpr const Value &getSource() const noexcept
      {
        return Source.get();
      }

    private:
      friend struct detail::InstructionOperands<StoreInstruction>;

      ValueRef Destination;
      ValueRef Source;
  };
} // namespace ink::ir
