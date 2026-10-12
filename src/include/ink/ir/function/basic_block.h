#ifndef INK_IR_FUNCTION_BASIC_BLOCK_H
#define INK_IR_FUNCTION_BASIC_BLOCK_H

#include "ink/core/name/name.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/value.h"

#include <cstddef>
#include <memory>
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
      Function *function() noexcept;
      const Function *function() const noexcept;

      const std::vector<std::unique_ptr<Instruction>> &instructions() const noexcept;
      std::size_t instructionCount() const noexcept;
      // Indexed access returns nullptr outside the owned instruction list.
      Instruction *instruction(std::size_t Index) noexcept;
      const Instruction *instruction(std::size_t Index) const noexcept;
      // Takes ownership; nullptr is rejected without changing the block.
      Instruction *appendInstruction(std::unique_ptr<Instruction> InstructionValue);
      static bool classof(const Value *Value) noexcept;

    private:
      friend class Function;

      core::Name Name = core::NAME_None;
      Function *Parent = nullptr;
      std::vector<std::unique_ptr<Instruction>> Instructions;
  };
} // namespace ink::ir

#endif
