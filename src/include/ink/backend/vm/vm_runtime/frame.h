#pragma once

#include <cstdint>

namespace ink::vm
{
  // Native stubs receive the caller's validated call area without a VM BP/PC save area.
  struct Frame
  {
      // The first free byte after the return and argument slots; stubs leave it unchanged.
      std::uint8_t *SP = nullptr;
  };
} // namespace ink::vm
