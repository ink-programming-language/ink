#include "ink/ir/instruction/icmp_instruction.h"

#include "ink/ir/type/builtin_type/bool_type.h"

namespace ink::ir
{
  ICmpInstruction::ICmpInstruction(const BoolType &ResultType, ICmpPredicate Predicate, const Value &Left, const Value &Right) noexcept
      : Instruction(ValueKind::ICmpInstruction, &ResultType),
        Predicate(Predicate),
        Left(&Left),
        Right(&Right)
  {
  }

  ICmpPredicate ICmpInstruction::predicate() const noexcept
  {
    return Predicate;
  }

  const Value *ICmpInstruction::left() const noexcept
  {
    return Left;
  }

  const Value *ICmpInstruction::right() const noexcept
  {
    return Right;
  }

  bool ICmpInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::ICmpInstruction;
  }
} // namespace ink::ir
