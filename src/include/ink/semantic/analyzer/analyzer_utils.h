#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_UTILS_H

#include "ink/core/source/source_range.h"

#include <string_view>

namespace ink::ir
{
  class Type;
  class TypePool;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;

  // Returns a borrowed builtin type, or nullptr for an unknown spelling; scope lookup belongs to the caller.
  const ir::Type *lookupBuiltinType(const ir::TypePool &Types, std::string_view Name);

  // Requires a type registered in Context's IRContext; reports against Range in the currently selected source.
  // Returns a borrowed type, or nullptr on failure; a null Target propagates without another diagnostic.
  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target);
} // namespace ink::semantic

#endif
