#ifndef INK_CORE_DIAGNOSTIC_BUILDER_H
#define INK_CORE_DIAGNOSTIC_BUILDER_H

#include "ink/core/diagnostic/diagnostic_traits.h"

#include <type_traits>
#include <utility>

namespace ink::core
{
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
    return {Kind, {}, Span, detail::makeDiagnosticArguments(ArgumentSchema{}, std::forward<ArgumentTypes>(Arguments)...), {}, DiagnosticClass::Unknown};
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
} // namespace ink::core

#endif
