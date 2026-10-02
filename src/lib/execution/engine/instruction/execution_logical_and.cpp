#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/logical_and_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeLogicalAnd(const ir::LogicalAndInstruction &And, ExecutionFrame &Frame)
  {
    const ExecutionValueResult Left = evaluate(And.left(), Frame);
    if (!Left)
    {
      return ExecutionInstructionResult::failure(Left.Status);
    }
    const ExecutionValueResult Right = evaluate(And.right(), Frame);
    if (!Right)
    {
      return ExecutionInstructionResult::failure(Right.Status);
    }
    if (Left.Value.kind() != ExecutionValueKind::Boolean || Right.Value.kind() != ExecutionValueKind::Boolean || Left.Value.type() != Right.Value.type())
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    return ExecutionInstructionResult::continueWith(Heap.boolean(And.type(), Left.Value.boolean() && Right.Value.boolean()));
  }
} // namespace ink::execution
