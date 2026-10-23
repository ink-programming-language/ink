#ifndef INK_IR_MODULE_SYMBOL_EXPORT_H
#define INK_IR_MODULE_SYMBOL_EXPORT_H

#include "ink/ir/module/symbol_id.h"

namespace ink::ir
{
  class Type;

  // Records an exported symbol and its borrowed type; the type must outlive this record.
  struct SymbolExport
  {
      SymbolId Symbol;
      const Type *SymbolType = nullptr;
  };
} // namespace ink::ir

#endif
