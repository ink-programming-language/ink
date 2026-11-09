#pragma once

#include "ink/ir/function/basic_block.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/phi_instruction.h"
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

      // Creation requires a valid insertion point and returns a block-owned instruction reference.
      // API preconditions are checked with checkMsgf in assertion-enabled builds; callers must satisfy them in every build.
      // Terminators may only be created at the end of an unterminated block.
      /* -------------------- Add -------------------- */
      // Operands must have the same integer or floating-point type value.
      [[nodiscard]] AddInstruction &createAdd(ValueRef LHS, ValueRef RHS);

      /* -------------------- Alloca -------------------- */
      // The operand must be a type value; the borrowed value must outlive the type pool.
      [[nodiscard]] AllocaInstruction &createAlloca(ValueRef AllocatedType);

      /* -------------------- Load -------------------- */
      // The source must have PointerType; the result removes exactly one pointer layer.
      [[nodiscard]] LoadInstruction &createLoad(ValueRef Source);

      /* -------------------- Store -------------------- */
      // The destination must point to the source's exact type value.
      [[nodiscard]] StoreInstruction &createStore(ValueRef Destination, ValueRef Source);

      /* -------------------- Call -------------------- */
      // The callee must have FunctionType and arguments must match its fixed parameters; receivers are explicit first arguments.
      // Variadic callees accept additional non-void arguments. The instruction copies the borrowed argument reference list.
      [[nodiscard]] CallInstruction &createCall(ValueRef Callee, std::span<const ValueRef> Arguments = {});

      /* -------------------- Phi -------------------- */
      // ResultType must be a non-void type value; phi instructions must form a prefix at the start of the block.
      // Incoming pairs may be added after dependent instructions and loop backedges have been created.
      [[nodiscard]] PhiInstruction &createPhi(ValueRef ResultType);

      /* -------------------- Return -------------------- */
      // Owned blocks must return the enclosing signature's exact type; detached blocks have no signature to check.
      [[nodiscard]] ReturnInstruction &createReturn();
      [[nodiscard]] ReturnInstruction &createReturn(ValueRef ReturnValue);

      /* -------------------- Branch -------------------- */
      // The source and all destinations must belong to the same function; conditional branches require a boolean value.
      [[nodiscard]] BranchInstruction &createBranch(BasicBlock &Target);
      [[nodiscard]] CondBranchInstruction &createCondBranch(ValueRef Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget);

    private:
      void checkInsertPoint(InstructionKind Kind) const noexcept;

      template <typename InstructionType, typename... Arguments>
      InstructionType &createInstruction(Arguments &&...Args);

      TypePool &Types;
      BasicBlock *InsertBlock = nullptr;
      BasicBlock::InstructionStorage::iterator InsertPosition;
  };
} // namespace ink::ir
