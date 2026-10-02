#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/conditional_branch_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeConditionalBranch(const ir::ConditionalBranchInstruction &Branch, ExecutionFrame &Frame)
  {
    const ExecutionValueResult Condition = evaluate(Branch.condition(), Frame);
    if (!Condition)
    {
      return ExecutionInstructionResult::failure(Condition.Status);
    }
    if (Condition.Value.kind() != ExecutionValueKind::Boolean)
    {
      return ExecutionInstructionResult::failure(ExecutionStatus::TypeMismatch);
    }
    return ExecutionInstructionResult::jumpTo(Condition.Value.boolean() ? Branch.trueTarget() : Branch.falseTarget());
  }
} // namespace ink::execution
