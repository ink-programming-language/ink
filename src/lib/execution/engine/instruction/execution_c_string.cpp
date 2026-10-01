#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/c_string_instruction.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::executeCString(const ir::CStringInstruction &CString, ExecutionFrame &Frame)
  {
    const ExecutionStorageRef Buffer = Heap.allocateBuffer(CString.source().value());
    if (!Buffer.valid())
    {
      if (Heap.lastStatus() == ExecutionStatus::BudgetExceeded)
      {
        StopStatus = Heap.lastStatus();
      }
      return {Heap.lastStatus()};
    }
    Frame.Storage.push_back(Buffer);
    return {ExecutionStatus::Success, Heap.pointer(CString.type(), ExecutionPointer::fromBuffer(Buffer))};
  }
} // namespace ink::execution
