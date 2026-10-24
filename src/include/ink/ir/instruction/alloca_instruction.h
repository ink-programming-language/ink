#ifndef INK_IR_INSTRUCTION_ALLOCA_INSTRUCTION_H
#define INK_IR_INSTRUCTION_ALLOCA_INSTRUCTION_H

#include "ink/core/name/name.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/structure_type/pointer_type.h"

namespace ink::ir
{
  // Allocates one function-local object and produces its address as an ordinary pointer value.
  // The result pointer type and its pointee are borrowed and must outlive this instruction.
  class AllocaInstruction final : public Instruction
  {
    public:
      explicit AllocaInstruction(const PointerType &ResultType) noexcept;

      const PointerType *type() const noexcept override;
      const Type *allocatedType() const noexcept;
      // Records the allocated object's declaration initialization, not the pointer value's state or control-flow definite initialization.
      bool isInitialized() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class IRBuilder;

      bool Initialized = false;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
