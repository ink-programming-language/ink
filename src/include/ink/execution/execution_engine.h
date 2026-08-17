#ifndef INK_EXECUTION_EXECUTION_ENGINE_H
#define INK_EXECUTION_EXECUTION_ENGINE_H

#include "ink/core/diagnostic.h"
#include "ink/execution/context.h"
#include "ink/execution/module/dynamic_module_load_result.h"
#include "ink/execution/module/module_provider.h"
#include "ink/execution/runtime/runtime_value.h"
#include "ink/ir/model/module.h"

#include <memory>
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

  class ShutdownResult
  {
    public:
      bool succeeded() const noexcept;
      const std::vector<core::Diagnostic> &diagnostics() const noexcept;

    private:
      bool Succeeded = false;
      std::vector<core::Diagnostic> Diagnostics;

      friend class ExecutionEngine;
  };

  class ExecutionEngine
  {
    public:
      // Calls on one engine instance must be externally serialized.
      ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue);
      // Provider images are loaded by canonical module name on reached imports and must share one IRContext and target within this engine.
      ExecutionEngine(ExecutionContext &Context, ModuleProvider &Provider, ir::Name EntryModule);
      ~ExecutionEngine();
      ExecutionEngine(const ExecutionEngine &) = delete;
      ExecutionEngine &operator=(const ExecutionEngine &) = delete;
      ExecutionEngine(ExecutionEngine &&) = delete;
      ExecutionEngine &operator=(ExecutionEngine &&) = delete;

      InitializationResult initialize();
      // Loads and initializes an arbitrary canonical module name. Successful modules remain alive until engine shutdown.
      DynamicModuleLoadResult loadModule(const ir::Name &ModuleName);
      // Argument values are borrowed and must remain alive and immutable until this synchronous call returns.
      ExecutionResult execute(const ir::Name &EntryName, const std::vector<RuntimeValueRef> &Arguments = {});
      ShutdownResult shutdown();

    private:
      class Impl;
      std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
