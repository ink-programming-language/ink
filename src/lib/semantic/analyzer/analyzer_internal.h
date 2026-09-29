#ifndef INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H
#define INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H

#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/source_id.h"

#include <cstddef>

namespace ink::semantic
{
  class NameResolver;

  struct Analyzer::AnalysisState
  {
      SemanticContext &Context;
      NameResolver &Resolver;
      core::SourceId Source;
      std::size_t BlockDepth = 0;
  };
} // namespace ink::semantic

#endif
