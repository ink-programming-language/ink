#pragma once

#include "ink/core/name/name.h"
#include "ink/backend/vm/instruction/machine_instruction.h"

#include <cstddef>
#include <list>
#include <memory>
#include <ranges>
#include <utility>

namespace ink::vm
{
  class MachineFunction;
  class MachineInstructionBuilder;

  // Owns instructions in insertion order. Insertion preserves existing instruction addresses and iterators.
  class MachineBasicBlock final
  {
    private:
      using InstructionOwner = std::unique_ptr<MachineInstruction, void (*)(MachineInstruction *) noexcept>;
      using InstructionStorage = std::list<InstructionOwner>;

    public:
      explicit MachineBasicBlock(core::Name Name = core::NAME_None) noexcept
          : Name(Name)
      {
      }

      MachineBasicBlock(const MachineBasicBlock &) = delete;
      MachineBasicBlock &operator=(const MachineBasicBlock &) = delete;
      MachineBasicBlock(MachineBasicBlock &&) = delete;
      MachineBasicBlock &operator=(MachineBasicBlock &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      std::size_t instructionCount() const noexcept
      {
        return Instructions.size();
      }

      // The block must outlive this borrowed view and all references obtained from it.
      auto instructions(this auto &&Self) noexcept
      {
        return Self.Instructions | std::views::transform([](const InstructionOwner &Owned) -> decltype(auto)
                                                         {
                                                           return std::forward_like<decltype(Self) &>(*Owned);
                                                         });
      }

    private:
      friend class MachineFunction;
      friend class MachineInstructionBuilder;

      core::Name Name;
      MachineFunction *Parent = nullptr;
      InstructionStorage Instructions;
  };
} // namespace ink::vm
