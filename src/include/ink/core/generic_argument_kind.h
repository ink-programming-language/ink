#ifndef INK_CORE_GENERIC_ARGUMENT_KIND_H
#define INK_CORE_GENERIC_ARGUMENT_KIND_H

#include <cstdint>

namespace ink::core
{
  // Closed generic arguments identify either a type or a constant value of a type.
  enum class GenericArgumentKind : std::uint8_t
  {
    Type,
    Value,
  };
} // namespace ink::core

#endif
