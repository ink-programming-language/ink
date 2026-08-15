#ifndef INK_EXECUTION_RUNTIME_H
#define INK_EXECUTION_RUNTIME_H

#include "ink/execution/native_symbol_registry.h"

namespace ink::execution
{
  bool registerRuntimeSymbols(NativeSymbolRegistry &Registry);
} // namespace ink::execution

#endif
