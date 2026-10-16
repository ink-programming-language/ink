#ifndef INK_CORE_CONFIG_NUMERIC_LITERAL_H
#define INK_CORE_CONFIG_NUMERIC_LITERAL_H

#include <cstddef>

namespace ink::core
{
  enum class NumericBase : unsigned
  {
    Binary = 2,
    Octal = 8,
    Decimal = 10,
    Hexadecimal = 16,
  };

  // Binary, octal and hexadecimal literals start with a two-byte ASCII prefix.
  inline constexpr std::size_t NumericBasePrefixLength = 2;
} // namespace ink::core

#endif
