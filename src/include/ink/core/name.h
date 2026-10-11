#ifndef INK_CORE_NAME_H
#define INK_CORE_NAME_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace ink::core
{
  class NamePool;

  // An index into the process-wide NamePool, shared by all frontend contexts.
  class Name final
  {
    public:
      using IndexType = std::uint32_t;
      static constexpr IndexType InvalidIndex = std::numeric_limits<IndexType>::max();

      constexpr Name() noexcept = default;

      constexpr bool valid() const noexcept
      {
        return Index != InvalidIndex;
      }

      constexpr IndexType index() const noexcept
      {
        return Index;
      }

    private:
      explicit constexpr Name(IndexType Index) noexcept
          : Index(Index)
      {
      }

      IndexType Index = InvalidIndex;

      friend class NamePool;
  };

  // The predefined invalid name, with the same InvalidIndex as a default Name.
  inline constexpr Name NAME_None{};

  constexpr bool operator==(Name Left, Name Right) noexcept
  {
    return Left.index() == Right.index();
  }

  constexpr bool operator!=(Name Left, Name Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::core

namespace std
{
  template <>
  struct hash<ink::core::Name>
  {
      std::size_t operator()(ink::core::Name Value) const noexcept
      {
        return std::hash<ink::core::Name::IndexType>{}(Value.index());
      }
  };
} // namespace std

#endif
