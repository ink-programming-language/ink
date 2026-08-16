#include "function_executor.h"

#include <memory>
#include <vector>

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeCallInstruction(const ir::CallInstruction &Call, FunctionExecutionState &State)
  {
    std::vector<RuntimeValueRef> CallArguments;
    CallArguments.reserve(Call.Arguments.size());
    for (const std::unique_ptr<ir::Value> &Argument : Call.Arguments)
    {
      RuntimeValueRef ArgumentValue = evaluateValue(*Argument, State.Frame);
      if (ArgumentValue == nullptr)
      {
        return InstructionFlow::Failed;
      }
      CallArguments.push_back(ArgumentValue);
    }

    RuntimeValueRef CallResult = nullptr;
    if (!executeFunction(Call.Callee.value(), CallArguments, State.Depth + 1, CallResult))
    {
      return InstructionFlow::Failed;
    }
    if (!Call.Result.has_value())
    {
      return InstructionFlow::Continue;
    }
    if (CallResult == nullptr)
    {
      addFailure<core::DiagnosticKind::CallResultMissing>(State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(*Call.Result, CallResult))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("call", State.FunctionValue.Name, Call.Result->value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }
} // namespace ink::execution
