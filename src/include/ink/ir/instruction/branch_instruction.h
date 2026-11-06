#pragma once

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  class BasicBlock;

  // Borrows a void result type and target block; both must outlive this instruction.
  // The caller must provide VoidType and a valid branch target; construction does not validate either operand.
  class BranchInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Branch;

      constexpr BranchInstruction(ValueRef ResultType, BasicBlock &Target) noexcept
          : Instruction(Kind, ResultType),
            Target(Target)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr auto &getTarget(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Target);
      }

    private:
      BasicBlock &Target;
  };
} // namespace ink::ir
