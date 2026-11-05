#pragma once

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/reference_type.h"

namespace ink::ir
{
  // Models stack storage for a type value, including an unbound generic parameter; target layout is resolved after specialization.
  // The caller must provide ReferenceType(T) as the result type; it and T must outlive this instruction.
  class AllocaInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Alloca;

      constexpr explicit AllocaInstruction(ValueRef ResultType) noexcept
          : Instruction(Kind, ResultType)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value &getAllocatedType() const noexcept
      {
        return static_cast<const ReferenceType &>(getType()).referencedType();
      }
  };
} // namespace ink::ir
