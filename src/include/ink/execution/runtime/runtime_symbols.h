#ifndef INK_EXECUTION_RUNTIME_SYMBOLS_H
#define INK_EXECUTION_RUNTIME_SYMBOLS_H

#include "ink/execution/runtime/native_symbol_registry.h"

namespace ink::execution
{
  bool registerRuntimeSymbols(NativeSymbolRegistry &Registry);
} // namespace ink::execution

#endif
