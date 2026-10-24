#ifndef INK_CORE_DIAGNOSTIC_TRAITS_H
#define INK_CORE_DIAGNOSTIC_TRAITS_H

#include "ink/core/diagnostic/diagnostic.h"

namespace ink::core
{
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
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, SeverityValue, DefaultMessage, FormatPattern, Schema) \
  template <>                                                                                                     \
  struct DiagnosticTraits<DiagnosticKind::Name>                                                                   \
  {                                                                                                               \
      static constexpr DiagnosticClass Classification = DiagnosticClass::Class;                                   \
      static constexpr DiagnosticSeverity DefaultSeverity = DiagnosticSeverity::SeverityValue;                    \
      using Arguments = Schema;                                                                                   \
  };
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
#undef INK_DIAGNOSTIC_ARGUMENTS
#undef INK_DIAGNOSTIC_ARGUMENT
} // namespace ink::core

#endif
