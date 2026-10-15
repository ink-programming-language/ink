#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/diagnostic/diagnostic_traits.h"
#include "ink/core/source/source_manager.h"

#include <spdlog/fmt/bundled/args.h>
#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <type_traits>

namespace ink::core
{
  namespace
  {
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
      auto AddArgument = [&Store, Name](const auto &Value)
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
      };
      std::visit(AddArgument, Argument.Value);
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
      return fmt::vformat(FormatPattern, Store);
    }

    std::string formatRegisteredMessage(const Diagnostic &DiagnosticEntry)
    {
      switch (DiagnosticEntry.Kind)
      {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) \
  case DiagnosticKind::Name:                                                                                              \
    return formatMessage(typename DiagnosticTraits<DiagnosticKind::Name>::Arguments{}, DiagnosticEntry, FormatPattern, DefaultMessage);
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
      }
      return "unknown diagnostic";
    }

    std::string formatRelatedMessage(const DiagnosticRelatedInformation &Related)
    {
      switch (Related.Kind)
      {
      case DiagnosticRelatedKind::MostRecentUnclosedBlockComment:
        return "most recent unclosed block comment starts here";
      case DiagnosticRelatedKind::MostRecentBlockCommentOpeningUnavailable:
        return "the opening of the most recent block comment is unavailable";
      case DiagnosticRelatedKind::PreviousVisibleCharacter:
        return "previous visible character is here";
      case DiagnosticRelatedKind::NextVisibleCharacter:
        return "next visible character is here";
      case DiagnosticRelatedKind::Unknown:
        return "related source location";
      }
      return "related source location";
    }

    void appendSourcePreview(std::string &Output, const SourceBuffer &Source, SourceRange Span)
    {
      const auto &Text = Source.text();
      const auto &LineStarts = Source.lineStarts();
      const std::size_t Begin = Span.getBegin().getByteOffset();
      const std::size_t End = Span.getEnd().getByteOffset();
      const std::size_t FirstLine = Source.lineNumber(Begin);
      const std::size_t LastLine = Source.lineNumber(End == Begin ? End : End - 1);
      const std::size_t NumberWidth = std::to_string(LastLine).size();
      for (std::size_t Line = FirstLine; Line <= LastLine; ++Line)
      {
        const std::size_t LineBegin = LineStarts[Line - 1];
        std::size_t LineEnd = Line < LineStarts.size() ? LineStarts[Line] : Text.size();
        while (LineEnd > LineBegin && (Text[LineEnd - 1] == '\r' || Text[LineEnd - 1] == '\n'))
        {
          --LineEnd;
        }
        const std::size_t HighlightBegin = std::clamp(Begin, LineBegin, LineEnd);
        const std::size_t HighlightEnd = std::clamp(End, HighlightBegin, LineEnd);
        std::string Preview;
        std::size_t MarkerBegin = 0;
        std::size_t MarkerEnd = 0;
        for (std::size_t Offset = LineBegin; Offset <= LineEnd; ++Offset)
        {
          if (Offset == HighlightBegin)
          {
            MarkerBegin = Preview.size();
          }
          if (Offset == HighlightEnd)
          {
            MarkerEnd = Preview.size();
          }
          if (Offset == LineEnd)
          {
            break;
          }
          const auto Byte = static_cast<unsigned char>(Text[Offset]);
          if (Byte == '\t')
          {
            Preview.append(8 - Preview.size() % 8, ' ');
          }
          else if (Byte >= 0x20 && Byte < 0x7f)
          {
            Preview.push_back(static_cast<char>(Byte));
          }
          else
          {
            Preview += fmt::format("\\x{:02X}", Byte);
          }
        }
        const std::string Number = std::to_string(Line);
        Output += fmt::format(" {}{} | {}\n", std::string(NumberWidth - Number.size(), ' '), Number, Preview);
        Output += fmt::format(" {} | {}^{}\n", std::string(NumberWidth, ' '), std::string(MarkerBegin, ' '), std::string(MarkerEnd > MarkerBegin ? MarkerEnd - MarkerBegin - 1 : 0, '~'));
      }
    }

    void appendSourceMessage(std::string &Output, const SourceBuffer *Source, SourceRange Span, std::string_view Label, std::string_view Message)
    {
      Output += Source ? Source->name() : "<unknown>";
      const bool HasSourceRange = Source && Span.isValid() && Span.getEnd().getByteOffset() <= Source->text().size();
      if (HasSourceRange)
      {
        const std::size_t Begin = Span.getBegin().getByteOffset();
        const std::size_t Line = Source->lineNumber(Begin);
        const std::size_t Column = Begin - Source->lineStarts()[Line - 1] + 1;
        Output += fmt::format(":{}:{}", Line, Column);
      }
      Output += fmt::format(": {}: {}", Label, Message);
      if (Span.isValid())
      {
        Output += fmt::format(" [bytes {}, {})", Span.getBegin().getByteOffset(), Span.getEnd().getByteOffset());
      }
      Output.push_back('\n');
      if (HasSourceRange)
      {
        appendSourcePreview(Output, *Source, Span);
      }
    }
  } // namespace

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
    for (const DiagnosticRelatedInformation &Related : DiagnosticEntry.Related)
    {
      Result.Notes.push_back({Related.Span.isValid() ? std::optional<SourceRange>(Related.Span) : std::nullopt, formatRelatedMessage(Related)});
    }
    return Result;
  }

  std::string DiagnosticFormatter::format(const Diagnostic &DiagnosticEntry, const SourceManager &Sources) const
  {
    const FormattedDiagnostic Formatted = format(DiagnosticEntry);
    const auto Source = Sources.findSource(DiagnosticEntry.Source);
    const char *Severity = DiagnosticEntry.classification() == DiagnosticClass::InternalCompilerError ? diagnosticClassName(DiagnosticClass::InternalCompilerError) : diagnosticSeverityName(Formatted.Severity);
    std::string Result;
    appendSourceMessage(Result, Source.get(), DiagnosticEntry.Span, fmt::format("{}[{}]", Severity, DiagnosticEntry.code()), Formatted.Message);
    for (const FormattedDiagnosticNote &Note : Formatted.Notes)
    {
      // Related information currently uses ranges in the primary diagnostic's source buffer.
      appendSourceMessage(Result, Source.get(), Note.Span.value_or(SourceRange{}), "note", Note.Message);
    }
    return Result;
  }
} // namespace ink::core
