#ifndef INK_IR_VALUE_H
#define INK_IR_VALUE_H

#include "ink/ir/id.h"
#include "ink/ir/type.h"

#include <cstddef>
#include <cstdint>

namespace ink::ir
{
  enum class ValueKind : std::uint8_t
  {
#define INK_IR_VALUE(Name) Name,
#include "ink/ir/ir.def"
  };

  const char *valueKindName(ValueKind Kind) noexcept;

  class Value
  {
  public:
    virtual ~Value() = 0;

    ValueKind kind() const noexcept
    {
      return Kind;
    }

    const Type &type() const noexcept
    {
      return *ValueType;
    }

  private:
    Value(ValueKind Kind, const Type &ValueType) noexcept : Kind(Kind), ValueType(&ValueType)
    {
    }

    ValueKind Kind;
    const Type *ValueType;

#define INK_IR_VALUE(Name) friend class Name;
#include "ink/ir/ir.def"
  };

  class IntegerConstant final : public Value
  {
  public:
    IntegerConstant(const Type &ValueType, std::int64_t Integer) noexcept : Value(ValueKind::IntegerConstant, ValueType), Integer(Integer)
    {
    }

    std::int64_t value() const noexcept
    {
      return Integer;
    }

  private:
    std::int64_t Integer;
  };

  class ValueOperand final : public Value
  {
  public:
    ValueOperand(const Type &ValueType, ValueId Id) noexcept : Value(ValueKind::ValueOperand, ValueType), Id(Id)
    {
    }

    ValueId id() const noexcept
    {
      return Id;
    }

  private:
    ValueId Id;
  };

  class GlobalAddressOperand final : public Value
  {
  public:
    GlobalAddressOperand(const Type &ValueType, GlobalId Global, std::size_t ByteOffset) noexcept : Value(ValueKind::GlobalAddressOperand, ValueType), Global(Global), ByteOffset(ByteOffset)
    {
    }

    GlobalId global() const noexcept
    {
      return Global;
    }

    std::size_t byteOffset() const noexcept
    {
      return ByteOffset;
    }

  private:
    GlobalId Global;
    std::size_t ByteOffset;
  };

  class ZeroInitializer final : public Value
  {
  public:
    explicit ZeroInitializer(const Type &ValueType) noexcept : Value(ValueKind::ZeroInitializer, ValueType)
    {
    }
  };
} // namespace ink::ir

#endif
