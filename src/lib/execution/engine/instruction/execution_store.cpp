#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/store_instruction.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::executeStore(const ir::StoreInstruction &Store, ExecutionFrame &Frame)
  {
    ExecutionValueResult Address = evaluate(Store.address(), Frame);
    if (!Address)
    {
      return Address;
    }
    ExecutionValueResult Value = evaluate(Store.storedValue(), Frame);
    if (!Value)
    {
      return Value;
    }
    const ExecutionStatus Status = storePointer(Address.Value, Value.Value);
    return Status == ExecutionStatus::Success ? ExecutionValueResult{Status, Heap.voidValue(Store.type())} : ExecutionValueResult{Status};
  }
} // namespace ink::execution
