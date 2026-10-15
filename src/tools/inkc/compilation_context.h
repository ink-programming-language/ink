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
          : Target(target::TargetContext::native()),
            Diagnostics(Sources)
      {
      }

      explicit CompilationContext(target::TargetContext Target)
          : Target(Target),
            Diagnostics(Sources)
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

      auto &diagnosticEngine(this auto &&Self) noexcept
      {
        return Self.Diagnostics;
      }

      auto &sourceManager(this auto &&Self) noexcept
      {
        return Self.Sources;
      }

    private:
      target::TargetContext Target;
      core::SourceManager Sources;
      core::DiagnosticEngine Diagnostics;
  };
} // namespace ink::inkc

#endif
