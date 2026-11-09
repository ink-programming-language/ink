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

  // Describes an unresolved host function, referenced by a u32 NativeFunctions index in future callNative instructions.
  // Function bindings are not image address relocations. Complete C signatures and call-area layouts are outside this model.
  struct NativeFunctionImport
  {
      NativeSymbolProvider Provider = NativeSymbolProvider::Native;
      core::Name Name = core::NAME_None;
      NativeCallKind CallKind = NativeCallKind::C;
  };
} // namespace ink::vm
