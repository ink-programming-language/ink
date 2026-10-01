#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/add_instruction.h"

#include <utility>

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::executeAdd(const ir::AddInstruction &Add, ExecutionFrame &Frame)
  {
    ExecutionValueResult Left = evaluate(Add.left(), Frame);
    if (!Left)
    {
      return Left;
    }
    ExecutionValueResult Right = evaluate(Add.right(), Frame);
    if (!Right)
    {
      return Right;
    }
    if (Left.Value.kind() != ExecutionValueKind::Integer || Right.Value.kind() != ExecutionValueKind::Integer || Left.Value.type() != Right.Value.type())
    {
      return {ExecutionStatus::TypeMismatch};
    }
    ExecutionInteger Sum(Left.Value.integer().bitWidth());
    const ExecutionStatus Status = Left.Value.integer().add(Right.Value.integer(), Sum);
    return Status == ExecutionStatus::Success ? ExecutionValueResult{Status, Heap.integer(Add.type(), std::move(Sum))} : ExecutionValueResult{Status};
  }
} // namespace ink::execution
