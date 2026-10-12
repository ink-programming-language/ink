#include "ink/ir/instruction/return_instruction.h"

#include "ink/ir/type/builtin_type/void_type.h"

namespace ink::ir
{
  ReturnInstruction::ReturnInstruction(const VoidType &ResultType, const Value *ReturnedValue) noexcept
      : Instruction(ValueKind::ReturnInstruction, &ResultType),
        ReturnedValue(ReturnedValue)
  {
  }

  const Value *ReturnInstruction::returnedValue() const noexcept
  {
    return ReturnedValue;
  }

  bool ReturnInstruction::hasReturnedValue() const noexcept
  {
    return ReturnedValue != nullptr;
  }

  bool ReturnInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::ReturnInstruction;
  }
} // namespace ink::ir
