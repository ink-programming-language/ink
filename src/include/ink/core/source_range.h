#ifndef INK_CORE_SOURCE_RANGE_H
#define INK_CORE_SOURCE_RANGE_H

#include "ink/core/source_location.h"

namespace ink::core
{
  // Unlike Clang's token ranges, Ink ranges always denote bytes in [Start, End).
  class SourceRange
  {
    public:
      constexpr SourceRange() noexcept = default;

      explicit constexpr SourceRange(SourceLocation Location) noexcept
          : Start(Location),
            End(Location)
      {
      }

      constexpr SourceRange(SourceLocation Start, SourceLocation End) noexcept
          : Start(Start),
            End(End)
      {
      }

      static constexpr SourceRange fromByteOffsets(std::size_t Start, std::size_t End) noexcept
      {
        return {SourceLocation::fromByteOffset(Start), SourceLocation::fromByteOffset(End)};
      }

      constexpr SourceLocation getBegin() const noexcept
      {
        return Start;
      }

      constexpr SourceLocation getEnd() const noexcept
      {
        return End;
      }

      constexpr void setBegin(SourceLocation Location) noexcept
      {
        Start = Location;
      }

      constexpr void setEnd(SourceLocation Location) noexcept
      {
        End = Location;
      }

      constexpr bool isValid() const noexcept
      {
        return Start.isValid() && End.isValid() && Start <= End;
      }

      constexpr bool isInvalid() const noexcept
      {
        return !isValid();
      }

      constexpr std::size_t size() const noexcept
      {
        assert(isValid());
        return End.getByteOffset() - Start.getByteOffset();
      }

      constexpr bool empty() const noexcept
      {
        return isValid() && Start == End;
      }

      constexpr bool fullyContains(SourceRange Other) const noexcept
      {
        return isValid() && Other.isValid() && Start <= Other.Start && Other.End <= End;
      }

    private:
      SourceLocation Start;
      SourceLocation End;
  };

  constexpr bool operator==(SourceRange Left, SourceRange Right) noexcept
  {
    return Left.getBegin() == Right.getBegin() && Left.getEnd() == Right.getEnd();
  }

  constexpr bool operator!=(SourceRange Left, SourceRange Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::core

#endif
