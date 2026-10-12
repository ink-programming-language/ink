#include "ink/ir/instruction/branch_instruction.h"

#include "ink/ir/type/builtin_type/void_type.h"

namespace ink::ir
{
  BranchInstruction::BranchInstruction(const VoidType &ResultType, BasicBlock &Target) noexcept
      : Instruction(ValueKind::BranchInstruction, &ResultType),
        Target(&Target)
  {
  }

  const BasicBlock *BranchInstruction::target() const noexcept
  {
    return Target;
  }

  std::size_t BranchInstruction::successorCount() const noexcept
  {
    return 1;
  }

  const BasicBlock *BranchInstruction::successor(std::size_t Index) const noexcept
  {
    return Index == 0 ? Target : nullptr;
  }

  bool BranchInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::BranchInstruction;
  }
} // namespace ink::ir
