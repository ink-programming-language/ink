#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/store_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeStore(const ir::StoreInstruction &Store, ExecutionFrame &Frame)
  {
    ExecutionValueResult Address = evaluate(Store.address(), Frame);
    if (!Address)
    {
      return ExecutionInstructionResult::failure(Address.Status);
    }
    ExecutionValueResult Value = evaluate(Store.storedValue(), Frame);
    if (!Value)
    {
      return ExecutionInstructionResult::failure(Value.Status);
    }
    const ExecutionStatus Status = storePointer(Address.Value, Value.Value);
    return Status == ExecutionStatus::Success ? ExecutionInstructionResult::continueWith(Heap.voidValue(Store.type())) : ExecutionInstructionResult::failure(Status);
  }
} // namespace ink::execution
