#include "ink/ir/instruction/alloca_instruction.h"

namespace ink::ir
{
  AllocaInstruction::AllocaInstruction(const Type &AllocatedType, bool Mutable, bool Initialized) noexcept
      : Instruction(ValueKind::AllocaInstruction, &AllocatedType),
        Mutable(Mutable),
        Initialized(Initialized)
  {
  }

  const Type *AllocaInstruction::allocatedType() const noexcept
  {
    return type();
  }

  bool AllocaInstruction::isStorage() const noexcept
  {
    return true;
  }

  bool AllocaInstruction::isMutable() const noexcept
  {
    return Mutable;
  }

  bool AllocaInstruction::isInitialized() const noexcept
  {
    return Initialized;
  }

  core::Name AllocaInstruction::name() const noexcept
  {
    return Name;
  }

  bool AllocaInstruction::hasName() const noexcept
  {
    return Name.valid();
  }

  void AllocaInstruction::setName(core::Name NewName) noexcept
  {
    Name = NewName;
  }

  bool AllocaInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::AllocaInstruction;
  }
} // namespace ink::ir
