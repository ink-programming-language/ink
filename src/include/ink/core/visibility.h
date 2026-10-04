#ifndef INK_CORE_VISIBILITY_H
#define INK_CORE_VISIBILITY_H

#include <cstdint>

namespace ink::core
{
  // Declaration/member visibility, independent of data access and binding mutability.
  enum class VisibilityKind : std::uint8_t
  {
    Public,
    Private,
  };
} // namespace ink::core

#endif
