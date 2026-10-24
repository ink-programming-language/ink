#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/frontend/context.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/context.h"
#include "ink/ir/module/module_manager.h"
#include "ink/semantic/name_resolve/scope_store.h"

#include <utility>

namespace ink::semantic
{
  // Owns the IRContext, module manager, persistent scopes and source selection; source and diagnostic services are borrowed.
  // Managed modules outlive scopes and die before the IR pools; external users must finish before this context is destroyed.
  // Standalone outputs remain supported and must die before this context after their scope bindings are removed.
  // Successive analysis calls preserve pooled IR while selecting a new source; access is not thread-safe.
  class SemanticContext final
  {
    public:
      explicit SemanticContext(core::SourceManager &Sources, core::DiagnosticEngine &Diagnostics, target::TargetContext Target)
          : Frontend(Sources, Diagnostics),
            IR(Target),
            Modules(IR)
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

      auto &moduleManager(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Modules);
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

      // Reports a fatal invariant or unsupported feature against the selected source; never returns.
      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      [[noreturn]] void reportICE(core::SourceRange Span, ArgumentTypes &&...Arguments)
      {
        diagnosticEngine().reportICE<Kind>(Source, Span, std::forward<ArgumentTypes>(Arguments)...);
      }

    private:
      frontend::FrontendContext Frontend;
      ir::IRContext IR;
      ir::ModuleManager Modules;
      ScopeStore Scopes;
      core::SourceId Source;
  };
} // namespace ink::semantic

#endif
