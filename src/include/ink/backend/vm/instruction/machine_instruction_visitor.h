#pragma once

#include "ink/backend/vm/instruction/machine_instructions.h"

#include <functional>
#include <type_traits>
#include <utility>

namespace ink::vm
{
  // Invokes Visitor once with the concrete const instruction type. Every overload must return the same type.
  // Unknown opcodes do not invoke Visitor and return a value-initialized result, or return normally for void.
  // Value results must be default-constructible; reference results are not supported.
  template <typename Visitor>
  constexpr auto visitInstruction(const MachineInstruction &Instruction, Visitor &&VisitorValue) -> std::invoke_result_t<Visitor, const MachineRetInstruction &>
  {
    using Result = std::invoke_result_t<Visitor, const MachineRetInstruction &>;
    static_assert(std::is_void_v<Result> || std::is_default_constructible_v<Result>, "instruction visitors must return void or a default-constructible value");
    switch (Instruction.getOpcode())
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition)                                                                    \
  case Opcode::Kind:                                                                                                                                                                       \
    static_assert(std::is_same_v<Result, std::invoke_result_t<Visitor, const MachineInstructionImpl<Opcode::Kind> &>>, "instruction visitors must return the same type for every opcode"); \
    return std::invoke(std::forward<Visitor>(VisitorValue), static_cast<const MachineInstructionImpl<Opcode::Kind> &>(Instruction));
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    }
    if constexpr (!std::is_void_v<Result>)
    {
      return Result{};
    }
  }
} // namespace ink::vm
