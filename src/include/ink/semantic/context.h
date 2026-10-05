#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/core/context.h"

namespace ink::semantic
{
  // The dispatch skeleton borrows source and diagnostic services without owning IR or execution state.
  class SemanticContext final : public core::FrontendContext
  {
    public:
      explicit SemanticContext(core::CompilationContext &Compilation)
          : core::FrontendContext(Compilation)
      {
      }

      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;
  };
} // namespace ink::semantic

#endif
