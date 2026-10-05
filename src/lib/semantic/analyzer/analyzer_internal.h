#ifndef INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H
#define INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H

#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/parser/token_cursor.h"
#include "ink/core/config_manager.h"

#include <cstddef>
#include <utility>

namespace ink::semantic
{
  struct Analyzer::AnalysisState
  {
      AnalysisState(SemanticContext &Context, const parser::TokenBuffer &Input)
          : Context(Context),
            Source(Input.lexedFile().sourceId()),
            BlockDepthLimit(core::ConfigManager::getSize<core::ConfigKind::SemanticBlockDepthLimit>())
      {
      }

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void report(core::SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        Context.diagnosticEngine().report<Kind>(Source, Span, std::forward<ArgumentTypes>(Arguments)...);
      }

      SemanticContext &Context;
      core::SourceId Source;
      const std::size_t BlockDepthLimit;
      std::size_t BlockDepth = 0;
  };
} // namespace ink::semantic

#endif
