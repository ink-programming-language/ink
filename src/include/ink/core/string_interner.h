#ifndef INK_CORE_STRING_INTERNER_H
#define INK_CORE_STRING_INTERNER_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ink::core
{
  class InternedStringId
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr InternedStringId() noexcept = default;

    static constexpr InternedStringId fromValue(ValueType Value) noexcept
    {
      return InternedStringId(Value);
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
    explicit constexpr InternedStringId(ValueType Value) noexcept : Value(Value)
    {
    }

    ValueType Value = InvalidValue;
  };

  constexpr bool operator==(InternedStringId Left, InternedStringId Right) noexcept
  {
    return Left.value() == Right.value();
  }

  constexpr bool operator!=(InternedStringId Left, InternedStringId Right) noexcept
  {
    return !(Left == Right);
  }

  constexpr bool operator<(InternedStringId Left, InternedStringId Right) noexcept
  {
    return Left.value() < Right.value();
  }

  constexpr bool operator<=(InternedStringId Left, InternedStringId Right) noexcept
  {
    return !(Right < Left);
  }

  constexpr bool operator>(InternedStringId Left, InternedStringId Right) noexcept
  {
    return Right < Left;
  }

  constexpr bool operator>=(InternedStringId Left, InternedStringId Right) noexcept
  {
    return !(Left < Right);
  }

  class StringInterner
  {
  public:
    StringInterner() = default;
    StringInterner(const StringInterner &) = delete;
    StringInterner &operator=(const StringInterner &) = delete;
    StringInterner(StringInterner &&) = delete;
    StringInterner &operator=(StringInterner &&) = delete;

    InternedStringId intern(std::string_view Value);
    bool contains(InternedStringId Id) const noexcept;
    std::string_view string(InternedStringId Id) const;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

  private:
    std::deque<std::string> Strings;
    std::unordered_map<std::string_view, InternedStringId> StringIds;
  };
} // namespace ink::core

namespace std
{
  template <>
  struct hash<ink::core::InternedStringId>
  {
    size_t operator()(ink::core::InternedStringId Id) const noexcept
    {
      return hash<ink::core::InternedStringId::ValueType>{}(Id.value());
    }
  };
} // namespace std

#endif
