#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/load_instruction.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::executeLoad(const ir::LoadInstruction &Load, ExecutionFrame &Frame)
  {
    ExecutionValueResult Address = evaluate(Load.address(), Frame);
    return Address ? loadPointer(Address.Value) : Address;
  }
} // namespace ink::execution
