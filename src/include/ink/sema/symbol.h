#ifndef INK_SEMA_SYMBOL_H
#define INK_SEMA_SYMBOL_H

#include "ink/ast/ast.h"
#include "ink/core/string_interner.h"
#include "ink/sema/ids.h"
#include "ink/type/type_id.h"

#include <cstdint>

namespace ink::sema
{
  enum class SymbolKind : std::uint8_t
  {
    Function,
    Parameter,
    Binding,
  };

  const char *symbolKindName(SymbolKind Kind) noexcept;

  struct Symbol
  {
    SymbolId Id;
    SymbolKind Kind = SymbolKind::Binding;
    core::InternedStringId Name;
    ast::AstDeclId Declaration;
    ScopeId Scope;
    type::TypeId Type;
    bool Mutable = false;
  };
} // namespace ink::sema

#endif
