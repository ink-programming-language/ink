#pragma once

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  // Borrows the source and result type; both must outlive this instruction.
  // The caller must provide a pointer source whose pointee type is ResultType; construction does not validate types.
  class LoadInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Load;

      constexpr LoadInstruction(ValueRef ResultType, ValueRef Source) noexcept
          : Instruction(Kind, ResultType),
            Source(Source)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value &getSource() const noexcept
      {
        return Source.get();
      }

    private:
      friend struct detail::InstructionOperands<LoadInstruction>;

      ValueRef Source;
  };
} // namespace ink::ir
