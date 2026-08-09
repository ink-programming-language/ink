#ifndef INK_IR_IDS_H
#define INK_IR_IDS_H

#include <cstddef>
#include <cstdint>
#include <limits>

namespace ink::ir
{
  template <typename Tag>
  class IrId
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr IrId() noexcept = default;

    static constexpr IrId fromValue(ValueType Value) noexcept
    {
      return IrId(Value);
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
    explicit constexpr IrId(ValueType Value) noexcept : Value(Value)
    {
    }

    ValueType Value = InvalidValue;
  };

  template <typename Tag>
  constexpr bool operator==(IrId<Tag> Left, IrId<Tag> Right) noexcept
  {
    return Left.value() == Right.value();
  }

  template <typename Tag>
  constexpr bool operator!=(IrId<Tag> Left, IrId<Tag> Right) noexcept
  {
    return !(Left == Right);
  }

  template <typename Tag>
  constexpr bool operator<(IrId<Tag> Left, IrId<Tag> Right) noexcept
  {
    return Left.value() < Right.value();
  }

  using IrTypeId = IrId<struct IrTypeIdTag>;
  using IrConstantId = IrId<struct IrConstantIdTag>;
  using IrOriginId = IrId<struct IrOriginIdTag>;
  using IrFunctionId = IrId<struct IrFunctionIdTag>;
  using IrBlockId = IrId<struct IrBlockIdTag>;
  using IrOperationId = IrId<struct IrOperationIdTag>;
  using IrValueId = IrId<struct IrValueIdTag>;
  using IrPlanNodeId = IrId<struct IrPlanNodeIdTag>;

  struct IrTableRange
  {
    std::uint32_t First = 0;
    std::uint32_t Count = 0;

    constexpr bool empty() const noexcept
    {
      return Count == 0;
    }

    constexpr std::uint32_t end() const noexcept
    {
      return First + Count;
    }

    constexpr bool contains(std::uint32_t Index) const noexcept
    {
      return Index >= First && Index < end();
    }
  };

  constexpr bool operator==(IrTableRange Left, IrTableRange Right) noexcept
  {
    return Left.First == Right.First && Left.Count == Right.Count;
  }

  constexpr bool operator!=(IrTableRange Left, IrTableRange Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::ir

#endif
