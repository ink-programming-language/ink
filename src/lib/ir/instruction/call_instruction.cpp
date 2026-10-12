#include "ink/ir/instruction/call_instruction.h"

#include "ink/ir/function/function.h"

#include <utility>

namespace ink::ir
{
  CallInstruction::CallInstruction(const FunctionType &Signature, const Value &Callee, std::vector<Argument> Arguments) noexcept
      : Instruction(ValueKind::CallInstruction, Signature.returnType()),
        Signature(&Signature),
        Callee(&Callee),
        Arguments(std::move(Arguments))
  {
  }

  CallInstruction::CallInstruction(const Function &Callee, std::vector<Argument> Arguments) noexcept
      : CallInstruction(*Callee.functionType(), Callee, std::move(Arguments))
  {
  }

  const FunctionType *CallInstruction::functionType() const noexcept
  {
    return Signature;
  }

  const Value *CallInstruction::callee() const noexcept
  {
    return Callee;
  }

  const Function *CallInstruction::directCallee() const noexcept
  {
    return Function::classof(Callee) ? static_cast<const Function *>(Callee) : nullptr;
  }

  const std::vector<Argument> &CallInstruction::arguments() const noexcept
  {
    return Arguments;
  }

  std::size_t CallInstruction::argumentCount() const noexcept
  {
    return Arguments.size();
  }

  const Argument *CallInstruction::argument(std::size_t Index) const noexcept
  {
    return Index < Arguments.size() ? &Arguments[Index] : nullptr;
  }

  bool CallInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::CallInstruction;
  }
} // namespace ink::ir
