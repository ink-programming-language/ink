#ifndef INK_CORE_DIAGNOSTIC_ENGINE_H
#define INK_CORE_DIAGNOSTIC_ENGINE_H

#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"

#include <utility>
#include <vector>

namespace ink::core
{
  class SourceManager;

  class DiagnosticEngine
  {
    public:
      DiagnosticEngine() = default;
      // The optional source manager is borrowed and must outlive the engine.
      explicit DiagnosticEngine(const SourceManager &Sources)
          : Sources(&Sources)
      {
      }

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

      // Explicit fatal reporting accepts only registered ICE kinds and never returns to the caller.
      template <DiagnosticKind Kind, typename... ArgumentTypes>
      [[noreturn]] void reportICE(SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        static_assert(DiagnosticTraits<Kind>::Classification == DiagnosticClass::InternalCompilerError, "reportICE requires an internal compiler error diagnostic");
        reportICE(makeDiagnostic<Kind>(Span, std::forward<ArgumentTypes>(Arguments)...));
      }

      template <DiagnosticKind Kind, typename... ArgumentTypes>
      [[noreturn]] void reportICE(SourceId Source, SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        static_assert(DiagnosticTraits<Kind>::Classification == DiagnosticClass::InternalCompilerError, "reportICE requires an internal compiler error diagnostic");
        auto Entry = makeDiagnostic<Kind>(Span, std::forward<ArgumentTypes>(Arguments)...);
        Entry.Source = Source;
        reportICE(Entry);
      }

    private:
      [[noreturn]] void reportICE(const Diagnostic &DiagnosticEntry) const;

      const SourceManager *Sources = nullptr;
      std::vector<DiagnosticConsumer *> Consumers;
  };
} // namespace ink::core

#endif
