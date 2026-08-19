#ifndef INK_EXECUTION_MODULE_INSTANCE_H
#define INK_EXECUTION_MODULE_INSTANCE_H

#include "ink/core/target_context.h"
#include "ink/execution/module/module_id.h"
#include "ink/execution/runtime/runtime_value.h"
#include "ink/ir/model/id.h"
#include "ink/ir/model/name.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace ink::ir
{
  class Module;
  class StringConstant;
} // namespace ink::ir

namespace ink::execution
{
  class FunctionExecutor;
  class ModuleLoader;

  enum class ModuleState : std::uint8_t
  {
    Created,
    Preparing,
    Initializing,
    Publishing,
    Ready,
    Failed,
    Finalizing,
    Stopped,
  };

  enum class ModuleLoadErrorKind : std::uint8_t
  {
    None,
    ModuleNotFound,
    ProviderFailure,
    ModuleIdentityMismatch,
    InvalidGlobalStorage,
    CircularImport,
    ImportDepthLimitExceeded,
    PreparationFailed,
    InitializationFailed,
    FinalizationFailed,
    InvalidImporter,
    InvalidState,
    LoaderStopped,
    ShutdownDuringInitialization,
    ShutdownDuringFinalization,
  };

  struct ModuleLoadError
  {
      ModuleLoadErrorKind Kind = ModuleLoadErrorKind::None;
      ModuleId Module;
      ModuleId RelatedModule;
      bool DiagnosticReported = false;

      bool failed() const noexcept
      {
        return Kind != ModuleLoadErrorKind::None;
      }

      static ModuleLoadError failure(ModuleLoadErrorKind Kind, ModuleId Module, ModuleId RelatedModule = {}, bool DiagnosticReported = false) noexcept
      {
        return {Kind, Module, RelatedModule, DiagnosticReported};
      }
  };

  class ModuleInstance
  {
    public:
      ~ModuleInstance();
      ModuleInstance(const ModuleInstance &) = delete;
      ModuleInstance &operator=(const ModuleInstance &) = delete;
      ModuleInstance(ModuleInstance &&) = delete;
      ModuleInstance &operator=(ModuleInstance &&) = delete;

      ModuleId id() const noexcept;
      const ir::Name &name() const noexcept;
      const ir::Module &definition() const noexcept;
      const core::TargetContext &targetContext() const noexcept;
      ModuleState state() const noexcept;
      ModuleLoadError failure() const noexcept;
      std::vector<ModuleId> activeDependencies() const;

    private:
      class Impl;

      ModuleInstance(const ModuleLoader &Owner, ModuleId Id, ir::Name Name, std::shared_ptr<const ir::Module> Definition, core::TargetContext Target);
      bool belongsTo(const ModuleLoader &Loader) const noexcept;
      bool acceptsImport() const noexcept;
      bool tryBeginPreparation() noexcept;
      ModuleLoadError prepareGlobalStorage() noexcept;
      ModuleLoadError beginInitialization() noexcept;
      ModuleLoadError completeLoading(ModuleLoadError Error) noexcept;
      void publishReady() noexcept;
      void waitUntilLoaded() const noexcept;
      void recordPendingFailure(ModuleLoadError Error) noexcept;
      bool addActiveDependency(ModuleId Dependency) noexcept;
      bool beginFinalization() noexcept;
      void completeFinalization(ModuleLoadError Error) noexcept;
      RuntimeValueRef byteConstantAddress(ir::ByteConstantId Constant) const noexcept;
      RuntimeValueRef stringConstantValue(const ir::StringConstant &Constant) noexcept;
      // Returned values are owned by this instance and keep one stable backing for the instance lifetime.
      RuntimeValueRef globalAddress(ir::GlobalId Global) const noexcept;
      RuntimeValueRef mutableGlobalAddress(ir::GlobalId Global) const noexcept;

      std::unique_ptr<Impl> Implementation;

      friend class FunctionExecutor;
      friend class ModuleLoader;
  };
} // namespace ink::execution

#endif
