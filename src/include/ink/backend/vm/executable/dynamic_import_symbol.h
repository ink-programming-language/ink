#pragma once

#include "ink/backend/vm/executable/native_symbol_provider.h"
#include "ink/core/name/name.h"

namespace ink::vm
{
  // Identifies an unresolved host symbol independently of whether a reference calls it or takes its address.
  // Provider and Name form its identity; call contracts and bound runtime addresses are outside this record.
  struct DynamicImportSymbol
  {
      NativeSymbolProvider Provider = NativeSymbolProvider::Native;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::vm
