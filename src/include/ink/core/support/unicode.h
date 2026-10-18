#ifndef INK_CORE_SUPPORT_UNICODE_H
#define INK_CORE_SUPPORT_UNICODE_H

namespace ink::core
{
  // Accepts Unicode code points outside the surrogate range, up to U+10FFFF.
  constexpr bool isUnicodeScalarValue(char32_t Value) noexcept
  {
    return Value <= 0x10FFFF && (Value < 0xD800 || Value > 0xDFFF);
  }
} // namespace ink::core

#endif
