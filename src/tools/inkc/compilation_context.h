#ifndef INK_TOOLS_INKC_COMPILATION_CONTEXT_H
#define INK_TOOLS_INKC_COMPILATION_CONTEXT_H

#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/source/source_manager.h"
#include "ink/target/target_context.h"

namespace ink::inkc
{
  // Owns the compiler tool's session resources; frontend libraries borrow individual services.
  class CompilationContext
  {
    public:
      CompilationContext()
          : Target(target::TargetContext::native())
      {
      }

      explicit CompilationContext(target::TargetContext Target)
          : Target(Target)
      {
      }

      CompilationContext(const CompilationContext &) = delete;
      CompilationContext &operator=(const CompilationContext &) = delete;
      CompilationContext(CompilationContext &&) = delete;
      CompilationContext &operator=(CompilationContext &&) = delete;

      const target::TargetContext &targetContext() const noexcept
      {
        return Target;
      }

      core::DiagnosticEngine &diagnosticEngine() noexcept
      {
        return Diagnostics;
      }

      const core::DiagnosticEngine &diagnosticEngine() const noexcept
      {
        return Diagnostics;
      }

      core::SourceManager &sourceManager() noexcept
      {
        return Sources;
      }

      const core::SourceManager &sourceManager() const noexcept
      {
        return Sources;
      }

    private:
      target::TargetContext Target;
      core::DiagnosticEngine Diagnostics;
      core::SourceManager Sources;
  };
} // namespace ink::inkc

#endif
