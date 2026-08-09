#ifndef INK_SEMA_SCOPE_H
#define INK_SEMA_SCOPE_H

#include "ink/ast/ast.h"
#include "ink/sema/ids.h"

#include <optional>
#include <vector>

namespace ink::sema
{
  struct Scope
  {
    ScopeId Id;
    std::optional<ScopeId> Parent;
    ast::AstNodeRef Owner;
    std::vector<SymbolId> Symbols;
  };
} // namespace ink::sema

#endif
