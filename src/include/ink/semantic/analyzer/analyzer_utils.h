#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H

#include "ink/core/diagnostic/diagnostic.h"
#include "ink/core/source/source_range.h"
#include "ink/core/support/return_macros.h"

#include <string_view>

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;

  // Returns a builtin type, boolean or null pointer constant from Context, or nullptr for an unknown name.
  const ir::Value *lookupBuiltinValue(SemanticContext &Context, std::string_view Name);

  // Requires a type registered in Context's IRContext; reports against Range in the currently selected source.
  // Returns a borrowed type, or nullptr on failure; a null Target propagates without another diagnostic.
  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target);

  // Validates an emitted runtime value, preserving its identity.
  // Storage and type/module descriptors are semantic results but cannot be consumed as runtime operands.
  const ir::Value *requireRuntimeValue(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target);
} // namespace ink::semantic

// Evaluates Value once; report arguments start with the source range and are evaluated only when Value is false.
#define RETURN_REPORT_IF_FALSE(Context, Value, Kind, FailureValue, ...) \
  do \
  { \
    if (!(Value)) \
    { \
      (Context).template report<Kind>(__VA_ARGS__); \
      return (FailureValue); \
    } \
  } while (false)

// Evaluates Value once; report arguments start with the source range and are evaluated only when Value is true.
#define RETURN_REPORT_IF(Context, Value, Kind, FailureValue, ...) \
  do \
  { \
    if (Value) \
    { \
      (Context).template report<Kind>(__VA_ARGS__); \
      return (FailureValue); \
    } \
  } while (false)

// Evaluates Value once; the unsupported feature's source range and description are evaluated only on failure.
#define UNSUPPORT_IF(State, Value, FailureValue, ...) \
  do \
  { \
    if (Value) \
    { \
      (State).semanticContext().template report<::ink::core::DiagnosticKind::SemanticUnsupported>(__VA_ARGS__); \
      return (FailureValue); \
    } \
  } while (false)

// Reports invalid names against the supplied source range and returns the handler's failure value.
#define CHECK_VALID_NAME(Context, Range, NameValue, FailureValue) \
  do \
  { \
    if (!(NameValue).valid()) \
    { \
      (Context).template report<::ink::core::DiagnosticKind::SemanticConstructionFailed>((Range)); \
      return (FailureValue); \
    } \
  } while (false)

// Reports invalid hints against the supplied source range and returns an empty result from the handler.
#define CHECK_VALID_TYPE(Context, TypeValue, Range) \
  do \
  { \
    auto &InkCheckValidTypeContext = (Context); \
    const ::ink::ir::Type *InkCheckValidTypeValue = (TypeValue); \
    if (InkCheckValidTypeValue && !InkCheckValidTypeContext.irContext().containsType(InkCheckValidTypeValue)) \
    { \
      InkCheckValidTypeContext.template report<::ink::core::DiagnosticKind::SemanticConstructionFailed>((Range)); \
      return {}; \
    } \
  } while (false)

// Checks the supplied type class and reports its description on mismatch, then returns an empty result from the handler.
#define EXPECTED_TYPE(Context, Range, TypeValue, TypeClass, Description) \
  do \
  { \
    const ::ink::ir::Type *InkCheckExpectedType = (TypeValue); \
    if (InkCheckExpectedType && !TypeClass::classof(InkCheckExpectedType)) \
    { \
      (Context).template report<::ink::core::DiagnosticKind::SemanticTypeMismatch>((Range), (Description), InkCheckExpectedType->toString()); \
      return {}; \
    } \
  } while (false)

// Requires a non-null actual type pointer; reports a mismatched non-null expectation and returns the supplied failure value.
#define CHECK_TYPE_MATCH(Context, Range, ExpectedType, ActualType, FailureValue) \
  do \
  { \
    const ::ink::ir::Type *InkCheckTypeMatchExpected = (ExpectedType); \
    const ::ink::ir::Type *InkCheckTypeMatchActual = (ActualType); \
    if (InkCheckTypeMatchExpected && InkCheckTypeMatchExpected != InkCheckTypeMatchActual) \
    { \
      (Context).template report<::ink::core::DiagnosticKind::SemanticTypeMismatch>((Range), InkCheckTypeMatchExpected->toString(), InkCheckTypeMatchActual->toString()); \
      return (FailureValue); \
    } \
  } while (false)

// Requires a non-null type pointer; reports missing layout and returns the supplied failure value.
#define CHECK_HAS_LAYOUT(Context, Range, TypeValue, Description, FailureValue) \
  do \
  { \
    const ::ink::ir::Type *InkCheckLayoutType = (TypeValue); \
    if (!InkCheckLayoutType->hasLayout()) \
    { \
      (Context).template report<::ink::core::DiagnosticKind::SemanticTypeMismatch>((Range), (Description), InkCheckLayoutType->toString()); \
      return (FailureValue); \
    } \
  } while (false)

// Uses the supplied source range and the handler's failure return value, such as nullptr or false.
#define CHECK_RECURSION_DEPTH(State, Range, FailureValue) \
  do \
  { \
    auto &InkCheckRecursionState = (State); \
    if (InkCheckRecursionState.recursionDepth() >= InkCheckRecursionState.recursionDepthLimit()) \
    { \
      InkCheckRecursionState.semanticContext().template report<::ink::core::DiagnosticKind::SemanticNestingLimit>((Range)); \
      return (FailureValue); \
    } \
  } while (false)

#endif
