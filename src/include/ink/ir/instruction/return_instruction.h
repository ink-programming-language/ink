#ifndef INK_IR_INSTRUCTION_RETURN_INSTRUCTION_H
#define INK_IR_INSTRUCTION_RETURN_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  class VoidType;

  // Borrows its void result type and optional returned value; both must outlive this instruction.
  // The IR producer must match the returned value, or its absence, to the enclosing function signature.
  class ReturnInstruction final : public Instruction
  {
    public:
      explicit ReturnInstruction(const VoidType &ResultType, const Value *ReturnedValue = nullptr) noexcept;

      const Value *returnedValue() const noexcept;
      bool hasReturnedValue() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *ReturnedValue = nullptr;
  };
} // namespace ink::ir

#endif
