#include "engine/function_executor.h"

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeReturnInstruction(const ir::ReturnInstruction &Return, FunctionExecutionState &State)
  {
    if (!Return.ReturnValue)
    {
      State.ReturnValue = Values.voidValue(*State.FunctionValue.ResultType);
    }
    else
    {
      State.ReturnValue = evaluateValue(*Return.ReturnValue, State.Module, State.Frame, State.FunctionValue.Name);
    }
    return State.ReturnValue == nullptr ? InstructionFlow::Failed : InstructionFlow::Return;
  }
} // namespace ink::execution
