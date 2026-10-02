#ifndef INK_IR_BRANCH_INSTRUCTION_H
#define INK_IR_BRANCH_INSTRUCTION_H

#include "ink/ir/function/basic_block.h"

namespace ink::ir
{
  // Ends the current block and transfers control to another block in the same function.
  class BranchInstruction final : public Value
  {
    public:
      const BasicBlock &target() const noexcept
      {
        return Target;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::BranchInstruction;
      }

    private:
      BranchInstruction(const Type &VoidType, const BasicBlock &Target) noexcept
          : Value(Target.context(), ValueKind::BranchInstruction, VoidType),
            Target(Target)
      {
      }

      const BasicBlock &Target;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
