#ifndef INK_IR_INSTRUCTION_ICMP_INSTRUCTION_H
#define INK_IR_INSTRUCTION_ICMP_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

#include <cstdint>

namespace ink::ir
{
  class BoolType;

  enum class ICmpPredicate : std::uint8_t
  {
    Equal,
    NotEqual,
    UnsignedGreaterThan,
    UnsignedGreaterEqual,
    UnsignedLessThan,
    UnsignedLessEqual,
    SignedGreaterThan,
    SignedGreaterEqual,
    SignedLessThan,
    SignedLessEqual,
  };

  // Compares two resolved operands of the same integer or pointer type and produces a bool.
  // Equality predicates also allow the module's null literal with a typed pointer supplying its concrete type.
  // Signedness belongs to the predicate; operands and the result type are borrowed.
  class ICmpInstruction final : public Instruction
  {
    public:
      ICmpInstruction(const BoolType &ResultType, ICmpPredicate Predicate, const Value &Left, const Value &Right) noexcept;

      ICmpPredicate predicate() const noexcept;
      const Value *left() const noexcept;
      const Value *right() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      ICmpPredicate Predicate = ICmpPredicate::Equal;
      const Value *Left = nullptr;
      const Value *Right = nullptr;
  };
} // namespace ink::ir

#endif
