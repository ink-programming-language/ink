#ifndef INK_CORE_DIAGNOSTIC_ENGINE_H
#define INK_CORE_DIAGNOSTIC_ENGINE_H

#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"

#include <utility>
#include <vector>

namespace ink::core
{
  class DiagnosticEngine
  {
    public:
      DiagnosticEngine() = default;
      DiagnosticEngine(const DiagnosticEngine &) = delete;
      DiagnosticEngine &operator=(const DiagnosticEngine &) = delete;
      DiagnosticEngine(DiagnosticEngine &&) = delete;
      DiagnosticEngine &operator=(DiagnosticEngine &&) = delete;

      void addConsumer(DiagnosticConsumer &Consumer);
      void removeConsumer(DiagnosticConsumer &Consumer) noexcept;
      // ICE is printed synchronously and panics before any consumer can defer or suppress it.
      void report(const Diagnostic &DiagnosticEntry) const;

      template <DiagnosticKind Kind, typename... ArgumentTypes>
      void report(SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        report(makeDiagnostic<Kind>(Span, std::forward<ArgumentTypes>(Arguments)...));
      }

      template <DiagnosticKind Kind, typename... ArgumentTypes>
      void report(SourceId Source, SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        auto Entry = makeDiagnostic<Kind>(Span, std::forward<ArgumentTypes>(Arguments)...);
        Entry.Source = Source;
        report(Entry);
      }

    private:
      std::vector<DiagnosticConsumer *> Consumers;
  };
} // namespace ink::core

#endif
