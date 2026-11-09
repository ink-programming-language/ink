#include "ink/ir/instruction/call_instruction.h"

#include "ink/core/support/core_define.h"

namespace ink::ir
{
  namespace
  {
    const FunctionType &calleeSignature(ValueRef Callee) noexcept
    {
      checkMsgf(FunctionType::classof(&Callee.get().getType()), "Call requires a function-typed callee");
      return static_cast<const FunctionType &>(Callee.get().getType());
    }
  } // namespace

  CallInstruction::CallInstruction(ValueRef Callee, std::span<const ValueRef> Arguments)
      : Instruction(Kind, calleeSignature(Callee).returnType()),
        Callee(Callee),
        Arguments(Arguments.begin(), Arguments.end())
  {
  }

  const Value &CallInstruction::argument(std::size_t Index) const noexcept
  {
    checkMsgf(Index < Arguments.size(), "Call argument index is out of range");
    return Arguments[Index].get();
  }
} // namespace ink::ir
