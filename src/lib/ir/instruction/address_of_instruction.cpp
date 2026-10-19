#include "ink/ir/instruction/address_of_instruction.h"

#include "ink/ir/type/structure_type/pointer_type.h"

namespace ink::ir
{
  AddressOfInstruction::AddressOfInstruction(const PointerType &ResultType, const Value &Storage) noexcept
      : Instruction(ValueKind::AddressOfInstruction, &ResultType),
        Storage(&Storage)
  {
  }

  const Value *AddressOfInstruction::storage() const noexcept
  {
    return Storage;
  }

  bool AddressOfInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::AddressOfInstruction;
  }
} // namespace ink::ir
