#ifndef INK_IR_NAME_NAME_H
#define INK_IR_NAME_NAME_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace ink::ir
{
  class NamePool;

  // An index into one NamePool. Equality and hashing require the same pool.
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

  constexpr bool operator==(Name Left, Name Right) noexcept
  {
    return Left.index() == Right.index();
  }

  constexpr bool operator!=(Name Left, Name Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::ir

namespace std
{
  template <>
  struct hash<ink::ir::Name>
  {
      std::size_t operator()(ink::ir::Name Value) const noexcept
      {
        return std::hash<ink::ir::Name::IndexType>{}(Value.index());
      }
  };
} // namespace std

#endif
