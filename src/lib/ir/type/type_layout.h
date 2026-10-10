#ifndef INK_IR_TYPE_LAYOUT_INTERNAL_H
#define INK_IR_TYPE_LAYOUT_INTERNAL_H

#include <cstdint>

namespace ink::ir::detail
{
  inline bool validAlignment(std::uint64_t Alignment) noexcept
  {
    return Alignment != 0 && (Alignment & (Alignment - 1)) == 0;
  }

  inline bool validLayout(std::uint64_t Size, std::uint64_t Alignment) noexcept
  {
    return validAlignment(Alignment) && Size % Alignment == 0;
  }
} // namespace ink::ir::detail

#endif
