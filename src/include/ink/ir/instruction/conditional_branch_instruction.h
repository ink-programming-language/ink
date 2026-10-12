#ifndef INK_IR_INSTRUCTION_CONDITIONAL_BRANCH_INSTRUCTION_H
#define INK_IR_INSTRUCTION_CONDITIONAL_BRANCH_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

#include <cstddef>

namespace ink::ir
{
  class BasicBlock;
  class VoidType;

  // Borrows its void result type, boolean condition and both destinations; all must outlive this instruction.
  // Condition type and destination validity are checked by the IR producer.
  class ConditionalBranchInstruction final : public Instruction
  {
    public:
      ConditionalBranchInstruction(const VoidType &ResultType, const Value &Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget) noexcept;

      const Value *condition() const noexcept;
      const BasicBlock *trueTarget() const noexcept;
      const BasicBlock *falseTarget() const noexcept;
      std::size_t successorCount() const noexcept;
      // Successors are ordered true then false; out-of-range indices return nullptr.
      const BasicBlock *successor(std::size_t Index) const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *Condition = nullptr;
      const BasicBlock *TrueTarget = nullptr;
      const BasicBlock *FalseTarget = nullptr;
  };
} // namespace ink::ir

#endif
