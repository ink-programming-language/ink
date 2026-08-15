#ifndef INK_IR_CONTEXT_H
#define INK_IR_CONTEXT_H

#include "ink/core/context.h"

namespace ink::ir
{
  class IRContext
  {
  public:
    explicit IRContext(core::CompilationContext &Compilation) : Compilation(Compilation)
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

  private:
    core::CompilationContext &Compilation;
  };
} // namespace ink::ir

#endif
