#ifndef INK_EXECUTION_BYTECODE_EXECUTION_COMPILER_H
#define INK_EXECUTION_BYTECODE_EXECUTION_COMPILER_H

#include "ink/execution/bytecode/executable_function.h"
#include "ink/execution/support/execution_result.h"

#include <memory>

namespace ink::ir
{
  class Function;
} // namespace ink::ir

namespace ink::execution
{
  class SemanticValueBridge;

  struct ExecutionCompilationResult
  {
      ExecutionStatus Status = ExecutionStatus::InvalidArguments;
      std::unique_ptr<ExecutableFunction> Function;

      explicit operator bool() const noexcept
      {
        return Status == ExecutionStatus::Success && Function != nullptr;
      }
  };

  class ExecutionCompiler final
  {
    public:
      ExecutionCompilationResult compile(const ir::Function &Function, SemanticValueBridge &Bridge) const;
      ExecutionStatus verify(const ExecutableFunction &Function) const noexcept;
  };
} // namespace ink::execution

#endif
