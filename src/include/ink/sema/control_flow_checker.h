#ifndef INK_SEMA_CONTROL_FLOW_CHECKER_H
#define INK_SEMA_CONTROL_FLOW_CHECKER_H

#include "ink/core/diagnostic.h"
#include "ink/sema/semantic_model.h"

#include <vector>

namespace ink::sema
{
  class ControlFlowChecker
  {
  public:
    ControlFlowChecker(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept;
    void run();

  private:
    SemanticModel &Model;
    std::vector<core::Diagnostic> &Diagnostics;
  };
} // namespace ink::sema

#endif
