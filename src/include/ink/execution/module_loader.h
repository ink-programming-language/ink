#ifndef INK_EXECUTION_MODULE_LOADER_H
#define INK_EXECUTION_MODULE_LOADER_H

#include "ink/execution/module_instance.h"
#include "ink/execution/module_provider.h"

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

  private:
    ModuleLoadResult(std::shared_ptr<ModuleInstance> Instance, ModuleLoadError Error) noexcept;

    std::shared_ptr<ModuleInstance> Instance;
    ModuleLoadError Error;

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

    ModuleLoadResult loadModule(ir::ModuleId Target);
    // Imports are accepted only while Importer is executing its module initializer.
    ModuleLoadResult importModule(ModuleInstance &Importer, ir::ModuleId Target);
    std::shared_ptr<ModuleInstance> findModule(ir::ModuleId Target) const noexcept;
    std::vector<ModuleLoadError> shutdown();

  private:
    class Impl;
    class OperationGuard;

    bool beginOperation() noexcept;
    void endOperation() noexcept;
    ModuleLoadError shutdownImpl() noexcept;
    ModuleLoadResult loadModuleImpl(ir::ModuleId Target, ModuleInstance *Importer);
    ModuleLoadResult resolveModule(ir::ModuleId Target);
    ModuleLoadResult failedResult(ModuleLoadError Error, std::shared_ptr<ModuleInstance> Instance = nullptr) const noexcept;
    ModuleLoadResult successfulResult(std::shared_ptr<ModuleInstance> Instance) const noexcept;
    ModuleLoadError normalizeLifecycleError(ModuleLoadError Error, ModuleLoadErrorKind Fallback, ir::ModuleId Module) const noexcept;

    std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
