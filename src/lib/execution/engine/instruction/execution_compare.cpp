#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/compare_instruction.h"
#include "ink/ir/type/integer_type.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeCompare(const ir::CompareInstruction &Compare, ExecutionFrame &Frame)
  {
    const ExecutionValueResult Left = evaluate(Compare.left(), Frame);
    if (!Left)
    {
      return ExecutionInstructionResult::failure(Left.Status);
    }
    const ExecutionValueResult Right = evaluate(Compare.right(), Frame);
    if (!Right)
    {
      return ExecutionInstructionResult::failure(Right.Status);
    }
    if (Left.Value.kind() != Right.Value.kind() || Left.Value.type() != Right.Value.type())
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    int Ordering = 0;
    if (Left.Value.kind() == ExecutionValueKind::Integer)
    {
      const bool Signed = static_cast<const ir::IntegerType &>(*Left.Value.type()).isSigned();
      const ExecutionStatus Status = Left.Value.integer().compare(Right.Value.integer(), Signed, Ordering);
      if (Status != ExecutionStatus::Success)
      {
        return ExecutionInstructionResult::failure(Status);
      }
    }
    else if (Left.Value.kind() == ExecutionValueKind::Boolean && (Compare.predicate() == ir::ComparisonPredicate::Equal || Compare.predicate() == ir::ComparisonPredicate::NotEqual))
    {
      Ordering = Left.Value.boolean() == Right.Value.boolean() ? 0 : 1;
    }
    else
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    bool Result = false;
    switch (Compare.predicate())
    {
    case ir::ComparisonPredicate::Equal:
      Result = Ordering == 0;
      break;
    case ir::ComparisonPredicate::NotEqual:
      Result = Ordering != 0;
      break;
    case ir::ComparisonPredicate::Less:
      Result = Ordering < 0;
      break;
    case ir::ComparisonPredicate::LessEqual:
      Result = Ordering <= 0;
      break;
    case ir::ComparisonPredicate::Greater:
      Result = Ordering > 0;
      break;
    case ir::ComparisonPredicate::GreaterEqual:
      Result = Ordering >= 0;
      break;
    default:
      return ExecutionInstructionResult::failure(ExecutionStatus::UnsupportedOperation);
    }
    return ExecutionInstructionResult::continueWith(Heap.boolean(Compare.type(), Result));
  }
} // namespace ink::execution
