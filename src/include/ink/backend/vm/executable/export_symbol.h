#pragma once

#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
  // Names an exported image address. Offset is relative to the beginning of Image, never a host pointer.
  struct ExportSymbol
  {
      core::Name Name = core::NAME_None;
      std::uint64_t Offset = 0;
  };
} // namespace ink::vm
