#include "ink/ir/instruction/alloca_instruction.h"

#include "ink/ir/type/type.h"

namespace ink::ir
{
  AllocaInstruction::AllocaInstruction(const PointerType &ResultType) noexcept
      : Instruction(ValueKind::AllocaInstruction, &ResultType)
  {
  }

  const PointerType *AllocaInstruction::type() const noexcept
  {
    return static_cast<const PointerType *>(Instruction::type());
  }

  const Type *AllocaInstruction::allocatedType() const noexcept
  {
    return type()->pointeeType();
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
