#ifndef INK_CORE_SOURCE_FILE_ID_H
#define INK_CORE_SOURCE_FILE_ID_H

#include <cstdint>
#include <functional>
#include <limits>

namespace ink::core
{
  class SourceFileId
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr SourceFileId() noexcept = default;

    static constexpr SourceFileId fromValue(ValueType Value) noexcept
    {
      return SourceFileId(Value);
    }

    constexpr ValueType value() const noexcept
    {
      return Value;
    }

    constexpr bool isValid() const noexcept
    {
      return Value != InvalidValue;
    }

    explicit constexpr operator bool() const noexcept
    {
      return isValid();
    }

  private:
    explicit constexpr SourceFileId(ValueType Value) noexcept : Value(Value)
    {
    }

    ValueType Value = InvalidValue;
  };

  constexpr bool operator==(SourceFileId Left, SourceFileId Right) noexcept
  {
    return Left.value() == Right.value();
  }

  constexpr bool operator!=(SourceFileId Left, SourceFileId Right) noexcept
  {
    return !(Left == Right);
  }

  constexpr bool operator<(SourceFileId Left, SourceFileId Right) noexcept
  {
    return Left.value() < Right.value();
  }

  constexpr bool operator<=(SourceFileId Left, SourceFileId Right) noexcept
  {
    return !(Right < Left);
  }

  constexpr bool operator>(SourceFileId Left, SourceFileId Right) noexcept
  {
    return Right < Left;
  }

  constexpr bool operator>=(SourceFileId Left, SourceFileId Right) noexcept
  {
    return !(Left < Right);
  }
} // namespace ink::core

namespace std
{
  template <>
  struct hash<ink::core::SourceFileId>
  {
    size_t operator()(ink::core::SourceFileId Id) const noexcept
    {
      return hash<ink::core::SourceFileId::ValueType>{}(Id.value());
    }
  };
} // namespace std

#endif
