#ifndef INK_CORE_FUNCTION_BINDING_H
#define INK_CORE_FUNCTION_BINDING_H

#include <cstdint>

namespace ink::core
{
  // Native symbol direction, independent of language linkage and source visibility.
  enum class FunctionBinding : std::uint8_t
  {
    Local,
    Import,
    Export,
  };
} // namespace ink::core

#endif
