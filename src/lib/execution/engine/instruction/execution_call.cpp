#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/call_instruction.h"

#include <utility>
#include <vector>

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeCall(const ir::CallInstruction &Call, ExecutionFrame &Frame)
  {
    ExecutionValueResult Callee = evaluate(Call.callee(), Frame);
    if (!Callee)
    {
      return ExecutionInstructionResult::failure(Callee.Status);
    }
    if (Callee.Value.kind() != ExecutionValueKind::Function || !Callee.Value.valid())
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    std::vector<ExecutionValueRef> Arguments;
    Arguments.reserve(Call.arguments().size());
    for (const ir::Value *Argument : Call.arguments())
    {
      ExecutionValueResult Result = evaluate(*Argument, Frame);
      if (!Result)
      {
        return ExecutionInstructionResult::failure(Result.Status);
      }
      Arguments.push_back(std::move(Result.Value));
    }
    return ExecutionInstructionResult::continueWith(execute(*Callee.Value.function(), Arguments));
  }
} // namespace ink::execution
