#pragma once

#include "ink/backend/vm/executable/native_symbol_provider.h"
#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
  // C selects the host C ABI; InkNative selects the host integration's Ink-native bridge contract.
  // The provider identifies the resolver and does not determine either call contract.
  enum class NativeCallKind : std::uint8_t
  {
    C,
    InkNative,
  };

  // Preserves the logical native function declaration's lookup identity and call contract for module linking.
  // ExecutableFile uses DynamicImportSymbol for both function and data imports; complete call layouts remain outside that record.
  struct NativeFunctionImport
  {
      NativeSymbolProvider Provider = NativeSymbolProvider::Native;
      core::Name Name = core::NAME_None;
      NativeCallKind CallKind = NativeCallKind::C;
  };
} // namespace ink::vm
