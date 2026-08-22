#ifndef INK_TESTCASE_DIAGNOSTIC_TEST_SUPPORT_H
#define INK_TESTCASE_DIAGNOSTIC_TEST_SUPPORT_H

#include "ink/core/context.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ink::test
{
  inline bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind) noexcept
  {
    for (const core::Diagnostic &DiagnosticEntry : Diagnostics)
    {
      if (DiagnosticEntry.Kind == Kind)
      {
        return true;
      }
    }
    return false;
  }

  class DiagnosticCapture
  {
    public:
      explicit DiagnosticCapture(core::CompilationContext &Compilation)
          : Compilation(Compilation)
      {
        Compilation.diagnosticEngine().addConsumer(Consumer);
      }

      ~DiagnosticCapture()
      {
        Compilation.diagnosticEngine().removeConsumer(Consumer);
      }

      DiagnosticCapture(const DiagnosticCapture &) = delete;
      DiagnosticCapture &operator=(const DiagnosticCapture &) = delete;
      DiagnosticCapture(DiagnosticCapture &&) = delete;
      DiagnosticCapture &operator=(DiagnosticCapture &&) = delete;

      const std::vector<core::Diagnostic> &diagnostics() const noexcept
      {
        return Consumer.diagnostics();
      }

      void clear() noexcept
      {
        Consumer.clear();
      }

    private:
      core::CompilationContext &Compilation;
      core::CollectingDiagnosticConsumer Consumer;
  };

  class SharedDiagnosticTestContext
  {
    public:
      SharedDiagnosticTestContext()
      {
        Compilation.diagnosticEngine().addConsumer(Consumer);
      }

      ~SharedDiagnosticTestContext()
      {
        Compilation.diagnosticEngine().removeConsumer(Consumer);
      }

      SharedDiagnosticTestContext(const SharedDiagnosticTestContext &) = delete;
      SharedDiagnosticTestContext &operator=(const SharedDiagnosticTestContext &) = delete;
      SharedDiagnosticTestContext(SharedDiagnosticTestContext &&) = delete;
      SharedDiagnosticTestContext &operator=(SharedDiagnosticTestContext &&) = delete;

      core::CompilationContext &compilationContext() noexcept
      {
        return Compilation;
      }

      std::size_t checkpoint() const noexcept
      {
        return Consumer.diagnostics().size();
      }

      void record(core::SourceId Source, std::size_t Checkpoint)
      {
        const std::vector<core::Diagnostic> &AllDiagnostics = Consumer.diagnostics();
        Diagnostics[Source.value()] = std::vector<core::Diagnostic>(AllDiagnostics.begin() + static_cast<std::ptrdiff_t>(Checkpoint), AllDiagnostics.end());
      }

      void record(core::SourceId Source, std::size_t Checkpoint, const std::vector<core::Diagnostic> &Prefix)
      {
        const std::vector<core::Diagnostic> &AllDiagnostics = Consumer.diagnostics();
        std::vector<core::Diagnostic> &Recorded = Diagnostics[Source.value()];
        Recorded = Prefix;
        Recorded.insert(Recorded.end(), AllDiagnostics.begin() + static_cast<std::ptrdiff_t>(Checkpoint), AllDiagnostics.end());
      }

      const std::vector<core::Diagnostic> &diagnostics(core::SourceId Source) const noexcept
      {
        const auto Iterator = Diagnostics.find(Source.value());
        if (Iterator == Diagnostics.end())
        {
          static const std::vector<core::Diagnostic> Empty;
          return Empty;
        }
        return Iterator->second;
      }

    private:
      core::CompilationContext Compilation;
      core::CollectingDiagnosticConsumer Consumer;
      std::unordered_map<std::uint64_t, std::vector<core::Diagnostic>> Diagnostics;
  };

  inline SharedDiagnosticTestContext &sharedDiagnosticTestContext()
  {
    static SharedDiagnosticTestContext Context;
    return Context;
  }
} // namespace ink::test

#endif
