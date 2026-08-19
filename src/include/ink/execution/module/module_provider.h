#ifndef INK_EXECUTION_MODULE_PROVIDER_H
#define INK_EXECUTION_MODULE_PROVIDER_H

#include "ink/ir/model/name.h"

#include <cstdint>
#include <memory>
#include <utility>
namespace ink::ir
{
  class IRContext;
  class Module;
} // namespace ink::ir

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
      bool DiagnosticReported = false;

      static ModuleProvisionResult found(std::shared_ptr<const ir::Module> Module) noexcept
      {
        return {ModuleProvisionStatus::Found, std::move(Module), false};
      }

      static ModuleProvisionResult notFound() noexcept
      {
        return {};
      }

      static ModuleProvisionResult failure(bool DiagnosticReported = false) noexcept
      {
        return {ModuleProvisionStatus::Failed, nullptr, DiagnosticReported};
      }
  };

  class ModuleProvider
  {
    public:
      virtual ~ModuleProvider() = default;

      // The loader requests images lazily and retains the returned immutable image for the lifetime of its ModuleInstance.
      // A callback must not synchronously or indirectly reenter the same ModuleLoader.
      // The provider and its IR context must outlive every loader and engine that uses it.
      virtual ir::IRContext &irContext() noexcept = 0;
      // A non-empty image module name must match ModuleName and the image must use irContext().
      virtual ModuleProvisionResult provideModule(const ir::Name &ModuleName) noexcept = 0;
  };
} // namespace ink::execution

#endif
