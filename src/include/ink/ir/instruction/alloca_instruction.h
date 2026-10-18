#ifndef INK_IR_INSTRUCTION_ALLOCA_INSTRUCTION_H
#define INK_IR_INSTRUCTION_ALLOCA_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  class PointerType;

  // Allocates one uninitialized function-local object and produces its address.
  // The allocated and result types are borrowed and must outlive this instruction.
  class AllocaInstruction final : public Instruction
  {
    public:
      AllocaInstruction(const PointerType &ResultType, const Type &AllocatedType) noexcept;

      const Type *allocatedType() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Type *AllocatedType = nullptr;
  };
} // namespace ink::ir

#endif
