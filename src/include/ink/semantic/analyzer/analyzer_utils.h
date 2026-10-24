#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H

#include "ink/core/diagnostic/diagnostic.h"
#include "ink/core/support/return_macros.h"

#include <string_view>

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class AnalysisSite;
  class SemanticContext;

  // Returns a builtin type, boolean or null pointer constant from Context, or nullptr for an unknown name.
  const ir::Value *lookupBuiltinValue(SemanticContext &Context, std::string_view Name);

  // Requires a type registered in Site's IRContext; reports against its captured source and range.
  // Returns a borrowed type, or nullptr on failure; a null Target propagates without another diagnostic.
  const ir::Type *requireType(const AnalysisSite &Site, const ir::Value *Target);

  // Validates an emitted runtime value, preserving its identity.
  // Storage and type/module descriptors are semantic results but cannot be consumed as runtime operands.
  // Reports against Site's captured source and range without changing the context's selected source.
  const ir::Value *requireRuntimeValue(const AnalysisSite &Site, const ir::Value *Target);
} // namespace ink::semantic

// Always evaluates Condition once; Site and diagnostic arguments are evaluated only on failure, which never returns.
#define CHECK_ICE(Site, Condition, Kind, ...) \
  do \
  { \
    if (!(Condition)) \
    { \
      (Site).template reportICE<Kind>(__VA_ARGS__); \
    } \
  } while (false)

// Evaluates Value once; Site and diagnostic arguments are evaluated only when Value is false.
#define RETURN_ERROR_IF_NOT(Site, Value, Kind, FailureValue, ...) \
  do \
  { \
    if (!(Value)) \
    { \
      (Site).template reportError<Kind>(__VA_ARGS__); \
      return (FailureValue); \
    } \
  } while (false)

// Evaluates Value once; Site and diagnostic arguments are evaluated only when Value is true.
#define RETURN_ERROR_IF(Site, Value, Kind, FailureValue, ...) \
  do \
  { \
    if (Value) \
    { \
      (Site).template reportError<Kind>(__VA_ARGS__); \
      return (FailureValue); \
    } \
  } while (false)

// Evaluates Value once; Site and diagnostic arguments are evaluated only when Value is true, and analysis continues.
#define WARNING_IF(Site, Value, Kind, ...) \
  do \
  { \
    if (Value) \
    { \
      (Site).template reportWarning<Kind>(__VA_ARGS__); \
    } \
  } while (false)

// Evaluates Value once; Site and diagnostic arguments are evaluated only when Value is false, and analysis continues.
#define WARNING_IF_NOT(Site, Value, Kind, ...) \
  do \
  { \
    if (!(Value)) \
    { \
      (Site).template reportWarning<Kind>(__VA_ARGS__); \
    } \
  } while (false)

// Checks the supplied type class and reports its description on mismatch, then returns an empty result from the handler.
#define EXPECTED_TYPE(Site, TypeValue, TypeClass, Description) \
  do \
  { \
    const ::ink::ir::Type *InkCheckExpectedType = (TypeValue); \
    if (InkCheckExpectedType && !TypeClass::classof(InkCheckExpectedType)) \
    { \
      (Site).template reportError<::ink::core::DiagnosticKind::SemanticTypeMismatch>((Description), InkCheckExpectedType->toString()); \
      return {}; \
    } \
  } while (false)

// Requires a non-null actual type pointer; reports a mismatched non-null expectation and returns the supplied failure value.
#define CHECK_TYPE_MATCH(Site, ExpectedType, ActualType, FailureValue) \
  do \
  { \
    const ::ink::ir::Type *InkCheckTypeMatchExpected = (ExpectedType); \
    const ::ink::ir::Type *InkCheckTypeMatchActual = (ActualType); \
    if (InkCheckTypeMatchExpected && InkCheckTypeMatchExpected != InkCheckTypeMatchActual) \
    { \
      (Site).template reportError<::ink::core::DiagnosticKind::SemanticTypeMismatch>(InkCheckTypeMatchExpected->toString(), InkCheckTypeMatchActual->toString()); \
      return (FailureValue); \
    } \
  } while (false)

// Requires a non-null type pointer; reports missing layout and returns the supplied failure value.
#define CHECK_HAS_LAYOUT(Site, TypeValue, Description, FailureValue) \
  do \
  { \
    const ::ink::ir::Type *InkCheckLayoutType = (TypeValue); \
    if (!InkCheckLayoutType->hasLayout()) \
    { \
      (Site).template reportError<::ink::core::DiagnosticKind::SemanticTypeMismatch>((Description), InkCheckLayoutType->toString()); \
      return (FailureValue); \
    } \
  } while (false)

#endif
