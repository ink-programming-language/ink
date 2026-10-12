#ifndef INK_CORE_DIAGNOSTIC_CONSUMER_H
#define INK_CORE_DIAGNOSTIC_CONSUMER_H

#include "ink/core/diagnostic/diagnostic.h"

#include <utility>
#include <vector>

namespace ink::core
{
  class DiagnosticConsumer
  {
    public:
      virtual ~DiagnosticConsumer() = default;
      virtual void consume(const Diagnostic &DiagnosticEntry) = 0;
  };

  class CollectingDiagnosticConsumer final : public DiagnosticConsumer
  {
    public:
      void consume(const Diagnostic &DiagnosticEntry) override;

      const std::vector<Diagnostic> &diagnostics() const noexcept
      {
        return Diagnostics;
      }

      std::vector<Diagnostic> takeDiagnostics() noexcept
      {
        return std::move(Diagnostics);
      }

      void clear() noexcept
      {
        Diagnostics.clear();
      }

    private:
      std::vector<Diagnostic> Diagnostics;
  };
} // namespace ink::core

#endif
