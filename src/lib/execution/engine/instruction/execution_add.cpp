#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/add_instruction.h"

#include <utility>

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeAdd(const ir::AddInstruction &Add, ExecutionFrame &Frame)
  {
    ExecutionValueResult Left = evaluate(Add.left(), Frame);
    if (!Left)
    {
      return ExecutionInstructionResult::failure(Left.Status);
    }
    ExecutionValueResult Right = evaluate(Add.right(), Frame);
    if (!Right)
    {
      return ExecutionInstructionResult::failure(Right.Status);
    }
    if (Left.Value.kind() != ExecutionValueKind::Integer || Right.Value.kind() != ExecutionValueKind::Integer || Left.Value.type() != Right.Value.type())
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    ExecutionInteger Sum(Left.Value.integer().bitWidth());
    const ExecutionStatus Status = Left.Value.integer().add(Right.Value.integer(), Sum);
    return Status == ExecutionStatus::Success ? ExecutionInstructionResult::continueWith(Heap.integer(Add.type(), std::move(Sum))) : ExecutionInstructionResult::failure(Status);
  }
} // namespace ink::execution
