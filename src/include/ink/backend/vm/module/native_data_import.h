#pragma once

#include "ink/core/name/name.h"

namespace ink::vm
{
  // Preserves a logical native data declaration's lookup name for module linking without a bound runtime address.
  struct NativeDataImport
  {
      core::Name Name = core::NAME_None;
  };
} // namespace ink::vm
