#ifndef INK_IR_INSTRUCTION_ADDRESS_OF_INSTRUCTION_H
#define INK_IR_INSTRUCTION_ADDRESS_OF_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  class PointerType;

  // Produces an ordinary pointer value for storage without reading the stored object.
  // The result type and storage operand are borrowed and must outlive this instruction.
  class AddressOfInstruction final : public Instruction
  {
    public:
      AddressOfInstruction(const PointerType &ResultType, const Value &Storage) noexcept;

      const Value *storage() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *Storage = nullptr;
  };
} // namespace ink::ir

#endif
