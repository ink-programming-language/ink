#include "ink/ir/instruction/dereference_instruction.h"

#include "ink/ir/type/type.h"

namespace ink::ir
{
  DereferenceInstruction::DereferenceInstruction(const Type &ObjectType, const Value &Pointer) noexcept
      : Instruction(ValueKind::DereferenceInstruction, &ObjectType),
        Pointer(&Pointer)
  {
  }

  const Value *DereferenceInstruction::pointer() const noexcept
  {
    return Pointer;
  }

  bool DereferenceInstruction::isStorage() const noexcept
  {
    return true;
  }

  bool DereferenceInstruction::isMutable() const noexcept
  {
    return type()->isMutable();
  }

  bool DereferenceInstruction::isInitialized() const noexcept
  {
    return false;
  }

  bool DereferenceInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::DereferenceInstruction;
  }
} // namespace ink::ir
