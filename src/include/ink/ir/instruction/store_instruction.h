#ifndef INK_IR_INSTRUCTION_STORE_INSTRUCTION_H
#define INK_IR_INSTRUCTION_STORE_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  class VoidType;

  // Writes a value to an address with a known pointee type and produces no operand value.
  // The void result type and both operands are borrowed and must outlive this instruction.
  class StoreInstruction final : public Instruction
  {
    public:
      StoreInstruction(const VoidType &ResultType, const Value &StoredValue, const Value &Pointer) noexcept;

      const Value *storedValue() const noexcept;
      const Value *pointer() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *StoredValue = nullptr;
      const Value *Pointer = nullptr;
  };
} // namespace ink::ir

#endif
