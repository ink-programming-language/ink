#include "ink/core/diagnostic.h"

#include <algorithm>
#include <utility>

namespace ink::core
{
  namespace
  {
    template <typename ValueType>
    const ValueType *findArgument(const std::vector<DiagnosticArgument> &Arguments, DiagnosticArgumentName Name)
    {
      for (const DiagnosticArgument &Argument : Arguments)
      {
        if (Argument.Name == Name)
        {
          return std::get_if<ValueType>(&Argument.Value);
        }
      }
      return nullptr;
    }

    std::string codePointName(char32_t Value)
    {
      constexpr char Digits[] = "0123456789ABCDEF";
      std::string Result = "U+";
      std::string Hexadecimal;
      do
      {
        Hexadecimal.push_back(Digits[Value & 0xFU]);
        Value >>= 4U;
      } while (Value != 0);
      while (Hexadecimal.size() < 4)
      {
        Hexadecimal.push_back('0');
      }
      Result.append(Hexadecimal.rbegin(), Hexadecimal.rend());
      return Result;
    }

    void formatUnterminatedBlockComment(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
      if (const std::uint64_t *RemainingDepth = findArgument<std::uint64_t>(DiagnosticEntry.Arguments, DiagnosticArgumentName::RemainingNestingDepth))
      {
        Result.Message += "; remaining nesting depth: " + std::to_string(*RemainingDepth);
      }
      for (const DiagnosticRelatedInformation &RelatedEntry : DiagnosticEntry.Related)
      {
        if (RelatedEntry.Kind == DiagnosticRelatedKind::MostRecentUnclosedBlockComment)
        {
          Result.Notes.push_back({RelatedEntry.Span, "most recent unclosed block comment opening is here"});
        }
      }
      if (const bool *Unavailable = findArgument<bool>(DiagnosticEntry.Arguments, DiagnosticArgumentName::MostRecentOpeningUnavailable); Unavailable != nullptr && *Unavailable)
      {
        Result.Notes.push_back({std::nullopt, "most recent unclosed opening was not retained after the nesting limit was exceeded"});
      }
    }

