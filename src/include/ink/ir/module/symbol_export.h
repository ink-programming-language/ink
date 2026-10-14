#ifndef INK_IR_MODULE_SYMBOL_EXPORT_H
#define INK_IR_MODULE_SYMBOL_EXPORT_H

#include "ink/ir/module/symbol_id.h"

namespace ink::ir
{
  // Records a symbol exported by the module that owns this entry.
  struct SymbolExport
  {
      SymbolId Symbol;
  };
} // namespace ink::ir

#endif
