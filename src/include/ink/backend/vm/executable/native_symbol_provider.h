#pragma once

#include <cstdint>

namespace ink::vm
{
  // Selects the host resolver category independently of a symbol's call contract or ABI.
  enum class NativeSymbolProvider : std::uint8_t
  {
    Native,
    SystemApi,
  };
} // namespace ink::vm
