#ifndef INK_EXECUTION_CONTEXT_H
#define INK_EXECUTION_CONTEXT_H

#include "ink/core/context.h"
#include "ink/execution/runtime/native_symbol_registry.h"

namespace ink::execution
{
  class ExecutionContext
  {
    public:
      explicit ExecutionContext(core::CompilationContext &Compilation)
          : Compilation(Compilation)
      {
      }

      core::CompilationContext &compilationContext() noexcept
      {
        return Compilation;
      }

      const core::CompilationContext &compilationContext() const noexcept
      {
        return Compilation;
      }

      core::DiagnosticEngine &diagnosticEngine() noexcept
      {
        return Compilation.diagnosticEngine();
      }

      const core::DiagnosticEngine &diagnosticEngine() const noexcept
      {
        return Compilation.diagnosticEngine();
      }

      NativeSymbolRegistry &nativeSymbols() noexcept
      {
        return Symbols;
      }

      const NativeSymbolRegistry &nativeSymbols() const noexcept
      {
        return Symbols;
      }

    private:
      core::CompilationContext &Compilation;
      NativeSymbolRegistry Symbols;
  };
} // namespace ink::execution

#endif
