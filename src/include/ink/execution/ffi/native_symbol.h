#ifndef INK_EXECUTION_FFI_NATIVE_SYMBOL_H
#define INK_EXECUTION_FFI_NATIVE_SYMBOL_H

#include <string_view>

namespace ink::execution
{
  // An untyped function address for an ABI-aware caller such as libffi.
  using NativeSymbol = void (*)();

  // Finds the exact name in already loaded process modules without loading a
  // library or applying aliases. The defining module must remain loaded while
  // the caller uses the result. Missing or invalid names return null.
  NativeSymbol findNativeSymbol(std::string_view Name);
} // namespace ink::execution

#endif