    void formatInvisibleCharacter(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
      if (const char32_t *Character = findArgument<char32_t>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Character))
      {
        Result.Message = "invisible format character " + codePointName(*Character);
        if (const DiagnosticSourceContext *Context = findArgument<DiagnosticSourceContext>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Context))
        {
          if (*Context == DiagnosticSourceContext::Identifier)
          {
            Result.Message += " appears in an identifier";
          }
          else if (*Context == DiagnosticSourceContext::SourceText)
          {
            Result.Message += " appears in source text";
          }
        }
      }
      for (const DiagnosticRelatedInformation &RelatedEntry : DiagnosticEntry.Related)
      {
        const bool IsPrevious = RelatedEntry.Kind == DiagnosticRelatedKind::PreviousVisibleCharacter;
        const bool IsNext = RelatedEntry.Kind == DiagnosticRelatedKind::NextVisibleCharacter;
        if (!IsPrevious && !IsNext)
        {
          continue;
        }
        std::string Message = IsPrevious ? "previous visible character" : "next visible character";
        if (const char32_t *Character = findArgument<char32_t>(RelatedEntry.Arguments, DiagnosticArgumentName::Character))
        {
          Message += " is " + codePointName(*Character);
        }
        else
        {
          Message += " is here";
        }
        Result.Notes.push_back({RelatedEntry.Span, std::move(Message)});
      }
    }

    void formatExpected(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
      if (const std::string *Expected = findArgument<std::string>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Expected))
      {
        Result.Message += " '" + *Expected + "'";
      }
    }

    void formatActual(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
      if (const std::string *Actual = findArgument<std::string>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Actual))
      {
        Result.Message += " '" + *Actual + "'";
      }
    }

    void formatDetail(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
      if (const std::string *Detail = findArgument<std::string>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Detail))
      {
        Result.Message += ": " + *Detail;
      }
    }
  } // namespace

  std::uint32_t diagnosticNumber(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) \
  case DiagnosticKind::Name:                                                        \
    return Number;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return 0;
  }

  const char *diagnosticCode(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) \
  case DiagnosticKind::Name:                                                        \
    return Code;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "INK-0000";
  }

  const char *diagnosticKindName(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) \
  case DiagnosticKind::Name:                                                        \
    return #Name;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "Unknown";
  }

  const char *diagnosticDefaultMessage(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) \
  case DiagnosticKind::Name:                                                        \
    return DefaultMessage;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "unknown diagnostic";
  }

  DiagnosticDomain diagnosticDomain(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) \
  case DiagnosticKind::Name:                                                        \
    return DiagnosticDomain::Domain;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticDomain::Unknown;
  }

  DiagnosticSeverity diagnosticDefaultSeverity(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) \
  case DiagnosticKind::Name:                                                        \
    return DiagnosticSeverity::DefaultSeverity;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticSeverity::Unknown;
  }

  const char *diagnosticSeverityName(DiagnosticSeverity Severity) noexcept
  {
    switch (Severity)
    {
    case DiagnosticSeverity::Unknown:
      return "unknown";
    case DiagnosticSeverity::Error:
      return "error";
    case DiagnosticSeverity::Warning:
      return "warning";
    case DiagnosticSeverity::Note:
      return "note";
    }
    return "unknown";
  }

  bool operator==(const DiagnosticArgument &Left, const DiagnosticArgument &Right)
  {
    return Left.Name == Right.Name && Left.Value == Right.Value;
  }

  bool operator!=(const DiagnosticArgument &Left, const DiagnosticArgument &Right)
  {
    return !(Left == Right);
  }

  bool operator==(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right)
  {
    return Left.Kind == Right.Kind && Left.Span == Right.Span && Left.Arguments == Right.Arguments;
  }

  bool operator!=(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right)
  {
    return !(Left == Right);
  }

  std::uint32_t Diagnostic::number() const noexcept
  {
    return diagnosticNumber(Kind);
  }

  const char *Diagnostic::code() const noexcept
  {
    return diagnosticCode(Kind);
  }

  bool operator==(const Diagnostic &Left, const Diagnostic &Right)
  {
    return Left.Kind == Right.Kind && Left.Span == Right.Span && Left.Arguments == Right.Arguments && Left.Related == Right.Related;
  }

  bool operator!=(const Diagnostic &Left, const Diagnostic &Right)
  {
    return !(Left == Right);
  }

  DiagnosticBuilder::DiagnosticBuilder(DiagnosticKind Kind, SourceRange Span)
      : Result{Kind, Span, {}, {}}
  {
  }

  DiagnosticBuilder &DiagnosticBuilder::argument(DiagnosticArgumentName Name, DiagnosticArgumentValue Value) &
  {
    Result.Arguments.push_back({Name, std::move(Value)});
    return *this;
  }

  DiagnosticBuilder &&DiagnosticBuilder::argument(DiagnosticArgumentName Name, DiagnosticArgumentValue Value) &&
  {
    argument(Name, std::move(Value));
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

  bool operator==(const FormattedDiagnosticNote &Left, const FormattedDiagnosticNote &Right)
  {
    return Left.Span == Right.Span && Left.Message == Right.Message;
  }

  bool operator!=(const FormattedDiagnosticNote &Left, const FormattedDiagnosticNote &Right)
  {
    return !(Left == Right);
  }

  bool operator==(const FormattedDiagnostic &Left, const FormattedDiagnostic &Right)
  {
    return Left.Severity == Right.Severity && Left.Message == Right.Message && Left.Notes == Right.Notes;
  }

  bool operator!=(const FormattedDiagnostic &Left, const FormattedDiagnostic &Right)
  {
    return !(Left == Right);
  }

  FormattedDiagnostic DiagnosticFormatter::format(const Diagnostic &DiagnosticEntry) const
  {
    FormattedDiagnostic Result{diagnosticDefaultSeverity(DiagnosticEntry.Kind), diagnosticDefaultMessage(DiagnosticEntry.Kind), {}};
    if (DiagnosticEntry.Kind == DiagnosticKind::UnterminatedBlockComment)
    {
      formatUnterminatedBlockComment(DiagnosticEntry, Result);
    }
    else if (DiagnosticEntry.Kind == DiagnosticKind::InvisibleCharacter)
    {
      formatInvisibleCharacter(DiagnosticEntry, Result);
    }
    else if (DiagnosticEntry.Kind == DiagnosticKind::ExpectedToken || DiagnosticEntry.Kind == DiagnosticKind::ExpectedSyntax)
    {
      formatExpected(DiagnosticEntry, Result);
    }
    else if (DiagnosticEntry.Kind == DiagnosticKind::UnexpectedToken || DiagnosticEntry.Kind == DiagnosticKind::ReservedSymbolSequence)
    {
      formatActual(DiagnosticEntry, Result);
    }
    else if (DiagnosticEntry.Kind == DiagnosticKind::InvalidIrText || DiagnosticEntry.Kind == DiagnosticKind::InvalidIrModule || DiagnosticEntry.Kind == DiagnosticKind::ExecutionFailed)
    {
      formatDetail(DiagnosticEntry, Result);
    }
    return Result;
  }

  void DiagnosticEngine::addConsumer(DiagnosticConsumer &Consumer)
  {
    if (std::find(Consumers.begin(), Consumers.end(), &Consumer) == Consumers.end())
    {
      Consumers.push_back(&Consumer);
    }
  }

  void DiagnosticEngine::removeConsumer(DiagnosticConsumer &Consumer) noexcept
  {
    Consumers.erase(std::remove(Consumers.begin(), Consumers.end(), &Consumer), Consumers.end());
  }

  void DiagnosticEngine::report(const Diagnostic &DiagnosticEntry) const
  {
    const std::vector<DiagnosticConsumer *> Snapshot = Consumers;
    for (DiagnosticConsumer *Consumer : Snapshot)
    {
      Consumer->consume(DiagnosticEntry);
    }
  }

  void CollectingDiagnosticConsumer::consume(const Diagnostic &DiagnosticEntry)
  {
    Diagnostics.push_back(DiagnosticEntry);
  }
} // namespace ink::core
