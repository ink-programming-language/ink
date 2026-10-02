#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/alloca_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeAlloca(const ir::AllocaInstruction &Alloca, ExecutionFrame &Frame)
  {
    // A back edge can execute this allocation again. Earlier places stay alive
    // until the invocation ends, while the instruction names its latest result.
    Frame.Bindings.erase(&Alloca);
    const ExecutionPlaceResult Result = allocateValue(Frame, &Alloca, Alloca.allocatedType());
    return Result ? ExecutionInstructionResult::continueWith(Heap.pointer(Alloca.type(), ExecutionPointer::fromPlace(Result.Place))) : ExecutionInstructionResult::failure(Result.Status);
  }
} // namespace ink::execution
