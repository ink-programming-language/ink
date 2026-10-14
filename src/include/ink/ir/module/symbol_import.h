#ifndef INK_IR_MODULE_SYMBOL_IMPORT_H
#define INK_IR_MODULE_SYMBOL_IMPORT_H

#include "ink/ir/module/symbol_ref.h"

namespace ink::ir
{
  // Records the qualified target of an imported symbol.
  struct SymbolImport
  {
      SymbolRef Target;
  };
} // namespace ink::ir

#endif
