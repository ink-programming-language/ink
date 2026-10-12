#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/frontend/context.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

namespace ink::semantic
{
  // The dispatch skeleton borrows source and diagnostic services without owning IR or execution state.
  class SemanticContext final : public frontend::FrontendContext
  {
    public:
      explicit SemanticContext(core::SourceManager &Sources, core::DiagnosticEngine &Diagnostics)
          : frontend::FrontendContext(Sources, Diagnostics)
      {
      }

      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;
  };
} // namespace ink::semantic

#endif
