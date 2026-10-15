#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/frontend/context.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/context.h"
#include "ink/semantic/name_resolve/scope_store.h"

#include <utility>

namespace ink::semantic
{
  // Owns the IRContext, persistent scopes and source selection; source and diagnostic services are borrowed.
  // Caller-owned modules and all other users of its IRContext must die before this context.
  // Successive analysis calls preserve pooled IR while selecting a new source; access is not thread-safe.
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

      auto &scopeStore(this auto &&Self) noexcept
      {
        return Self.Scopes;
      }

      // The analysis entry selects its validated source before invoking node handlers.
      void setSource(core::SourceId Source) noexcept
      {
        this->Source = Source;
      }

      // Reports against the input selected by the current analysis call.
      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void report(core::SourceRange Span, ArgumentTypes &&...Arguments)
      {
        diagnosticEngine().report<Kind>(Source, Span, std::forward<ArgumentTypes>(Arguments)...);
      }

    private:
      frontend::FrontendContext Frontend;
      ir::IRContext IR;
      ScopeStore Scopes;
      core::SourceId Source;
  };
} // namespace ink::semantic

#endif
