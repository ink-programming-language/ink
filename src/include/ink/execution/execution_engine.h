#ifndef INK_EXECUTION_EXECUTION_ENGINE_H
#define INK_EXECUTION_EXECUTION_ENGINE_H

#include "ink/core/diagnostic.h"
#include "ink/execution/context.h"
#include "ink/execution/runtime_value.h"
#include "ink/ir/module.h"

#include <memory>
#include <string_view>
#include <vector>

namespace ink::execution
{
  class InitializationResult
  {
  public:
    bool succeeded() const noexcept;
    const std::vector<core::Diagnostic> &diagnostics() const noexcept;

  private:
    bool Succeeded = false;
    std::vector<core::Diagnostic> Diagnostics;

    friend class ExecutionEngine;
  };

  class ExecutionResult
  {
  public:
    ExecutionResult() = default;
    ExecutionResult(const ExecutionResult &) = default;
    ExecutionResult &operator=(const ExecutionResult &) = default;
    ExecutionResult(ExecutionResult &&Other) noexcept;
    ExecutionResult &operator=(ExecutionResult &&Other) noexcept;

    bool succeeded() const noexcept;
    RuntimeValueRef returnValue() const & noexcept;
    RuntimeValueRef returnValue() const && = delete;
    const std::vector<core::Diagnostic> &diagnostics() const noexcept;

  private:
    std::shared_ptr<RuntimeValueArena> ValueArena;
    RuntimeValueRef ReturnValue = nullptr;
    std::vector<core::Diagnostic> Diagnostics;

    friend class ExecutionEngine;
  };

  class ExecutionEngine
  {
  public:
    ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue);
    ~ExecutionEngine();
    ExecutionEngine(const ExecutionEngine &) = delete;
    ExecutionEngine &operator=(const ExecutionEngine &) = delete;
    ExecutionEngine(ExecutionEngine &&) = delete;
    ExecutionEngine &operator=(ExecutionEngine &&) = delete;

    InitializationResult initialize();
    // Argument values are borrowed and must remain alive and immutable until this synchronous call returns.
    ExecutionResult execute(std::string_view EntryName, const std::vector<RuntimeValueRef> &Arguments = {});

  private:
    class Impl;
    std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
