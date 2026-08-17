#ifndef INK_IR_VALUE_H
#define INK_IR_VALUE_H

#include "ink/ir/id.h"
#include "ink/ir/type.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  enum class ValueKind : std::uint8_t
  {
#define INK_IR_VALUE(Name) Name,
#include "ink/ir/ir.def"
  };

  const char *valueKindName(ValueKind Kind) noexcept;

  enum class FloatFormat : std::uint8_t
  {
    F16,
    F32,
    F64,
  };

  const char *floatFormatName(FloatFormat Format) noexcept;
  std::size_t floatFormatBitWidth(FloatFormat Format) noexcept;

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
    Value(ValueKind Kind, const Type &ValueType) noexcept
        : Kind(Kind),
          ValueType(&ValueType)
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
    template <typename IntegerType, std::enable_if_t<std::is_integral_v<IntegerType>, int> = 0>
    IntegerConstant(const Type &ValueType, IntegerType Integer) noexcept
        : Value(ValueKind::IntegerConstant, ValueType),
          Payload(static_cast<std::uint64_t>(Integer)),
          Negative(isNegativeInteger(Integer))
    {
    }

    std::int64_t value() const noexcept
    {
      return signedValue();
    }

    std::uint64_t unsignedValue() const noexcept
    {
      return Payload;
    }

    bool isNegative() const noexcept
    {
      return Negative;
    }

    std::int64_t signedValue() const noexcept
    {
      return Payload <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ? static_cast<std::int64_t>(Payload) : -1 - static_cast<std::int64_t>(std::numeric_limits<std::uint64_t>::max() - Payload);
    }

  private:
    template <typename IntegerType>
    static constexpr bool isNegativeInteger(IntegerType Integer) noexcept
    {
      if constexpr (std::is_signed_v<IntegerType>)
      {
        return Integer < 0;
      }
      else
      {
        return false;
      }
    }

    std::uint64_t Payload;
    bool Negative;
  };

  class ValueOperand final : public Value
  {
  public:
    ValueOperand(const Type &ValueType, ValueId Id) noexcept
        : Value(ValueKind::ValueOperand, ValueType),
          Id(Id)
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
    GlobalAddressOperand(const Type &ValueType, ByteConstantId Constant, std::size_t ByteOffset) noexcept
        : Value(ValueKind::GlobalAddressOperand, ValueType),
          Constant(Constant),
          ByteOffset(ByteOffset)
    {
    }

    GlobalAddressOperand(const Type &ValueType, GlobalId Constant, std::size_t ByteOffset) noexcept
        : GlobalAddressOperand(ValueType, ByteConstantId{Constant.value()}, ByteOffset)
    {
    }

    ByteConstantId byteConstant() const noexcept
    {
      return Constant;
    }

    ByteConstantId global() const noexcept
    {
      return byteConstant();
    }

    std::size_t byteOffset() const noexcept
    {
      return ByteOffset;
    }

    void resolveByteConstant(ByteConstantId ResolvedConstant) noexcept
    {
      Constant = ResolvedConstant;
    }

    void resolveGlobal(GlobalId ResolvedConstant) noexcept
    {
      resolveByteConstant(ByteConstantId{ResolvedConstant.value()});
    }

  private:
    ByteConstantId Constant;
    std::size_t ByteOffset;
  };

  class GlobalVariableAddressOperand final : public Value
  {
  public:
    GlobalVariableAddressOperand(const Type &ValueType, GlobalRef Global) noexcept
        : Value(ValueKind::GlobalVariableAddressOperand, ValueType),
          Global(Global)
    {
    }

    GlobalRef global() const noexcept
    {
      return Global;
    }

    void resolveGlobal(GlobalRef ResolvedGlobal) noexcept
    {
      Global = ResolvedGlobal;
    }

  private:
    GlobalRef Global;
  };

  class ZeroInitializer final : public Value
  {
  public:
    explicit ZeroInitializer(const Type &ValueType) noexcept
        : Value(ValueKind::ZeroInitializer, ValueType)
    {
    }
  };

  class FloatConstant final : public Value
  {
  public:
    FloatConstant(const Type &ValueType, FloatFormat Format, std::uint64_t BitPattern) noexcept
        : Value(ValueKind::FloatConstant, ValueType),
          Format(Format),
          BitPattern(BitPattern)
    {
    }

    FloatFormat format() const noexcept
    {
      return Format;
    }

    std::uint64_t bitPattern() const noexcept
    {
      return BitPattern;
    }

  private:
    FloatFormat Format;
    std::uint64_t BitPattern;
  };

  class StringConstant final : public Value
  {
  public:
    StringConstant(const Type &ValueType, std::string Data) noexcept
        : Value(ValueKind::StringConstant, ValueType),
          Data(std::move(Data))
    {
    }

    const std::string &data() const noexcept
    {
      return Data;
    }

  private:
    std::string Data;
  };

  class NullConstant final : public Value
  {
  public:
    explicit NullConstant(const Type &ValueType) noexcept
        : Value(ValueKind::NullConstant, ValueType)
    {
    }
  };

  class AggregateConstant final : public Value
  {
  public:
    AggregateConstant(const Type &ValueType, std::vector<std::unique_ptr<Value>> Elements) noexcept
        : Value(ValueKind::AggregateConstant, ValueType),
          Elements(std::move(Elements))
    {
    }

    const std::vector<std::unique_ptr<Value>> &elements() const noexcept
    {
      return Elements;
    }

  private:
    std::vector<std::unique_ptr<Value>> Elements;
  };
} // namespace ink::ir

#endif
