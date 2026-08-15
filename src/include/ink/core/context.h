#ifndef INK_CORE_CONTEXT_H
#define INK_CORE_CONTEXT_H

#include "ink/core/diagnostic.h"

namespace ink::core
{
  class CompilationContext
  {
  public:
    CompilationContext() = default;
    CompilationContext(const CompilationContext &) = delete;
    CompilationContext &operator=(const CompilationContext &) = delete;
    CompilationContext(CompilationContext &&) = delete;
    CompilationContext &operator=(CompilationContext &&) = delete;

    DiagnosticEngine &diagnosticEngine() noexcept
    {
      return Diagnostics;
    }

    const DiagnosticEngine &diagnosticEngine() const noexcept
    {
      return Diagnostics;
    }

  private:
    DiagnosticEngine Diagnostics;
  };

  class FrontendContext
  {
  public:
    explicit FrontendContext(CompilationContext &Compilation) : Compilation(Compilation)
    {
    }

    CompilationContext &compilationContext() noexcept
    {
      return Compilation;
    }

    const CompilationContext &compilationContext() const noexcept
    {
      return Compilation;
    }

    DiagnosticEngine &diagnosticEngine() noexcept
    {
      return Compilation.diagnosticEngine();
    }

    const DiagnosticEngine &diagnosticEngine() const noexcept
    {
      return Compilation.diagnosticEngine();
    }

  private:
    CompilationContext &Compilation;
  };
} // namespace ink::core

#endif
