#ifndef INK_IR_INSTRUCTION_DEREFERENCE_INSTRUCTION_H
#define INK_IR_INSTRUCTION_DEREFERENCE_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  // Refers to an existing object through an evaluated pointer without allocating or reading memory.
  // The result is storage of ObjectType, not an ordinary pointer or a loaded object value.
  // The object type and pointer operand are borrowed and must outlive this instruction.
  class DereferenceInstruction final : public Instruction
  {
    public:
      DereferenceInstruction(const Type &ObjectType, const Value &Pointer) noexcept;

      const Value *pointer() const noexcept;
      bool isStorage() const noexcept override;
      // The referenced object's top-level type qualifier controls write permission.
      bool isMutable() const noexcept override;
      // No declaration initialization is recorded; false does not prove the target is uninitialized.
      bool isInitialized() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      const Value *Pointer = nullptr;
  };
} // namespace ink::ir

#endif
