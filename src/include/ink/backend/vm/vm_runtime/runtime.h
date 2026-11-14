#pragma once

#include "ink/core/name/name.h"
#include "ink/core/config.h"

#include <array>

namespace ink::vm
{
  struct NativeSymbolAddress
  {
      core::Name Name = core::NAME_None;
      void *Address = nullptr;
  };

  extern const std::array<NativeSymbolAddress, INK_PLATFORM_WINDOWS ? 3 : 1> NativeSymbolAddresses;
} // namespace ink::vm
