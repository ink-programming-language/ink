#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  Instruction::Instruction() noexcept
      : Value(ValueKind::Instruction)
  {
  }

  Instruction::~Instruction() noexcept = default;

  bool Instruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Instruction;
  }
} // namespace ink::ir
