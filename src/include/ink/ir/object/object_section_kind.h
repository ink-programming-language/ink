#pragma once

#include <cstdint>

namespace ink::ir
{
  enum class ObjectSectionKind : std::uint8_t
  {
    Code,
    Constants,
    GlobalStorage,
  };
} // namespace ink::ir
