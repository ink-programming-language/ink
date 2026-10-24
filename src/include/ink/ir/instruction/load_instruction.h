#ifndef INK_IR_INSTRUCTION_LOAD_INSTRUCTION_H
#define INK_IR_INSTRUCTION_LOAD_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  // Reads a value through a pointer with a known pointee type.
  // The result type and pointer operand are borrowed and must outlive this instruction.
  class LoadInstruction final : public Instruction
  {
    public:
      LoadInstruction(const Type &LoadedType, const Value &Pointer) noexcept;

      const Value *pointer() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *Pointer = nullptr;
  };
} // namespace ink::ir

#endif
