#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/alloca_instruction.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::executeAlloca(const ir::AllocaInstruction &Alloca, ExecutionFrame &Frame)
  {
    const ExecutionPlaceResult Result = allocateValue(Frame, &Alloca, Alloca.allocatedType());
    return Result ? ExecutionValueResult{ExecutionStatus::Success, Heap.pointer(Alloca.type(), ExecutionPointer::fromPlace(Result.Place))} : ExecutionValueResult{Result.Status};
  }
} // namespace ink::execution
