#ifndef INK_CORE_CONTEXT_H
#define INK_CORE_CONTEXT_H

#include "ink/core/diagnostic.h"
#include "ink/core/source_id.h"
#include "ink/core/target_context.h"

#include <atomic>

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

      SourceId createSourceId() noexcept
      {
        return SourceId(NextSourceId.fetch_add(1, std::memory_order_relaxed));
      }

    private:
      TargetContext Target;
      DiagnosticEngine Diagnostics;
      std::atomic<std::uint64_t> NextSourceId{1};
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

    private:
      CompilationContext &Compilation;
  };
} // namespace ink::core

#endif
