#include "ink/core/diagnostic/diagnostic_builder.h"

#include <utility>

namespace ink::core
{
  DiagnosticBuilder::DiagnosticBuilder(Diagnostic Result)
      : Result(std::move(Result))
  {
  }

  DiagnosticBuilder &DiagnosticBuilder::classification(DiagnosticClass Class) &
  {
    Result.Class = Class;
    return *this;
  }

  DiagnosticBuilder &&DiagnosticBuilder::classification(DiagnosticClass Class) &&
  {
    classification(Class);
    return std::move(*this);
  }

  DiagnosticBuilder &DiagnosticBuilder::related(DiagnosticRelatedKind Kind, SourceRange Span, std::vector<DiagnosticArgument> Arguments) &
  {
    Result.Related.push_back({Kind, Span, std::move(Arguments)});
    return *this;
  }

  DiagnosticBuilder &&DiagnosticBuilder::related(DiagnosticRelatedKind Kind, SourceRange Span, std::vector<DiagnosticArgument> Arguments) &&
  {
    related(Kind, Span, std::move(Arguments));
    return std::move(*this);
  }

  Diagnostic DiagnosticBuilder::build() &&
  {
    return std::move(Result);
  }
} // namespace ink::core
