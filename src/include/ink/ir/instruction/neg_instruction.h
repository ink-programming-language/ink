#ifndef INK_IR_INSTRUCTION_NEG_INSTRUCTION_H
#define INK_IR_INSTRUCTION_NEG_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  // Negates an integer or floating-point value and produces the same type.
  // Integer negation requires a representable result; overflow has no valid result.
  // Floating-point negation flips the sign bit, preserving signed zero and NaN payloads.
  // The operand and its result type are borrowed and must outlive the instruction.
  class NegInstruction final : public Instruction
  {
    public:
      explicit NegInstruction(const Value &Operand) noexcept;

      const Value *operand() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *Operand = nullptr;
  };
} // namespace ink::ir

#endif
