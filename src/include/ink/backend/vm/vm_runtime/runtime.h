#pragma once

#include "ink/core/name/name.h"

#include <array>

namespace ink::vm
{
  struct NativeSymbolAddress
  {
      core::Name Name = core::NAME_None;
      void *Address = nullptr;
  };

  extern const std::array<NativeSymbolAddress, 1> NativeSymbolAddresses;
} // namespace ink::vm
