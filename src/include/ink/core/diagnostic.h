#ifndef INK_CORE_DIAGNOSTIC_H
#define INK_CORE_DIAGNOSTIC_H

#include "ink/core/source_range.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ink::core
{
  enum class DiagnosticDomain : std::uint8_t
  {
    Unknown = 0x00,
    Tokenizer = 0x01,
    Parser = 0x02,
    Semantic = 0x03,
    IR = 0x04,
    Execution = 0x05,
    Driver = 0x06,
  };

  enum class DiagnosticClass : std::uint8_t
  {
    Unknown,
    User,
    InternalCompilerError,
  };

  enum class DiagnosticSeverity : std::uint8_t
  {
    Unknown,
    Error,
    Warning,
    Note,
  };

  enum class DiagnosticSourceContext : std::uint8_t
  {
    Unknown,
    SourceText,
    Identifier,
  };

  enum class DiagnosticArgumentName : std::uint8_t
  {
    Unknown,
    Character,
    Context,
    RemainingNestingDepth,
    Expected,
    Actual,
    Description,
    TypeName,
    StructName,
    SymbolName,
    FunctionName,
    BlockName,
    CalleeName,
    Operation,
    ArgumentIndex,
    ParameterIndex,
    FieldIndex,
    ValueId,
    GlobalId,
    ExpectedCount,
    ActualCount,
    DeclaredSize,
    ActualSize,
    FieldCount,
    Offset,
    Size,
    ActualVersion,
    SupportedVersion,
    Alignment,
    MaximumAlignment,
    BlockCount,
    CallDepthLimit,
    InstructionName,
    ExceptionMessage,
  };

  enum class DiagnosticArgumentType : std::uint8_t
  {
    Boolean,
    SignedInteger,
    UnsignedInteger,
    Character,
    String,
    SourceContext,
  };

  enum class DiagnosticRelatedKind : std::uint8_t
  {
    Unknown,
    MostRecentUnclosedBlockComment,
    MostRecentBlockCommentOpeningUnavailable,
    PreviousVisibleCharacter,
    NextVisibleCharacter,
  };

  enum class DiagnosticKind : std::uint32_t
  {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) Name = Number,
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
  };

  std::uint32_t diagnosticNumber(DiagnosticKind Kind) noexcept;
  const char *diagnosticCode(DiagnosticKind Kind) noexcept;
  const char *diagnosticKindName(DiagnosticKind Kind) noexcept;
  const char *diagnosticDefaultMessage(DiagnosticKind Kind) noexcept;
  const char *diagnosticFormatPattern(DiagnosticKind Kind) noexcept;
  DiagnosticDomain diagnosticDomain(DiagnosticKind Kind) noexcept;
  DiagnosticClass diagnosticClass(DiagnosticKind Kind) noexcept;
  DiagnosticSeverity diagnosticDefaultSeverity(DiagnosticKind Kind) noexcept;
  const char *diagnosticClassName(DiagnosticClass Class) noexcept;
  const char *diagnosticSeverityName(DiagnosticSeverity Severity) noexcept;
  const char *diagnosticArgumentName(DiagnosticArgumentName Name) noexcept;

  template <DiagnosticArgumentType Type>
  struct DiagnosticArgumentTypeTraits;

  template <>
  struct DiagnosticArgumentTypeTraits<DiagnosticArgumentType::Boolean>
  {
    using ValueType = bool;
  };

  template <>
  struct DiagnosticArgumentTypeTraits<DiagnosticArgumentType::SignedInteger>
  {
    using ValueType = std::int64_t;
  };

  template <>
  struct DiagnosticArgumentTypeTraits<DiagnosticArgumentType::UnsignedInteger>
  {
    using ValueType = std::uint64_t;
  };

  template <>
  struct DiagnosticArgumentTypeTraits<DiagnosticArgumentType::Character>
  {
    using ValueType = char32_t;
  };

  template <>
  struct DiagnosticArgumentTypeTraits<DiagnosticArgumentType::String>
  {
    using ValueType = std::string;
  };

  template <>
  struct DiagnosticArgumentTypeTraits<DiagnosticArgumentType::SourceContext>
  {
    using ValueType = DiagnosticSourceContext;
  };

  template <DiagnosticArgumentName NameValue, DiagnosticArgumentType TypeValue>
  struct DiagnosticArgumentSpecification
  {
    static constexpr DiagnosticArgumentName Name = NameValue;
    static constexpr DiagnosticArgumentType Type = TypeValue;
    using ValueType = typename DiagnosticArgumentTypeTraits<TypeValue>::ValueType;
  };

  template <typename... Specifications>
  struct DiagnosticArgumentSchema
  {
  };

  template <DiagnosticKind Kind>
  struct DiagnosticTraits;

