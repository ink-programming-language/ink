#ifndef INK_CORE_SOURCE_RANGE_H
#define INK_CORE_SOURCE_RANGE_H

#include <cstddef>

namespace ink::core
{
  struct SourceRange
  {
    std::size_t Start = 0;
    std::size_t End = 0;

    constexpr std::size_t size() const noexcept
    {
      return End - Start;
    }

    constexpr bool empty() const noexcept
    {
      return Start == End;
    }
  };

  constexpr bool operator==(SourceRange Left, SourceRange Right) noexcept
  {
    return Left.Start == Right.Start && Left.End == Right.End;
  }

  constexpr bool operator!=(SourceRange Left, SourceRange Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::core

#endif
