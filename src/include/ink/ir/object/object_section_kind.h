#pragma once

#include <cstdint>

namespace ink::ir
{
  enum class ObjectSectionKind : std::uint8_t
  {
    Code = 0,
    Constants = 1,
    GlobalStorage = 2,
  };
} // namespace ink::ir
