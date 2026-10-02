#ifndef INK_EXECUTION_SUPPORT_EXECUTION_INSTRUCTION_RESULT_H
#define INK_EXECUTION_SUPPORT_EXECUTION_INSTRUCTION_RESULT_H

#include "ink/execution/value/execution_value.h"

namespace ink::ir
{
  class BasicBlock;
} // namespace ink::ir

namespace ink::execution
{
  enum class ExecutionInstructionAction
  {
    Continue,
    Jump,
    Return,
    Failure,
  };

  // Instruction results distinguish computed values from changes to control flow.
  // Jump targets borrow IR blocks; their owning function must outlive the result.
  class ExecutionInstructionResult final
  {
    public:
      ExecutionInstructionResult() noexcept = default;
      ExecutionInstructionResult(const ExecutionInstructionResult &) noexcept = default;
      ExecutionInstructionResult &operator=(const ExecutionInstructionResult &) noexcept = default;
      ExecutionInstructionResult(ExecutionInstructionResult &&Other) noexcept;
      ExecutionInstructionResult &operator=(ExecutionInstructionResult &&Other) noexcept;

      // Continue and Return require a nonempty payload, including an explicit void value.
      // Empty payloads and failure(Success) normalize to Failure with InvalidArguments.
      static ExecutionInstructionResult continueWith(ExecutionValueRef Value) noexcept;
      static ExecutionInstructionResult continueWith(ExecutionValueResult Result) noexcept;
      static ExecutionInstructionResult returnValue(ExecutionValueRef Value) noexcept;
      static ExecutionInstructionResult returnValue(ExecutionValueResult Result) noexcept;
      static ExecutionInstructionResult jumpTo(const ir::BasicBlock &Target) noexcept;
      static ExecutionInstructionResult failure(ExecutionStatus Status) noexcept;

      ExecutionInstructionAction action() const noexcept
      {
        return Action;
      }

      ExecutionStatus status() const noexcept
      {
        return Status;
      }

      const ExecutionValueRef &value() const noexcept
      {
        return Value;
      }

      const ir::BasicBlock *target() const noexcept
      {
        return Target;
      }

      bool succeeded() const noexcept
      {
        return Action != ExecutionInstructionAction::Failure;
      }

      explicit operator bool() const noexcept
      {
        return succeeded();
      }

    private:
      ExecutionInstructionResult(ExecutionInstructionAction Action, ExecutionStatus Status, ExecutionValueRef Value, const ir::BasicBlock *Target) noexcept;

      ExecutionInstructionAction Action = ExecutionInstructionAction::Failure;
      ExecutionStatus Status = ExecutionStatus::InvalidArguments;
      ExecutionValueRef Value;
      const ir::BasicBlock *Target = nullptr;
  };
} // namespace ink::execution

#endif
