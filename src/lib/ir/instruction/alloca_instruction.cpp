#include "ink/ir/instruction/alloca_instruction.h"

#include "ink/ir/type/structure_type/pointer_type.h"

namespace ink::ir
{
  AllocaInstruction::AllocaInstruction(const PointerType &ResultType, const Type &AllocatedType) noexcept
      : Instruction(ValueKind::AllocaInstruction, &ResultType),
        AllocatedType(&AllocatedType)
  {
  }

  const Type *AllocaInstruction::allocatedType() const noexcept
  {
    return AllocatedType;
  }

  bool AllocaInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::AllocaInstruction;
  }
} // namespace ink::ir
