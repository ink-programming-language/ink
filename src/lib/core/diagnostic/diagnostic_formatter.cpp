#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/diagnostic/diagnostic_traits.h"

#include <spdlog/fmt/bundled/args.h>
#include <spdlog/fmt/fmt.h>

#include <cstddef>
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
    return Result;
  }
} // namespace ink::core
