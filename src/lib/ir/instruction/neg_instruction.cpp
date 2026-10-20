#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  NegInstruction::NegInstruction(const Value &Operand) noexcept
      : Instruction(ValueKind::NegInstruction, Operand.type() ? Operand.type()->unqualifiedType() : nullptr),
        Operand(&Operand)
  {
  }

  const Value *NegInstruction::operand() const noexcept
  {
    return Operand;
  }

  bool NegInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::NegInstruction;
  }
} // namespace ink::ir
