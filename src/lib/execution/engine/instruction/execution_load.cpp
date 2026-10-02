#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/load_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeLoad(const ir::LoadInstruction &Load, ExecutionFrame &Frame)
  {
    ExecutionValueResult Address = evaluate(Load.address(), Frame);
    return Address ? ExecutionInstructionResult::continueWith(loadPointer(Address.Value)) : ExecutionInstructionResult::failure(Address.Status);
  }
} // namespace ink::execution
