#pragma once

#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/module/machine_basic_block.h"

#include <type_traits>
#include <utility>

namespace ink::vm
{
  // Borrows an insertion point. The selected block and any insertion anchor must outlive its use by the builder.
  // Instructions remain block-owned after the builder is destroyed. Inserting an instruction keeps the insertion point.
  class MachineInstructionBuilder final
  {
    public:
      MachineInstructionBuilder() noexcept = default;
      MachineInstructionBuilder(const MachineInstructionBuilder &) = delete;
      MachineInstructionBuilder &operator=(const MachineInstructionBuilder &) = delete;
      MachineInstructionBuilder(MachineInstructionBuilder &&) = delete;
      MachineInstructionBuilder &operator=(MachineInstructionBuilder &&) = delete;

      void setInsertPoint(MachineBasicBlock &Block) noexcept;

      // Selects the position before an owned instruction; failure leaves the current insertion point unchanged.
      [[nodiscard]] bool setInsertPoint(MachineInstruction &Before) noexcept;

      void clearInsertPoint() noexcept;
      MachineBasicBlock *getInsertBlock() const noexcept;
      MachineInstruction *getInsertBefore() const noexcept;

      // Requires an insertion point in every build mode; use tryCreate when its absence is recoverable.
      template <Opcode Operation, typename... ValueTypes>
      MachineInstructionImpl<Operation> &create(ValueTypes &&...Values) requires(std::is_constructible_v<MachineInstructionImpl<Operation>, ValueTypes && ...>)
      {
        checkInsertPoint();
        return *tryCreate<Operation>(std::forward<ValueTypes>(Values)...);
      }

      // Returns nullptr without allocating or modifying a block when no insertion point is selected.
      template <Opcode Operation, typename... ValueTypes>
      [[nodiscard]] MachineInstructionImpl<Operation> *tryCreate(ValueTypes &&...Values) requires(std::is_constructible_v<MachineInstructionImpl<Operation>, ValueTypes && ...>)
      {
        if (!InsertBlock)
        {
          return nullptr;
        }
        const auto Destroy = [](MachineInstruction *Value) noexcept
        {
          delete static_cast<MachineInstructionImpl<Operation> *>(Value);
        };
        MachineBasicBlock::InstructionOwner Owned(new MachineInstructionImpl<Operation>(std::forward<ValueTypes>(Values)...), Destroy);
        auto *Created = static_cast<MachineInstructionImpl<Operation> *>(Owned.get());
        InsertBlock->Instructions.insert(InsertPosition, std::move(Owned));
        Created->Parent = InsertBlock;
        return Created;
      }

    private:
      void checkInsertPoint() const noexcept;

      MachineBasicBlock *InsertBlock = nullptr;
      MachineBasicBlock::InstructionStorage::iterator InsertPosition;
  };
} // namespace ink::vm
