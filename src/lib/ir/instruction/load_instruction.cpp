#include "ink/ir/instruction/load_instruction.h"

#include "ink/ir/type/type.h"

namespace ink::ir
{
  LoadInstruction::LoadInstruction(const Type &LoadedType, const Value &Pointer) noexcept
      : Instruction(ValueKind::LoadInstruction, &LoadedType),
        Pointer(&Pointer)
  {
  }

  const Value *LoadInstruction::pointer() const noexcept
  {
    return Pointer;
  }

  bool LoadInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::LoadInstruction;
  }
} // namespace ink::ir
