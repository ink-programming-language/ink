#include "function_executor.h"

#include <optional>

namespace ink::execution
{
  bool FunctionExecutor::selectBlockTarget(const ir::BlockTarget &Target, FunctionExecutionState &State)
  {
    if (!Target.Block.valid() || Target.Block.value() == 0 || Target.Block.value() >= State.FunctionValue.Blocks.size())
    {
      addFailure<core::DiagnosticKind::InvalidBranchTargetDuringExecution>(State.FunctionValue.Name);
      return false;
    }

    State.PreviousBlock = State.NextBlock;
    State.NextBlock = Target.Block;
    return true;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeBranchInstruction(const ir::BranchInstruction &Branch, FunctionExecutionState &State)
  {
    return selectBlockTarget(Branch.Target, State) ? InstructionFlow::Branch : InstructionFlow::Failed;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeConditionalBranchInstruction(const ir::ConditionalBranchInstruction &Branch, FunctionExecutionState &State)
  {
    if (!Branch.Condition)
    {
      addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
      return InstructionFlow::Failed;
    }
    RuntimeValueRef Condition = evaluateValue(*Branch.Condition, State.Frame, State.FunctionValue.Name);
    if (Condition == nullptr)
    {
      return InstructionFlow::Failed;
    }
    const std::optional<std::uint64_t> Integer = Condition->integer();
    if (Condition->type().kind() != ir::TypeKind::Bool || !Integer.has_value() || *Integer > 1)
    {
      addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
      return InstructionFlow::Failed;
    }
    const ir::BlockTarget &Target = *Integer == 0 ? Branch.FalseTarget : Branch.TrueTarget;
    return selectBlockTarget(Target, State) ? InstructionFlow::Branch : InstructionFlow::Failed;
  }
} // namespace ink::execution
