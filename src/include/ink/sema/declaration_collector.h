#ifndef INK_SEMA_DECLARATION_COLLECTOR_H
#define INK_SEMA_DECLARATION_COLLECTOR_H

#include "ink/core/diagnostic.h"
#include "ink/sema/semantic_model.h"

#include <vector>

namespace ink::sema
{
  class DeclarationCollector
  {
  public:
    DeclarationCollector(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept;
    void run();

  private:
    SemanticModel &Model;
    std::vector<core::Diagnostic> &Diagnostics;
  };
} // namespace ink::sema

#endif
