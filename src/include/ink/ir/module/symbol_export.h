#ifndef INK_IR_MODULE_SYMBOL_EXPORT_H
#define INK_IR_MODULE_SYMBOL_EXPORT_H

#include "ink/ir/module/symbol_id.h"

#include <vector>

namespace ink::ir
{
  class Type;
  class Value;

  // Keeps one exported declaration paired with its type; both are borrowed and must outlive this entry.
  struct SymbolExportEntry
  {
      const Type *SymbolType = nullptr;
      const Value *Declaration = nullptr;
  };

  // Groups explicitly exported declarations under one name, including overload candidates in registration order.
  struct SymbolExport
  {
      SymbolId Symbol;
      std::vector<SymbolExportEntry> Entries;
  };
} // namespace ink::ir

#endif
