#pragma once

#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emitter.h"

namespace ink::vm
{
  class MachineModule;

  class ExecutableEmitter
  {
    public:
      // The borrowed diagnostic engine must outlive this emitter and its context.
      explicit ExecutableEmitter(const core::DiagnosticEngine &Diagnostics) noexcept;

      ExecutableEmitter(const ExecutableEmitter &) = delete;
      ExecutableEmitter &operator=(const ExecutableEmitter &) = delete;
      ExecutableEmitter(ExecutableEmitter &&) = delete;
      ExecutableEmitter &operator=(ExecutableEmitter &&) = delete;

      [[nodiscard]] ExecutableFile emit(const MachineModule &Module);

      // Borrows the current unbound image; further emission may change its contents.
      const ExecutableFile &getExecutable() const noexcept
      {
        return Context.getExecutable();
      }

      auto &getContext(this auto &&Self) noexcept
      {
        return Self.Context;
      }

      // Rebuilds the symbol map from all functions and globals, using zero offsets until image layout is known.
      void scanUnResolvedSymbol(const MachineModule &Module);

      // Appends aligned global storage and resolves each global to its image offset.
      void padGlobals(const MachineModule &Module);

      // Invokes the function emitter for each function in module order.
      void padFunction(const MachineModule &Module);

      // Finalizes forward references after all globals and functions are laid out; every referenced VM symbol must have an image offset.
      void resolveStaticRelocations();

    private:
      ExecutableEmissionContext Context;
      vm::FunctionEmitter FunctionEmitter;
  };
} // namespace ink::vm
