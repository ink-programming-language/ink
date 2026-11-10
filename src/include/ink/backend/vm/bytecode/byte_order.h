#pragma once

#include <cstdint>

namespace ink::vm
{
  enum class ByteOrder : std::uint8_t
  {
    Little,
    Big,
  };
} // namespace ink::vm
