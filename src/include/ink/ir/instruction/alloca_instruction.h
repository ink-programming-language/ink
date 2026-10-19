#ifndef INK_IR_INSTRUCTION_ALLOCA_INSTRUCTION_H
#define INK_IR_INSTRUCTION_ALLOCA_INSTRUCTION_H

#include "ink/core/name/name.h"
#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  // Allocates one function-local object and represents that storage with its object type.
  // The allocated type is borrowed and must outlive this instruction.
  class AllocaInstruction final : public Instruction
  {
    public:
      explicit AllocaInstruction(const Type &AllocatedType, bool Mutable = true, bool Initialized = false) noexcept;

      const Type *allocatedType() const noexcept;
      bool isStorage() const noexcept override;
      bool isMutable() const noexcept override;
      // Records declaration initialization only; this is not a control-flow definite-initialization result.
      bool isInitialized() const noexcept override;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class IRBuilder;

      bool Mutable = true;
      bool Initialized = false;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
