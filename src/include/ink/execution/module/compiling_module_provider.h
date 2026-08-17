#ifndef INK_EXECUTION_COMPILING_MODULE_PROVIDER_H
#define INK_EXECUTION_COMPILING_MODULE_PROVIDER_H

#include "ink/execution/module/module_provider.h"
#include "ink/ir/compilation/compilation_session.h"

namespace ink::execution
{
  class CompilingModuleProvider final : public ModuleProvider
  {
    public:
      explicit CompilingModuleProvider(ir::CompilationSession &Session) noexcept;

      ir::IRContext &irContext() noexcept override;
      ModuleProvisionResult provideModule(const ir::Name &ModuleName) noexcept override;

    private:
      ir::CompilationSession &Session;
  };
} // namespace ink::execution

#endif
