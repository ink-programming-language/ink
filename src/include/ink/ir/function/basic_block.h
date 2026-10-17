#ifndef INK_IR_FUNCTION_BASIC_BLOCK_H
#define INK_IR_FUNCTION_BASIC_BLOCK_H

#include "ink/core/name/name.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/value.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace ink::ir
{
  class Function;

  // Owns instructions and optionally belongs to a function; detached blocks have no parent.
  class BasicBlock final : public Value
  {
    public:
      explicit BasicBlock(core::Name Name = core::NAME_None) noexcept;
      ~BasicBlock() noexcept override;

      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;

      auto *function(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      const std::vector<std::unique_ptr<Instruction>> &instructions() const noexcept;
      std::size_t instructionCount() const noexcept;
      // Reports any terminator, including one followed by low-level appended instructions.
      bool hasTerminator() const noexcept;
      // Indexed access returns nullptr outside the owned instruction list.
      auto *instruction(this auto &&Self, std::size_t Index) noexcept
      {
        auto *Result = Index < Self.Instructions.size() ? Self.Instructions[Index].get() : nullptr;
        return Result ? &std::forward_like<decltype(Self) &>(*Result) : nullptr;
      }

      // Takes ownership and records the instruction's parent; nullptr leaves the block unchanged.
      Instruction *appendInstruction(std::unique_ptr<Instruction> InstructionValue);
      static bool classof(const Value *Value) noexcept;

    private:
      friend class Function;

      core::Name Name = core::NAME_None;
      Function *Parent = nullptr;
      std::vector<std::unique_ptr<Instruction>> Instructions;
      bool HasTerminator = false;
  };
} // namespace ink::ir

#endif
