#ifndef INK_CORE_SOURCE_ID_H
#define INK_CORE_SOURCE_ID_H

#include <cstdint>

namespace ink::core
{
  class SourceId final
  {
    public:
      constexpr SourceId() noexcept = default;

      explicit constexpr SourceId(std::uint64_t Value) noexcept
          : Value(Value)
      {
      }

      constexpr bool valid() const noexcept
      {
        return Value != 0;
      }

      constexpr std::uint64_t value() const noexcept
      {
        return Value;
      }

    private:
      std::uint64_t Value = 0;
  };

  constexpr bool operator==(SourceId Left, SourceId Right) noexcept
  {
    return Left.value() == Right.value();
  }

  constexpr bool operator!=(SourceId Left, SourceId Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::core

#endif
