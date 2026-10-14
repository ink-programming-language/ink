#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/frontend/context.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/context.h"

#include <cstddef>
#include <utility>

namespace ink::semantic
{
  // Owns the IRContext and analysis state; source and diagnostic services are borrowed.
  // Caller-owned modules and all other users of its IRContext must die before this context.
  // Successive analysis calls preserve pooled IR while selecting a new source and resetting traversal state; access is not thread-safe.
  class SemanticContext final
  {
    public:
      explicit SemanticContext(core::SourceManager &Sources, core::DiagnosticEngine &Diagnostics, target::TargetContext Target)
          : Frontend(Sources, Diagnostics),
            IR(Target)
      {
      }

      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;

      auto &diagnosticEngine(this auto &&Self) noexcept
      {
        return Self.Frontend.diagnosticEngine();
      }

      auto &sourceManager(this auto &&Self) noexcept
      {
        return Self.Frontend.sourceManager();
      }

      auto &typePool(this auto &&Self) noexcept
      {
        return Self.IR.typePool();
      }

      auto &irContext(this auto &&Self) noexcept
      {
        return Self.IR;
      }

      // Reports against the input selected by the current analysis call.
      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void report(core::SourceRange Span, ArgumentTypes &&...Arguments)
      {
        diagnosticEngine().report<Kind>(Source, Span, std::forward<ArgumentTypes>(Arguments)...);
      }

    private:
      friend class Analyzer;

      frontend::FrontendContext Frontend;
      ir::IRContext IR;
      core::SourceId Source;
      std::size_t BlockDepthLimit = 0;
      std::size_t BlockDepth = 0;
  };
} // namespace ink::semantic

#endif
