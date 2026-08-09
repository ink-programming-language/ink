#ifndef INK_SEMA_NAME_RESOLVER_H
#define INK_SEMA_NAME_RESOLVER_H

#include "ink/core/diagnostic.h"
#include "ink/sema/semantic_model.h"

#include <vector>

namespace ink::sema
{
  class NameResolver
  {
  public:
    NameResolver(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept;
    void run();

  private:
    SemanticModel &Model;
    std::vector<core::Diagnostic> &Diagnostics;
  };
} // namespace ink::sema

#endif
