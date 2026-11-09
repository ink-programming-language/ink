#pragma once

#include "ink/core/name/name.h"

namespace ink::vm
{
  // Describes unresolved host data without storing its bound runtime address.
  struct NativeDataImport
  {
      core::Name Name = core::NAME_None;
  };
} // namespace ink::vm
