#ifndef INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H
#define INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H

#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "ink/parser/token_cursor.h"

#include "ink/core/config_manager.h"
#include "ink/core/diagnostic.h"
#include "ink/core/source_id.h"

#include <cstddef>
#include <utility>

namespace ink::semantic
{
  struct Analyzer::AnalysisState
  {
      AnalysisState(SemanticContext &Context, Scope &InitialScope, const parser::TokenBuffer &Input)
          : Context(Context),
            Resolver(InitialScope),
            Builder(Context),
            Input(Input),
            Source(Input.lexedFile().sourceId())
      {
      }

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void report(core::SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        Context.compilationContext().diagnosticEngine().report<Kind>(Source, Span, std::forward<ArgumentTypes>(Arguments)...);
      }

      SemanticContext &Context;
      NameResolver Resolver;
      IRBuilder Builder;
      const parser::TokenBuffer &Input;
      core::SourceId Source;
      const std::size_t TypeDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticTypeDepthLimit>();
      std::size_t BlockDepth = 0;
  };
} // namespace ink::semantic

#endif
