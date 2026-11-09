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

      // Requires an insertion point and, for branches, a nonnull target in the insertion block's function.
      template <Opcode Operation, typename... ValueTypes>
      MachineInstructionImpl<Operation> &create(ValueTypes &&...Values) requires(std::is_constructible_v<MachineInstructionImpl<Operation>, ValueTypes && ...>)
      {
        checkInsertPoint();
        auto *Created = tryCreate<Operation>(std::forward<ValueTypes>(Values)...);
        if (!Created)
        {
          invalidBranchTarget();
        }
        return *Created;
      }

      // Returns nullptr without allocation or mutation if the insertion point or branch target ownership is invalid.
      template <Opcode Operation, typename... ValueTypes>
      [[nodiscard]] MachineInstructionImpl<Operation> *tryCreate(ValueTypes &&...Values) requires(std::is_constructible_v<MachineInstructionImpl<Operation>, ValueTypes && ...>)
      {
        if (!InsertBlock)
        {
          return nullptr;
        }
        if constexpr (OpcodeTraits<Operation>::Category == InstructionCategory::Branch)
        {
          const auto Operands = std::forward_as_tuple(Values...);
          const auto &TargetValue = std::get<0>(Operands);
          const MachineBasicBlock *Target = nullptr;
          if constexpr (std::is_same_v<std::remove_cvref_t<decltype(TargetValue)>, BasicBlockOperand>)
          {
            Target = TargetValue.getBlock();
          }
          else
          {
            Target = TargetValue;
          }
          if (!Target || !InsertBlock->parent() || Target->parent() != InsertBlock->parent())
          {
            return nullptr;
          }
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
      [[noreturn]] static void invalidBranchTarget() noexcept;

      MachineBasicBlock *InsertBlock = nullptr;
      MachineBasicBlock::InstructionStorage::iterator InsertPosition;
  };
} // namespace ink::vm
