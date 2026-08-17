#ifndef INK_IR_CONSTANT_H
#define INK_IR_CONSTANT_H

#include "ink/ir/model/value.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  class ConstantPool;

  enum class FloatFormat : std::uint8_t
  {
    F16,
    F32,
    F64,
  };

  const char *floatFormatName(FloatFormat Format) noexcept;
  std::size_t floatFormatBitWidth(FloatFormat Format) noexcept;

  class Constant : public Value
  {
    public:
      Constant(const Constant &) = delete;
      Constant &operator=(const Constant &) = delete;
      Constant(Constant &&) = delete;
      Constant &operator=(Constant &&) = delete;

    protected:
      Constant(ValueKind Kind, const Type &ValueType) noexcept
          : Value(Kind, ValueType)
      {
      }
  };

  class IntegerConstant final : public Constant
  {
    public:
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
      template <typename IntegerType, std::enable_if_t<std::is_integral_v<IntegerType>, int> = 0>
      IntegerConstant(const Type &ValueType, IntegerType Integer) noexcept
          : Constant(ValueKind::IntegerConstant, ValueType),
            Payload(static_cast<std::uint64_t>(Integer)),
            Negative(isNegativeInteger(Integer))
      {
      }

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

      friend class ConstantPool;
  };

  class FloatConstant final : public Constant
  {
    public:
      FloatFormat format() const noexcept
      {
        return Format;
      }

      std::uint64_t bitPattern() const noexcept
      {
        return BitPattern;
      }

    private:
      FloatConstant(const Type &ValueType, FloatFormat Format, std::uint64_t BitPattern) noexcept
          : Constant(ValueKind::FloatConstant, ValueType),
            Format(Format),
            BitPattern(BitPattern)
      {
      }

      FloatFormat Format;
      std::uint64_t BitPattern;

      friend class ConstantPool;
  };

  class StringConstant final : public Constant
  {
    public:
      const std::string &data() const noexcept
      {
        return Data;
      }

    private:
      StringConstant(const Type &ValueType, std::string Data) noexcept
          : Constant(ValueKind::StringConstant, ValueType),
            Data(std::move(Data))
      {
      }

      std::string Data;

      friend class ConstantPool;
  };

  class NullConstant final : public Constant
  {
    private:
      explicit NullConstant(const Type &ValueType) noexcept
          : Constant(ValueKind::NullConstant, ValueType)
      {
      }

      friend class ConstantPool;
  };

  class ZeroInitializer final : public Constant
  {
    private:
      explicit ZeroInitializer(const Type &ValueType) noexcept
          : Constant(ValueKind::ZeroInitializer, ValueType)
      {
      }

      friend class ConstantPool;
  };

  class AggregateConstant final : public Constant
  {
    public:
      const std::vector<std::reference_wrapper<const Constant>> &elements() const noexcept
      {
        return Elements;
      }

    private:
      AggregateConstant(const Type &ValueType, std::vector<std::reference_wrapper<const Constant>> Elements) noexcept
          : Constant(ValueKind::AggregateConstant, ValueType),
            Elements(std::move(Elements))
      {
      }

      std::vector<std::reference_wrapper<const Constant>> Elements;

      friend class ConstantPool;
  };

  bool isConstantKind(ValueKind Kind) noexcept;
  bool constantsEqual(const Constant &Left, const Constant &Right) noexcept;
} // namespace ink::ir

#endif
