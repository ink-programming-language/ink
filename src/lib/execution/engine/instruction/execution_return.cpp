#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/return_instruction.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::executeReturn(const ir::ReturnInstruction &Return, ExecutionFrame &Frame)
  {
    const ir::Value *ReturnedValue = Return.returnedValue();
    return ReturnedValue ? evaluate(*ReturnedValue, Frame) : ExecutionValueResult{ExecutionStatus::Success, Heap.voidValue(Return.type())};
  }
} // namespace ink::execution
