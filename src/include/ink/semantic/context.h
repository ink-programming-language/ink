#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/frontend/context.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_context.h"

#include <cstddef>
#include <utility>

namespace ink::semantic
{
  // Owns the types, module, builder and analysis state for one module; source and diagnostic services are borrowed.
  // Borrowed services must outlive this context, and all users of its IR must finish before it is destroyed.
  // Successive analysis calls preserve IR while selecting a new source and resetting traversal state; access is not thread-safe.
  class SemanticContext final
  {
    public:
      explicit SemanticContext(core::SourceManager &Sources, core::DiagnosticEngine &Diagnostics, target::TargetContext Target)
          : Frontend(Sources, Diagnostics),
            Types(Target),
            Builder(Types)
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

      auto &typeContext(this auto &&Self) noexcept
      {
        return Self.Types;
      }

      auto &module(this auto &&Self) noexcept
      {
        return Self.Module;
      }

      auto &irBuilder(this auto &&Self) noexcept
      {
        return Self.Builder;
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
      // Reverse destruction order releases the builder, then the module, then its borrowed types and constants.
      ir::TypeContext Types;
      ir::Module Module;
      ir::IRBuilder Builder;
      core::SourceId Source;
      std::size_t BlockDepthLimit = 0;
      std::size_t BlockDepth = 0;
  };
} // namespace ink::semantic

#endif
