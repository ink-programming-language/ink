#ifndef INK_SEMANTIC_ANALYZER_ANALYSIS_SITE_H
#define INK_SEMANTIC_ANALYZER_ANALYSIS_SITE_H

#include "ink/core/diagnostic/diagnostic_traits.h"
#include "ink/core/source/source_id.h"
#include "ink/core/source/source_range.h"
#include "ink/semantic/context.h"

#include <utility>

namespace ink::semantic
{
  // Captures one diagnostic location; the borrowed context must outlive this site.
  class AnalysisSite final
  {
    public:
      AnalysisSite(SemanticContext &Context, core::SourceId Source, core::SourceRange Range) noexcept
          : Context(Context),
            Source(Source),
            Range(Range)
      {
      }

      SemanticContext &semanticContext() const noexcept
      {
        return Context;
      }

      // Selects another range in the same source without changing this site or the context's source selection.
      AnalysisSite at(core::SourceRange NewRange) const noexcept
      {
        return AnalysisSite(Context, Source, NewRange);
      }

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void report(ArgumentTypes &&...Arguments) const
      {
        static_assert(core::DiagnosticTraits<Kind>::Classification != core::DiagnosticClass::InternalCompilerError, "report requires a non-ICE diagnostic; use reportICE for internal compiler errors");
        Context.diagnosticEngine().report<Kind>(Source, Range, std::forward<ArgumentTypes>(Arguments)...);
      }

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      [[noreturn]] void reportICE(ArgumentTypes &&...Arguments) const
      {
        static_assert(core::DiagnosticTraits<Kind>::Classification == core::DiagnosticClass::InternalCompilerError, "reportICE requires an internal compiler error diagnostic");
        Context.diagnosticEngine().reportICE<Kind>(Source, Range, std::forward<ArgumentTypes>(Arguments)...);
      }

    private:
      SemanticContext &Context;
      const core::SourceId Source;
      const core::SourceRange Range;
  };
} // namespace ink::semantic

#endif
