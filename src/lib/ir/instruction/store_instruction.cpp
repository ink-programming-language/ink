#include "ink/ir/instruction/store_instruction.h"

#include "ink/ir/type/builtin_type/void_type.h"

namespace ink::ir
{
  StoreInstruction::StoreInstruction(const VoidType &ResultType, const Value &StoredValue, const Value &Pointer) noexcept
      : Instruction(ValueKind::StoreInstruction, &ResultType),
        StoredValue(&StoredValue),
        Pointer(&Pointer)
  {
  }

  const Value *StoreInstruction::storedValue() const noexcept
  {
    return StoredValue;
  }

  const Value *StoreInstruction::pointer() const noexcept
  {
    return Pointer;
  }

  bool StoreInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::StoreInstruction;
  }
} // namespace ink::ir
