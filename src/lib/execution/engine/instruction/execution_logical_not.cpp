#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/logical_not_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeLogicalNot(const ir::LogicalNotInstruction &Not, ExecutionFrame &Frame)
  {
    const ExecutionValueResult Operand = evaluate(Not.operand(), Frame);
    if (!Operand)
    {
      return ExecutionInstructionResult::failure(Operand.Status);
    }
    if (Operand.Value.kind() != ExecutionValueKind::Boolean)
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    return ExecutionInstructionResult::continueWith(Heap.boolean(Not.type(), !Operand.Value.boolean()));
  }
} // namespace ink::execution
