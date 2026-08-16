#ifndef INK_IR_ID_H
#define INK_IR_ID_H

#include <cstddef>
#include <limits>

namespace ink::ir
{
  constexpr std::size_t InvalidId = std::numeric_limits<std::size_t>::max();

  class GlobalId
  {
  public:
    constexpr GlobalId() noexcept = default;

    explicit constexpr GlobalId(std::size_t Value) noexcept : Value(Value)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

    constexpr std::size_t value() const noexcept
    {
      return Value;
    }

    friend constexpr bool operator==(GlobalId Left, GlobalId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(GlobalId Left, GlobalId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

  class FunctionId
  {
  public:
    constexpr FunctionId() noexcept = default;

    explicit constexpr FunctionId(std::size_t Value) noexcept : Value(Value)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

    constexpr std::size_t value() const noexcept
    {
      return Value;
    }

    friend constexpr bool operator==(FunctionId Left, FunctionId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(FunctionId Left, FunctionId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

  class ValueId
  {
  public:
    constexpr ValueId() noexcept = default;

    explicit constexpr ValueId(std::size_t Value) noexcept : Value(Value)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

    constexpr std::size_t value() const noexcept
    {
      return Value;
    }

    friend constexpr bool operator==(ValueId Left, ValueId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(ValueId Left, ValueId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };
} // namespace ink::ir

#endif