#define INK_DIAGNOSTIC_ARGUMENT(Name, Type) DiagnosticArgumentSpecification<DiagnosticArgumentName::Name, DiagnosticArgumentType::Type>
#define INK_DIAGNOSTIC_ARGUMENTS(...) DiagnosticArgumentSchema<__VA_ARGS__>
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, Schema) \
  template <>                                                                                                    \
  struct DiagnosticTraits<DiagnosticKind::Name>                                                                  \
  {                                                                                                                \
    using Arguments = Schema;                                                                                     \
  };
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
#undef INK_DIAGNOSTIC_ARGUMENTS
#undef INK_DIAGNOSTIC_ARGUMENT

  using DiagnosticArgumentValue = std::variant<bool, std::int64_t, std::uint64_t, char32_t, std::string, DiagnosticSourceContext>;

  struct DiagnosticArgument
  {
    DiagnosticArgumentName Name = DiagnosticArgumentName::Unknown;
    DiagnosticArgumentValue Value;
  };

  bool operator==(const DiagnosticArgument &Left, const DiagnosticArgument &Right);
  bool operator!=(const DiagnosticArgument &Left, const DiagnosticArgument &Right);

  struct DiagnosticRelatedInformation
  {
    DiagnosticRelatedKind Kind = DiagnosticRelatedKind::Unknown;
    SourceRange Span;
    std::vector<DiagnosticArgument> Arguments;
  };

  bool operator==(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right);
  bool operator!=(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right);

  struct Diagnostic
  {
    DiagnosticKind Kind = DiagnosticKind::Unknown;
    SourceRange Span;
    std::vector<DiagnosticArgument> Arguments;
    std::vector<DiagnosticRelatedInformation> Related;
    DiagnosticClass Class = DiagnosticClass::Unknown;

    std::uint32_t number() const noexcept;
    const char *code() const noexcept;
    DiagnosticClass classification() const noexcept;
  };

  bool operator==(const Diagnostic &Left, const Diagnostic &Right);
  bool operator!=(const Diagnostic &Left, const Diagnostic &Right);

  namespace detail
  {
    template <typename ExpectedType, typename ArgumentType>
    ExpectedType normalizeDiagnosticArgument(ArgumentType &&Value)
    {
      using ActualType = std::decay_t<ArgumentType>;
      if constexpr (std::is_same_v<ExpectedType, std::string>)
      {
        static_assert(std::is_constructible_v<std::string, ArgumentType &&>, "diagnostic string arguments must be string-like values");
        return std::string(std::forward<ArgumentType>(Value));
      }
      else if constexpr (std::is_same_v<ExpectedType, std::uint64_t>)
      {
        static_assert(std::is_integral_v<ActualType> && std::is_unsigned_v<ActualType> && !std::is_same_v<ActualType, bool>, "diagnostic unsigned integer arguments must be unsigned integral values");
        return static_cast<std::uint64_t>(Value);
      }
      else if constexpr (std::is_same_v<ExpectedType, std::int64_t>)
      {
        static_assert(std::is_integral_v<ActualType> && std::is_signed_v<ActualType>, "diagnostic signed integer arguments must be signed integral values");
        return static_cast<std::int64_t>(Value);
      }
      else
      {
        static_assert(std::is_same_v<ExpectedType, ActualType>, "diagnostic argument does not match the registered type");
        return std::forward<ArgumentType>(Value);
      }
    }

    template <typename... Specifications, typename... ArgumentTypes>
    std::vector<DiagnosticArgument> makeDiagnosticArguments(DiagnosticArgumentSchema<Specifications...>, ArgumentTypes &&...Arguments)
    {
      if constexpr (sizeof...(Specifications) == sizeof...(ArgumentTypes))
      {
        return {DiagnosticArgument{Specifications::Name, normalizeDiagnosticArgument<typename Specifications::ValueType>(std::forward<ArgumentTypes>(Arguments))}...};
      }
      else
      {
        static_assert(sizeof...(Specifications) == sizeof...(ArgumentTypes), "diagnostic argument count does not match the registered schema");
        return {};
      }
    }
  } // namespace detail

  template <DiagnosticKind Kind, typename... ArgumentTypes>
  Diagnostic makeDiagnostic(SourceRange Span, ArgumentTypes &&...Arguments)
  {
    using ArgumentSchema = typename DiagnosticTraits<Kind>::Arguments;
    return {Kind, Span, detail::makeDiagnosticArguments(ArgumentSchema{}, std::forward<ArgumentTypes>(Arguments)...), {}, DiagnosticClass::Unknown};
  }

  class DiagnosticBuilder
  {
  public:
    explicit DiagnosticBuilder(Diagnostic Result);
    DiagnosticBuilder &classification(DiagnosticClass Class) &;
    DiagnosticBuilder &&classification(DiagnosticClass Class) &&;
    DiagnosticBuilder &related(DiagnosticRelatedKind Kind, SourceRange Span, std::vector<DiagnosticArgument> Arguments = {}) &;
    DiagnosticBuilder &&related(DiagnosticRelatedKind Kind, SourceRange Span, std::vector<DiagnosticArgument> Arguments = {}) &&;
    Diagnostic build() &&;

  private:
    Diagnostic Result;
  };

  template <DiagnosticKind Kind, typename... ArgumentTypes>
  DiagnosticBuilder makeDiagnosticBuilder(SourceRange Span, ArgumentTypes &&...Arguments)
  {
    return DiagnosticBuilder(makeDiagnostic<Kind>(Span, std::forward<ArgumentTypes>(Arguments)...));
  }

  struct FormattedDiagnosticNote
  {
    std::optional<SourceRange> Span;
    std::string Message;
  };

  bool operator==(const FormattedDiagnosticNote &Left, const FormattedDiagnosticNote &Right);
  bool operator!=(const FormattedDiagnosticNote &Left, const FormattedDiagnosticNote &Right);

  struct FormattedDiagnostic
  {
    DiagnosticSeverity Severity = DiagnosticSeverity::Unknown;
    std::string Message;
    std::vector<FormattedDiagnosticNote> Notes;
  };

  bool operator==(const FormattedDiagnostic &Left, const FormattedDiagnostic &Right);
  bool operator!=(const FormattedDiagnostic &Left, const FormattedDiagnostic &Right);

  class DiagnosticFormatter
  {
  public:
    FormattedDiagnostic format(const Diagnostic &DiagnosticEntry) const;
  };

  class DiagnosticConsumer
  {
  public:
    virtual ~DiagnosticConsumer() = default;
    virtual void consume(const Diagnostic &DiagnosticEntry) = 0;
  };

  class DiagnosticEngine
  {
  public:
    DiagnosticEngine() = default;
    DiagnosticEngine(const DiagnosticEngine &) = delete;
    DiagnosticEngine &operator=(const DiagnosticEngine &) = delete;
    DiagnosticEngine(DiagnosticEngine &&) = delete;
    DiagnosticEngine &operator=(DiagnosticEngine &&) = delete;

    void addConsumer(DiagnosticConsumer &Consumer);
    void removeConsumer(DiagnosticConsumer &Consumer) noexcept;
    void report(const Diagnostic &DiagnosticEntry) const;

    template <DiagnosticKind Kind, typename... ArgumentTypes>
    void report(SourceRange Span, ArgumentTypes &&...Arguments) const
    {
      report(makeDiagnostic<Kind>(Span, std::forward<ArgumentTypes>(Arguments)...));
    }

  private:
    std::vector<DiagnosticConsumer *> Consumers;
  };

  class CollectingDiagnosticConsumer final : public DiagnosticConsumer
  {
  public:
    void consume(const Diagnostic &DiagnosticEntry) override;

    const std::vector<Diagnostic> &diagnostics() const noexcept
    {
      return Diagnostics;
    }

    std::vector<Diagnostic> takeDiagnostics() noexcept
    {
      return std::move(Diagnostics);
    }

    void clear() noexcept
    {
      Diagnostics.clear();
    }

  private:
    std::vector<Diagnostic> Diagnostics;
  };
} // namespace ink::core

#endif
