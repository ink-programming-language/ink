#pragma once

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/function_type.h"

#include <cstddef>
#include <span>
#include <vector>

namespace ink::ir
{
  // Copies the argument reference list and borrows the callee, arguments, and their types during use.
  // The callee must have FunctionType; arguments must match its signature, including an explicit first receiver for instance methods.
  class CallInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::Call;

      explicit CallInstruction(ValueRef Callee, std::span<const ValueRef> Arguments = {});

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      const Value &getCallee() const noexcept
      {
        return Callee.get();
      }

      const FunctionType &signature() const noexcept
      {
        return static_cast<const FunctionType &>(getCallee().getType());
      }

      std::span<const ValueRef> arguments() const noexcept
      {
        return Arguments;
      }

      std::size_t argumentCount() const noexcept
      {
        return Arguments.size();
      }

      const Value &argument(std::size_t Index) const noexcept;

    private:
      friend struct detail::InstructionOperands<CallInstruction>;

      ValueRef Callee;
      std::vector<ValueRef> Arguments = {};
  };
} // namespace ink::ir
