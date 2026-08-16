#include "ink/core/diagnostic.h"

#include <spdlog/fmt/bundled/args.h>
#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <string_view>
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

    const char *sourceContextText(DiagnosticSourceContext Context) noexcept
    {
      switch (Context)
      {
      case DiagnosticSourceContext::SourceText:
        return "appears in source text";
      case DiagnosticSourceContext::Identifier:
        return "appears in an identifier";
      case DiagnosticSourceContext::Unknown:
        break;
      }
      return "appears in an unknown source context";
    }

    template <typename Specification>
    bool hasArgument(const std::vector<DiagnosticArgument> &Arguments)
    {
      using ValueType = typename Specification::ValueType;
      std::size_t MatchCount = 0;
      for (const DiagnosticArgument &Argument : Arguments)
      {
        if (Argument.Name == Specification::Name && std::holds_alternative<ValueType>(Argument.Value))
        {
          ++MatchCount;
        }
      }
      return MatchCount == 1;
    }

    template <typename... Specifications>
    bool matchesSchema(DiagnosticArgumentSchema<Specifications...>, const std::vector<DiagnosticArgument> &Arguments)
    {
      return Arguments.size() == sizeof...(Specifications) && (hasArgument<Specifications>(Arguments) && ...);
    }

    using FormatArgumentStore = fmt::dynamic_format_arg_store<fmt::format_context>;

    void addFormatArgument(FormatArgumentStore &Store, const DiagnosticArgument &Argument)
    {
      const char *Name = diagnosticArgumentName(Argument.Name);
      std::visit([&Store, Name](const auto &Value)
                 {
                   using ValueType = std::decay_t<decltype(Value)>;
                   if constexpr (std::is_same_v<ValueType, char32_t>)
                   {
                     Store.push_back(fmt::arg(Name, codePointName(Value)));
                   }
                   else if constexpr (std::is_same_v<ValueType, DiagnosticSourceContext>)
                   {
                     Store.push_back(fmt::arg(Name, sourceContextText(Value)));
                   }
                   else
                   {
                     Store.push_back(fmt::arg(Name, Value));
                   }
                 },
                 Argument.Value);
    }

    template <typename... Specifications>
    std::string formatMessage(DiagnosticArgumentSchema<Specifications...> Schema, const Diagnostic &DiagnosticEntry, const char *FormatPattern, const char *FallbackMessage)
    {
      if (!matchesSchema(Schema, DiagnosticEntry.Arguments))
      {
        return FallbackMessage;
      }
      FormatArgumentStore Store;
      for (const DiagnosticArgument &Argument : DiagnosticEntry.Arguments)
      {
        addFormatArgument(Store, Argument);
      }
      try
      {
        return fmt::vformat(FormatPattern, Store);
      }
      catch (const fmt::format_error &)
      {
        return FallbackMessage;
      }
    }

    std::string formatRegisteredMessage(const Diagnostic &DiagnosticEntry)
    {
      switch (DiagnosticEntry.Kind)
      {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
    return formatMessage(typename DiagnosticTraits<DiagnosticKind::Name>::Arguments{}, DiagnosticEntry, FormatPattern, DefaultMessage);
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
      }
      return "unknown diagnostic";
    }

    void appendUnterminatedBlockCommentNotes(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
      for (const DiagnosticRelatedInformation &RelatedEntry : DiagnosticEntry.Related)
      {
        if (RelatedEntry.Kind == DiagnosticRelatedKind::MostRecentUnclosedBlockComment)
        {
          Result.Notes.push_back({RelatedEntry.Span, "most recent unclosed block comment opening is here"});
        }
        else if (RelatedEntry.Kind == DiagnosticRelatedKind::MostRecentBlockCommentOpeningUnavailable)
        {
          Result.Notes.push_back({std::nullopt, "most recent unclosed opening was not retained after the nesting limit was exceeded"});
        }
      }
    }

    void appendInvisibleCharacterNotes(const Diagnostic &DiagnosticEntry, FormattedDiagnostic &Result)
    {
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
  } // namespace

  std::uint32_t diagnosticNumber(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
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
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
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
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
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
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
    return DefaultMessage;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "unknown diagnostic";
  }

  const char *diagnosticFormatPattern(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
    return FormatPattern;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return "unknown diagnostic";
  }

  DiagnosticDomain diagnosticDomain(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
    return DiagnosticDomain::Domain;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticDomain::Unknown;
  }

  DiagnosticClass diagnosticClass(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
    return DiagnosticClass::Class;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticClass::Unknown;
  }

  DiagnosticSeverity diagnosticDefaultSeverity(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                         \
    return DiagnosticSeverity::DefaultSeverity;
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }
    return DiagnosticSeverity::Unknown;
  }

  const char *diagnosticClassName(DiagnosticClass Class) noexcept
  {
    switch (Class)
    {
    case DiagnosticClass::Unknown:
      return "unknown";
    case DiagnosticClass::User:
      return "user";
    case DiagnosticClass::InternalCompilerError:
      return "internal compiler error";
    }
    return "unknown";
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

  const char *diagnosticArgumentName(DiagnosticArgumentName Name) noexcept
  {
    switch (Name)
    {
    case DiagnosticArgumentName::Unknown:
      return "Unknown";
    case DiagnosticArgumentName::Character:
      return "Character";
    case DiagnosticArgumentName::Context:
      return "Context";
    case DiagnosticArgumentName::RemainingNestingDepth:
      return "RemainingNestingDepth";
    case DiagnosticArgumentName::Expected:
      return "Expected";
    case DiagnosticArgumentName::Actual:
      return "Actual";
    case DiagnosticArgumentName::Description:
      return "Description";
    case DiagnosticArgumentName::TypeName:
      return "TypeName";
    case DiagnosticArgumentName::StructName:
      return "StructName";
    case DiagnosticArgumentName::SymbolName:
      return "SymbolName";
    case DiagnosticArgumentName::FunctionName:
      return "FunctionName";
    case DiagnosticArgumentName::BlockName:
      return "BlockName";
    case DiagnosticArgumentName::CalleeName:
      return "CalleeName";
    case DiagnosticArgumentName::Operation:
      return "Operation";
    case DiagnosticArgumentName::ArgumentIndex:
      return "ArgumentIndex";
    case DiagnosticArgumentName::ParameterIndex:
      return "ParameterIndex";
    case DiagnosticArgumentName::FieldIndex:
      return "FieldIndex";
    case DiagnosticArgumentName::ValueId:
      return "ValueId";
    case DiagnosticArgumentName::GlobalId:
      return "GlobalId";
    case DiagnosticArgumentName::ExpectedValue:
      return "ExpectedValue";
    case DiagnosticArgumentName::ActualValue:
      return "ActualValue";
    case DiagnosticArgumentName::ExpectedCount:
      return "ExpectedCount";
    case DiagnosticArgumentName::ActualCount:
      return "ActualCount";
    case DiagnosticArgumentName::DeclaredSize:
      return "DeclaredSize";
    case DiagnosticArgumentName::ActualSize:
      return "ActualSize";
    case DiagnosticArgumentName::FieldCount:
      return "FieldCount";
    case DiagnosticArgumentName::Offset:
      return "Offset";
    case DiagnosticArgumentName::Size:
      return "Size";
    case DiagnosticArgumentName::ActualVersion:
      return "ActualVersion";
    case DiagnosticArgumentName::SupportedVersion:
      return "SupportedVersion";
    case DiagnosticArgumentName::Alignment:
      return "Alignment";
    case DiagnosticArgumentName::MaximumAlignment:
      return "MaximumAlignment";
    case DiagnosticArgumentName::BlockCount:
      return "BlockCount";
    case DiagnosticArgumentName::CallDepthLimit:
      return "CallDepthLimit";
    case DiagnosticArgumentName::InstructionName:
      return "InstructionName";
    case DiagnosticArgumentName::ExceptionMessage:
      return "ExceptionMessage";
    }
    return "Unknown";
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

  DiagnosticClass Diagnostic::classification() const noexcept
  {
    return Class == DiagnosticClass::Unknown ? diagnosticClass(Kind) : Class;
  }

  bool operator==(const Diagnostic &Left, const Diagnostic &Right)
  {
    return Left.Kind == Right.Kind && Left.Span == Right.Span && Left.Arguments == Right.Arguments && Left.Related == Right.Related && Left.Class == Right.Class;
  }

  bool operator!=(const Diagnostic &Left, const Diagnostic &Right)
  {
    return !(Left == Right);
  }

  DiagnosticBuilder::DiagnosticBuilder(Diagnostic Result) : Result(std::move(Result))
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
    FormattedDiagnostic Result{diagnosticDefaultSeverity(DiagnosticEntry.Kind), formatRegisteredMessage(DiagnosticEntry), {}};
    if (DiagnosticEntry.Kind == DiagnosticKind::UnterminatedBlockComment)
    {
      appendUnterminatedBlockCommentNotes(DiagnosticEntry, Result);
    }
    else if (DiagnosticEntry.Kind == DiagnosticKind::InvisibleCharacterInContext)
    {
      appendInvisibleCharacterNotes(DiagnosticEntry, Result);
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
