#ifndef INK_TYPE_TYPE_ID_H
#define INK_TYPE_TYPE_ID_H

#include <cstdint>
#include <functional>
#include <limits>

namespace ink::type
{
  class TypeId
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr TypeId() noexcept = default;

    static constexpr TypeId fromValue(ValueType Value) noexcept
    {
      return TypeId(Value);
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
    explicit constexpr TypeId(ValueType Value) noexcept : Value(Value)
    {
    }

    ValueType Value = InvalidValue;
  };

  constexpr bool operator==(TypeId Left, TypeId Right) noexcept
  {
    return Left.value() == Right.value();
  }

  constexpr bool operator!=(TypeId Left, TypeId Right) noexcept
  {
    return !(Left == Right);
  }

  constexpr bool operator<(TypeId Left, TypeId Right) noexcept
  {
    return Left.value() < Right.value();
  }

  constexpr bool operator<=(TypeId Left, TypeId Right) noexcept
  {
    return !(Right < Left);
  }

  constexpr bool operator>(TypeId Left, TypeId Right) noexcept
  {
    return Right < Left;
  }

  constexpr bool operator>=(TypeId Left, TypeId Right) noexcept
  {
    return !(Left < Right);
  }
} // namespace ink::type

namespace std
{
  template <>
  struct hash<ink::type::TypeId>
  {
    size_t operator()(ink::type::TypeId Id) const noexcept
    {
      return hash<ink::type::TypeId::ValueType>{}(Id.value());
    }
  };
} // namespace std

#endif
