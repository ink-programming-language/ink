#ifndef INK_IR_MODULE_SYMBOL_REF_H
#define INK_IR_MODULE_SYMBOL_REF_H

#include "ink/ir/module/module_id.h"
#include "ink/ir/module/symbol_id.h"

namespace ink::ir
{
  // Qualifies a symbol name with its defining module; selecting a specific overload is a separate operation.
  struct SymbolRef
  {
      ModuleID Module;
      SymbolId Symbol;
  };
} // namespace ink::ir

#endif
