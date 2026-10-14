#ifndef INK_IR_MODULE_SYMBOL_ID_H
#define INK_IR_MODULE_SYMBOL_ID_H

#include "ink/core/name/name.h"

namespace ink::ir
{
  // Identifies a symbol within one module by its canonical name.
  struct SymbolId
  {
      core::Name SymbolName;
  };
} // namespace ink::ir

#endif
