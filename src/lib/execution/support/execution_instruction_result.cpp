#include "ink/execution/support/execution_instruction_result.h"

#include <utility>

namespace ink::execution
{
  ExecutionInstructionResult::ExecutionInstructionResult(ExecutionInstructionAction Action, ExecutionStatus Status, ExecutionValueRef Value, const ir::BasicBlock *Target) noexcept
      : Action(Action),
        Status(Status),
        Value(std::move(Value)),
        Target(Target)
  {
  }

  ExecutionInstructionResult::ExecutionInstructionResult(ExecutionInstructionResult &&Other) noexcept
      : Action(std::exchange(Other.Action, ExecutionInstructionAction::Failure)),
        Status(std::exchange(Other.Status, ExecutionStatus::InvalidArguments)),
        Value(std::move(Other.Value)),
        Target(std::exchange(Other.Target, nullptr))
  {
  }

  ExecutionInstructionResult &ExecutionInstructionResult::operator=(ExecutionInstructionResult &&Other) noexcept
  {
    if (this != &Other)
    {
      Action = std::exchange(Other.Action, ExecutionInstructionAction::Failure);
      Status = std::exchange(Other.Status, ExecutionStatus::InvalidArguments);
      Value = std::move(Other.Value);
      Target = std::exchange(Other.Target, nullptr);
    }
    return *this;
  }

  ExecutionInstructionResult ExecutionInstructionResult::continueWith(ExecutionValueRef Value) noexcept
  {
    return Value ? ExecutionInstructionResult(ExecutionInstructionAction::Continue, ExecutionStatus::Success, std::move(Value), nullptr) : failure(ExecutionStatus::InvalidArguments);
  }

  ExecutionInstructionResult ExecutionInstructionResult::continueWith(ExecutionValueResult Result) noexcept
  {
    return Result.Status == ExecutionStatus::Success ? continueWith(std::move(Result.Value)) : failure(Result.Status);
  }

  ExecutionInstructionResult ExecutionInstructionResult::returnValue(ExecutionValueRef Value) noexcept
  {
    return Value ? ExecutionInstructionResult(ExecutionInstructionAction::Return, ExecutionStatus::Success, std::move(Value), nullptr) : failure(ExecutionStatus::InvalidArguments);
  }

  ExecutionInstructionResult ExecutionInstructionResult::returnValue(ExecutionValueResult Result) noexcept
  {
    return Result.Status == ExecutionStatus::Success ? returnValue(std::move(Result.Value)) : failure(Result.Status);
  }

  ExecutionInstructionResult ExecutionInstructionResult::jumpTo(const ir::BasicBlock &Target) noexcept
  {
    return ExecutionInstructionResult(ExecutionInstructionAction::Jump, ExecutionStatus::Success, {}, &Target);
  }

  ExecutionInstructionResult ExecutionInstructionResult::failure(ExecutionStatus Status) noexcept
  {
    return ExecutionInstructionResult(ExecutionInstructionAction::Failure, Status == ExecutionStatus::Success ? ExecutionStatus::InvalidArguments : Status, {}, nullptr);
  }
} // namespace ink::execution
