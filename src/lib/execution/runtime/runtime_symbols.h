#ifndef INK_EXECUTION_RUNTIME_SYMBOL_ADDRESSES_H
#define INK_EXECUTION_RUNTIME_SYMBOL_ADDRESSES_H

#include "ink/execution/runtime/native_symbol_registry.h"

namespace ink::execution
{
  NativeFunctionAddress readRuntimeAddress() noexcept;
  NativeFunctionAddress writeRuntimeAddress() noexcept;
} // namespace ink::execution

#endif
