#pragma once

#include "ink/ir/function/basic_block.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"

namespace ink::ir
{
  class TypePool;

  // Borrows a type pool and an insertion point; the pool must outlive every instruction created with it.
  // The selected block and insertion anchor must remain alive while this builder uses the insertion point.
  // Blocks own created instructions, which remain alive after the builder is destroyed.
  class InstructionBuilder final
  {
    public:
      explicit InstructionBuilder(TypePool &Types) noexcept;
      InstructionBuilder(TypePool &&) = delete;

      InstructionBuilder(const InstructionBuilder &) = delete;
      InstructionBuilder &operator=(const InstructionBuilder &) = delete;
      InstructionBuilder(InstructionBuilder &&) = delete;
      InstructionBuilder &operator=(InstructionBuilder &&) = delete;

      // Selects the end of the block. Creating instructions preserves this insertion point.
      void setInsertPoint(BasicBlock &Block) noexcept;

      // Selects the position before an owned instruction; a detached instruction leaves the current point unchanged.
      [[nodiscard]] bool setInsertPoint(Instruction &Before) noexcept;

      void clearInsertPoint() noexcept;
      BasicBlock *getInsertBlock() const noexcept;
      Instruction *getInsertBefore() const noexcept;

      // Creation returns nullptr when there is no insertion point. Operand type requirements match the concrete classes.
      /* -------------------- Add -------------------- */
      [[nodiscard]] AddInstruction *createAdd(ValueRef LHS, ValueRef RHS);

      /* -------------------- Alloca -------------------- */
      // Returns nullptr unless the operand is a type value; the borrowed value must outlive the type pool.
      [[nodiscard]] AllocaInstruction *createAlloca(ValueRef AllocatedType);

      /* -------------------- Load -------------------- */
      // Returns nullptr unless the source has PointerType; the result removes exactly one pointer layer.
      [[nodiscard]] LoadInstruction *createLoad(ValueRef Source);

      /* -------------------- Store -------------------- */
      // Returns nullptr unless the destination points to the source's exact type value.
      [[nodiscard]] StoreInstruction *createStore(ValueRef Destination, ValueRef Source);

      /* -------------------- Return -------------------- */
      [[nodiscard]] ReturnInstruction *createReturn();
      [[nodiscard]] ReturnInstruction *createReturn(ValueRef ReturnValue);

    private:
      template <typename InstructionType, typename... Arguments>
      InstructionType *createInstruction(Arguments &&...Args);

      TypePool &Types;
      BasicBlock *InsertBlock = nullptr;
      BasicBlock::InstructionStorage::iterator InsertPosition;
  };
} // namespace ink::ir
