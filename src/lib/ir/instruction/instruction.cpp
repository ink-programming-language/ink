#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  Instruction::Instruction(ValueKind Kind, const Type *ResultType) noexcept
      : Value(Kind),
        ResultType(ResultType)
  {
  }

  Instruction::~Instruction() noexcept = default;

  const Type *Instruction::type() const noexcept
  {
    return ResultType;
  }

  bool Instruction::isTerminator() const noexcept
  {
    return kind() == ValueKind::BranchInstruction || kind() == ValueKind::ConditionalBranchInstruction || kind() == ValueKind::ReturnInstruction;
  }

  bool Instruction::classof(const Value *Value) noexcept
  {
    if (!Value)
    {
      return false;
    }
    switch (Value->kind())
    {
    case ValueKind::Instruction:
    case ValueKind::GetElementPtrInstruction:
    case ValueKind::CallInstruction:
    case ValueKind::ICmpInstruction:
    case ValueKind::NegInstruction:
    case ValueKind::AllocaInstruction:
    case ValueKind::LoadInstruction:
    case ValueKind::StoreInstruction:
    case ValueKind::BranchInstruction:
    case ValueKind::ConditionalBranchInstruction:
    case ValueKind::ReturnInstruction:
      return true;
    default:
      return false;
    }
  }
} // namespace ink::ir
