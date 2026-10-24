#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/support/core_define.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>

namespace ink::core
{
  void DiagnosticEngine::addConsumer(DiagnosticConsumer &Consumer)
  {
    if (std::find(Consumers.begin(), Consumers.end(), &Consumer) == Consumers.end())
    {
      Consumers.push_back(&Consumer);
    }
  }

  void DiagnosticEngine::removeConsumer(DiagnosticConsumer &Consumer) noexcept
  {
    Consumers.erase(std::remove(Consumers.begin(), Consumers.end(), &Consumer), Consumers.end());
  }

  void DiagnosticEngine::report(const Diagnostic &DiagnosticEntry) const
  {
    if (DiagnosticEntry.classification() == DiagnosticClass::InternalCompilerError)
    {
      reportICE(DiagnosticEntry);
    }
    const std::vector<DiagnosticConsumer *> Snapshot = Consumers;
    for (DiagnosticConsumer *Consumer : Snapshot)
    {
      Consumer->consume(DiagnosticEntry);
    }
  }

  [[noreturn]] void DiagnosticEngine::reportICE(const Diagnostic &DiagnosticEntry) const
  {
    if (Sources)
    {
      panic(DiagnosticFormatter{}.format(DiagnosticEntry, *Sources));
    }
    panic(fmt::format("internal compiler error[{}]: {}", DiagnosticEntry.code(), DiagnosticFormatter{}.format(DiagnosticEntry).Message));
  }
} // namespace ink::core
