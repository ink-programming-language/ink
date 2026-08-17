#ifndef INK_EXECUTION_MODULE_PROVIDER_H
#define INK_EXECUTION_MODULE_PROVIDER_H

#include "ink/ir/id.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace ink::ir
{
  class Module;
}

namespace ink::execution
{
  enum class ModuleProvisionStatus : std::uint8_t
  {
    Found,
    NotFound,
    Failed,
  };

  struct ModuleProvisionResult
  {
    ModuleProvisionStatus Status = ModuleProvisionStatus::NotFound;
    std::shared_ptr<const ir::Module> Module;

    static ModuleProvisionResult found(std::shared_ptr<const ir::Module> Module) noexcept
    {
      return {ModuleProvisionStatus::Found, std::move(Module)};
    }

    static ModuleProvisionResult notFound() noexcept
    {
      return {};
    }

    static ModuleProvisionResult failure() noexcept
    {
      return {ModuleProvisionStatus::Failed, nullptr};
    }
  };

  class ModuleProvider
  {
  public:
    virtual ~ModuleProvider() = default;

    // The loader requests images lazily and retains the returned immutable image for the lifetime of its ModuleInstance.
    // A callback must not synchronously or indirectly reenter the same ModuleLoader.
    // A valid image module id must match Module. Images loaded by one ExecutionEngine must share one IRContext and target.
    virtual ModuleProvisionResult provideModule(ir::ModuleId Module) noexcept = 0;
  };
} // namespace ink::execution

#endif
