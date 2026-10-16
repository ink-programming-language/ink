#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H

#include "ink/core/source/source_range.h"

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;

  // Requires a type registered in Context's IRContext; reports against Range in the currently selected source.
  // Returns a borrowed type, or nullptr on failure; a null Target propagates without another diagnostic.
  // Storage objects are non-type values and receive their own diagnostic.
  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target);

  // Validates a runtime value or storage object without loading storage, preserving its identity.
  // Type/module descriptors are semantic results but cannot be consumed as runtime operands.
  const ir::Value *requireRuntimeValue(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target);
} // namespace ink::semantic

#endif
