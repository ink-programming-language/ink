#pragma once

#include "ink/backend/vm/instruction/machine_instruction_visitor.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"

#include <functional>
#include <tuple>
#include <type_traits>

namespace ink::vm
{
  // Copies operands, then invokes bool Rewriter(ConcreteOperand &) in semantic order, stopping on false.
  // Rewriter may handle RegisterOperand, AddressOperand, ImmediateOperand and BasicBlockOperand independently.
  // Failure, including an invalid builder insertion point or branch target, returns nullptr without inserting code.
  // Input instructions stay unchanged. Rewriter side effects outside the operand copies are not rolled back.
  template <typename Rewriter>
  [[nodiscard]] MachineInstruction *tryCloneInstruction(const MachineInstruction &Instruction, MachineInstructionBuilder &Builder, Rewriter &&RewriterValue)
  {
    const auto Clone = [&](const auto &Concrete) -> MachineInstruction *
    {
      using InstructionType = std::remove_cvref_t<decltype(Concrete)>;
      auto Operands = Concrete.operands();
      const auto Rewrite = [&](auto &...Values)
      {
        return (static_cast<bool>(std::invoke(RewriterValue, Values)) && ...);
      };
      if (!std::apply(Rewrite, Operands))
      {
        return nullptr;
      }
      const auto Create = [&](const auto &...Values) -> MachineInstruction *
      {
        return Builder.tryCreate<InstructionType::Operation>(Values...);
      };
      return std::apply(Create, Operands);
    };
    return visitInstruction(Instruction, Clone);
  }
} // namespace ink::vm
