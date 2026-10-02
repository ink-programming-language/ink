#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/return_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeReturn(const ir::ReturnInstruction &Return, ExecutionFrame &Frame)
  {
    const ir::Value *ReturnedValue = Return.returnedValue();
    return ReturnedValue ? ExecutionInstructionResult::returnValue(evaluate(*ReturnedValue, Frame)) : ExecutionInstructionResult::returnValue(Heap.voidValue(Return.type()));
  }
} // namespace ink::execution
