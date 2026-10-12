#ifndef INK_IR_INSTRUCTION_BRANCH_INSTRUCTION_H
#define INK_IR_INSTRUCTION_BRANCH_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

#include <cstddef>

namespace ink::ir
{
  class BasicBlock;
  class VoidType;

  // An unconditional jump that borrows its void result type and destination; both must outlive this instruction.
  // Destination validity is checked by the IR producer.
  class BranchInstruction final : public Instruction
  {
    public:
      BranchInstruction(const VoidType &ResultType, BasicBlock &Target) noexcept;

      const BasicBlock *target() const noexcept;
      std::size_t successorCount() const noexcept;
      // The sole successor is at index zero; out-of-range indices return nullptr.
      const BasicBlock *successor(std::size_t Index) const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const BasicBlock *Target = nullptr;
  };
} // namespace ink::ir

#endif
