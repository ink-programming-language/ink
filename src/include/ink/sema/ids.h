#ifndef INK_SEMA_IDS_H
#define INK_SEMA_IDS_H

#include <cstdint>
#include <functional>
#include <limits>

namespace ink::sema
{
  template <typename Tag>
  class SemanticId
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr SemanticId() noexcept = default;

    static constexpr SemanticId fromValue(ValueType Value) noexcept
    {
      return SemanticId(Value);
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
    explicit constexpr SemanticId(ValueType Value) noexcept : Value(Value)
    {
    }

    ValueType Value = InvalidValue;
  };

  template <typename Tag>
  constexpr bool operator==(SemanticId<Tag> Left, SemanticId<Tag> Right) noexcept
  {
    return Left.value() == Right.value();
  }

  template <typename Tag>
  constexpr bool operator!=(SemanticId<Tag> Left, SemanticId<Tag> Right) noexcept
  {
    return !(Left == Right);
  }

  template <typename Tag>
  constexpr bool operator<(SemanticId<Tag> Left, SemanticId<Tag> Right) noexcept
  {
    return Left.value() < Right.value();
  }

  using SymbolId = SemanticId<struct SymbolIdTag>;
  using ScopeId = SemanticId<struct ScopeIdTag>;
} // namespace ink::sema

namespace std
{
  template <typename Tag>
  struct hash<ink::sema::SemanticId<Tag>>
  {
    size_t operator()(ink::sema::SemanticId<Tag> Id) const noexcept
    {
      return hash<typename ink::sema::SemanticId<Tag>::ValueType>{}(Id.value());
    }
  };
} // namespace std

#endif
