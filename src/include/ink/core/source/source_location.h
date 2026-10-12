#ifndef INK_CORE_SOURCE_LOCATION_H
#define INK_CORE_SOURCE_LOCATION_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace ink::core
{
  // Clang-style compact, opaque value. Ink has no macro locations. Offsets are
  // local to the original SourceBuffer; the buffer/diagnostic supplies its ID.
  class SourceLocation
  {
    public:
      using UIntTy = std::uint32_t;
      using IntTy = std::int64_t;
      static constexpr std::size_t MaxByteOffset = std::numeric_limits<UIntTy>::max() - 1;

      constexpr SourceLocation() noexcept = default;

      constexpr bool isValid() const noexcept
      {
        return Id != 0;
      }

      constexpr bool isInvalid() const noexcept
      {
        return !isValid();
      }

      static constexpr SourceLocation fromByteOffset(std::size_t Offset) noexcept
      {
        return Offset <= MaxByteOffset ? getFromRawEncoding(static_cast<UIntTy>(Offset + 1)) : SourceLocation{};
      }

      constexpr std::size_t getByteOffset() const noexcept
      {
        assert(isValid());
        return static_cast<std::size_t>(Id) - 1;
      }

      constexpr SourceLocation getLocWithOffset(IntTy Offset) const noexcept
      {
        if (isInvalid() || Offset < -static_cast<IntTy>(getByteOffset()) || Offset > static_cast<IntTy>(MaxByteOffset - getByteOffset()))
        {
          return {};
        }
        return fromByteOffset(static_cast<std::size_t>(static_cast<IntTy>(getByteOffset()) + Offset));
      }

      constexpr UIntTy getRawEncoding() const noexcept
      {
        return Id;
      }

      static constexpr SourceLocation getFromRawEncoding(UIntTy Encoding) noexcept
      {
        SourceLocation Result;
        Result.Id = Encoding;
        return Result;
      }

      constexpr UIntTy getHashValue() const noexcept
      {
        return Id;
      }

    private:
      UIntTy Id = 0;
  };

  constexpr bool operator==(SourceLocation Left, SourceLocation Right) noexcept
  {
    return Left.getRawEncoding() == Right.getRawEncoding();
  }

  constexpr bool operator!=(SourceLocation Left, SourceLocation Right) noexcept
  {
    return !(Left == Right);
  }

  // Ordering is meaningful only within the same source buffer.
  constexpr bool operator<(SourceLocation Left, SourceLocation Right) noexcept
  {
    return Left.getRawEncoding() < Right.getRawEncoding();
  }

  constexpr bool operator>(SourceLocation Left, SourceLocation Right) noexcept
  {
    return Right < Left;
  }

  constexpr bool operator<=(SourceLocation Left, SourceLocation Right) noexcept
  {
    return !(Right < Left);
  }

  constexpr bool operator>=(SourceLocation Left, SourceLocation Right) noexcept
  {
    return !(Left < Right);
  }
} // namespace ink::core

#endif
