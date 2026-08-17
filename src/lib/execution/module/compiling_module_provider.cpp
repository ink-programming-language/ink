#include "ink/execution/module/compiling_module_provider.h"

#include <utility>

namespace ink::execution
{
  CompilingModuleProvider::CompilingModuleProvider(ir::CompilationSession &Session) noexcept
      : Session(Session)
  {
  }

  ir::IRContext &CompilingModuleProvider::irContext() noexcept
  {
    return Session.irContext();
  }

  ModuleProvisionResult CompilingModuleProvider::provideModule(const ir::Name &ModuleName) noexcept
  {
    ir::ModuleCompilationResult Result = Session.getOrCompileModule(ModuleName);
    switch (Result.Status)
    {
    case ir::ModuleCompilationStatus::Found:
      return ModuleProvisionResult::found(std::move(Result.ModuleValue));
    case ir::ModuleCompilationStatus::NotFound:
      return ModuleProvisionResult::notFound();
    case ir::ModuleCompilationStatus::Failed:
      return ModuleProvisionResult::failure(std::move(Result.Diagnostics));
    }
    return ModuleProvisionResult::failure();
  }
} // namespace ink::execution
