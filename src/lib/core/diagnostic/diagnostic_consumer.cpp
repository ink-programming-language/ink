#include "ink/core/diagnostic/diagnostic_consumer.h"

namespace ink::core
{
  void CollectingDiagnosticConsumer::consume(const Diagnostic &DiagnosticEntry)
  {
    Diagnostics.push_back(DiagnosticEntry);
  }
} // namespace ink::core
