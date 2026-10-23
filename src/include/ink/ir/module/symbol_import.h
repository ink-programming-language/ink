#ifndef INK_IR_MODULE_SYMBOL_IMPORT_H
#define INK_IR_MODULE_SYMBOL_IMPORT_H

#include "ink/ir/module/symbol_ref.h"

namespace ink::ir
{
  class Type;

  // Records the qualified target and borrowed type of an imported symbol; the type must outlive this record.
  struct SymbolImport
  {
      SymbolRef Target;
      const Type *SymbolType = nullptr;
  };
} // namespace ink::ir

#endif
