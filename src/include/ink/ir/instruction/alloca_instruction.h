#pragma once

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/reference_type.h"

namespace ink::ir
{
  // Models stack storage for one complete type; allocation legality and target layout are the caller's responsibility.
  // The result reference type and its referenced type must outlive this instruction.
  class AllocaInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Alloca;

      constexpr explicit AllocaInstruction(const ReferenceType &ResultType) noexcept
          : Instruction(Kind, ResultType)
      {
      }

      AllocaInstruction(const ReferenceType &&) = delete;

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Type &getAllocatedType() const noexcept
      {
        return static_cast<const ReferenceType &>(getType()).referencedType();
      }
  };
} // namespace ink::ir
