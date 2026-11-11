#pragma once

#include "ink/core/name/name.h"

namespace ink::vm
{
  // Identifies an unresolved host symbol independently of whether a reference calls it or takes its address.
  // Name is its unique lookup identity; call contracts and bound runtime addresses are outside this record.
  struct DynamicImportSymbol
  {
      core::Name Name = core::NAME_None;
  };
} // namespace ink::vm
