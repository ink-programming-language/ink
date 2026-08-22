#ifndef INK_CORE_CONTEXT_H
#define INK_CORE_CONTEXT_H

#include "ink/core/diagnostic.h"
#include "ink/core/source_manager.h"
#include "ink/core/target_context.h"

namespace ink::core
{
  class CompilationContext
  {
    public:
      CompilationContext()
          : Target(TargetContext::native())
      {
      }

      explicit CompilationContext(TargetContext Target)
          : Target(Target)
      {
      }

      CompilationContext(const CompilationContext &) = delete;
      CompilationContext &operator=(const CompilationContext &) = delete;
      CompilationContext(CompilationContext &&) = delete;
      CompilationContext &operator=(CompilationContext &&) = delete;

      const TargetContext &targetContext() const noexcept
      {
        return Target;
      }

      DiagnosticEngine &diagnosticEngine() noexcept
      {
        return Diagnostics;
      }

      const DiagnosticEngine &diagnosticEngine() const noexcept
      {
        return Diagnostics;
      }

      SourceManager &sourceManager() noexcept
      {
        return Sources;
      }

      const SourceManager &sourceManager() const noexcept
      {
        return Sources;
      }

    private:
      TargetContext Target;
      DiagnosticEngine Diagnostics;
      SourceManager Sources;
  };

  class FrontendContext
  {
    public:
      explicit FrontendContext(CompilationContext &Compilation)
          : Compilation(Compilation)
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

      SourceManager &sourceManager() noexcept
      {
        return Compilation.sourceManager();
      }

      const SourceManager &sourceManager() const noexcept
      {
        return Compilation.sourceManager();
      }

    private:
      CompilationContext &Compilation;
  };
} // namespace ink::core

#endif
