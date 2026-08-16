#include "ink/execution/runtime_symbols.h"

#include "runtime/runtime_symbols.h"

namespace ink::execution
{
  bool registerRuntimeSymbols(NativeSymbolRegistry &Registry)
  {
    return Registry.registerSymbol("read", readRuntimeAddress()) && Registry.registerSymbol("write", writeRuntimeAddress());
  }
} // namespace ink::execution
