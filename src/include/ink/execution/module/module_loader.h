#ifndef INK_EXECUTION_MODULE_LOADER_H
#define INK_EXECUTION_MODULE_LOADER_H

#include "ink/execution/module/module_instance.h"
#include "ink/execution/module/module_provider.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace ink::execution
{
  constexpr std::size_t MaximumModuleImportDepth = 64;

  class ModuleLoader;

  class ModuleLifecycle
  {
    public:
      virtual ~ModuleLifecycle() = default;

      // Lifecycle callbacks run without loader or instance state locks held.
      virtual ModuleLoadError prepare(ModuleInstance &Instance) noexcept;
      virtual ModuleLoadError initialize(ModuleLoader &Loader, ModuleInstance &Instance) noexcept;
      virtual ModuleLoadError finalize(ModuleLoader &Loader, ModuleInstance &Instance) noexcept;
  };

  class ModuleLoadResult
  {
    public:
      bool succeeded() const noexcept;
      std::shared_ptr<ModuleInstance> instance() const noexcept;
      const ModuleLoadError &error() const noexcept;
      const std::vector<core::Diagnostic> &diagnostics() const noexcept;

    private:
      ModuleLoadResult(std::shared_ptr<ModuleInstance> Instance, ModuleLoadError Error, std::vector<core::Diagnostic> Diagnostics = {}) noexcept;

      std::shared_ptr<ModuleInstance> Instance;
      ModuleLoadError Error;
      std::vector<core::Diagnostic> Diagnostics;

      friend class ModuleLoader;
  };

  class ModuleLoader
  {
    public:
      ModuleLoader(ModuleProvider &Provider, ModuleLifecycle &Lifecycle, core::TargetContext Target);
      ~ModuleLoader() noexcept;
      ModuleLoader(const ModuleLoader &) = delete;
      ModuleLoader &operator=(const ModuleLoader &) = delete;
      ModuleLoader(ModuleLoader &&) = delete;
      ModuleLoader &operator=(ModuleLoader &&) = delete;

      ModuleLoadResult loadModule(const ir::Name &Target);
      // Imports are accepted while Importer is initializing or Ready. A Ready importer is not poisoned by a failed dynamic import.
      ModuleLoadResult importModule(ModuleInstance &Importer, const ir::Name &Target);
      std::shared_ptr<ModuleInstance> findModule(ModuleId Target) const noexcept;
      std::shared_ptr<ModuleInstance> findModule(const ir::Name &Target) const;
      ir::Name moduleName(ModuleId Target) const;
      std::vector<ModuleLoadError> shutdown();

    private:
      class Impl;
      class OperationGuard;

      bool beginOperation() noexcept;
      void endOperation() noexcept;
      ModuleLoadError shutdownImpl() noexcept;
      ModuleLoadResult loadModuleImpl(const ir::Name &TargetName, ModuleId Target, ModuleInstance *Importer);
      ModuleLoadResult resolveModule(const ir::Name &TargetName, ModuleId Target);
      ModuleLoadResult failedResult(ModuleLoadError Error, std::shared_ptr<ModuleInstance> Instance = nullptr, std::vector<core::Diagnostic> Diagnostics = {}) const noexcept;
      ModuleLoadResult successfulResult(std::shared_ptr<ModuleInstance> Instance) const noexcept;
      ModuleLoadError normalizeLifecycleError(ModuleLoadError Error, ModuleLoadErrorKind Fallback, ModuleId Module) const noexcept;

      std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
