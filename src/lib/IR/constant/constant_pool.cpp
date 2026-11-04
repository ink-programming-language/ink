#include "ink/ir/constant/constant_pool.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace ink::ir
{
  namespace
  {
    bool isSupportedIntegerWidth(std::uint32_t Width) noexcept
    {
      return Width == 8 || Width == 16 || Width == 32 || Width == 64;
    }

    std::size_t combineHash(std::size_t Seed, std::size_t Value) noexcept
    {
      return Seed ^ (Value + static_cast<std::size_t>(0x9e3779b9U) + (Seed << 6) + (Seed >> 2));
    }

    IntegerConstant::Key integerKey(const IntegerConstant &Value) noexcept
    {
      const auto &ValueType = static_cast<const IntegerType &>(Value.getType());
      return {ValueType.bitWidth(), ValueType.isSigned(), Value.bits()};
    }

    FloatConstant::Key floatKey(const FloatConstant &Value) noexcept
    {
      return {static_cast<const FloatType &>(Value.getType()).bitWidth(), Value.bits()};
    }

    static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559);
    static_assert(sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559);
  } // namespace

  std::size_t IntegerConstant::hash() const noexcept
  {
    return integerKey(*this).hash();
  }

  std::size_t IntegerConstant::Key::hash() const noexcept
  {
    auto Seed = std::hash<std::uint32_t>{}(BitWidth);
    Seed = combineHash(Seed, std::hash<bool>{}(Signed));
    return combineHash(Seed, std::hash<std::uint64_t>{}(Bits));
  }

  bool IntegerConstant::operator==(const IntegerConstant &Other) const noexcept
  {
    return integerKey(*this) == integerKey(Other);
  }

  bool IntegerConstant::operator==(const Key &Other) const noexcept
  {
    return integerKey(*this) == Other;
  }

  std::size_t FloatConstant::hash() const noexcept
  {
    return floatKey(*this).hash();
  }

  std::size_t FloatConstant::Key::hash() const noexcept
  {
    return combineHash(std::hash<std::uint32_t>{}(BitWidth), std::hash<std::uint64_t>{}(Bits));
  }

  bool FloatConstant::operator==(const FloatConstant &Other) const noexcept
  {
    return floatKey(*this) == floatKey(Other);
  }

  bool FloatConstant::operator==(const Key &Other) const noexcept
  {
    return floatKey(*this) == Other;
  }

  std::size_t StringConstant::hash() const noexcept
  {
    return hash(value());
  }

  std::size_t StringConstant::hash(std::string_view Value) noexcept
  {
    return std::hash<std::string_view>{}(Value);
  }

  bool StringConstant::operator==(const StringConstant &Other) const noexcept
  {
    return value() == Other.value();
  }

  bool StringConstant::operator==(std::string_view Other) const noexcept
  {
    return value() == Other;
  }

  ConstantPool::ConstantPool(TypePool &Types) noexcept
      : Types(Types)
  {
  }

  const IntegerConstant *ConstantPool::getInteger(const IntegerType &ValueType, std::uint64_t Bits)
  {
    const auto Width = ValueType.bitWidth();
    if (!isSupportedIntegerWidth(Width) || (Width < 64 && (Bits >> Width) != 0))
    {
      return nullptr;
    }
    const IntegerConstant::Key Key{Width, ValueType.isSigned(), Bits};
    const auto Existing = Integers.find(Key);
    if (Existing != Integers.end())
    {
      return &*Existing;
    }
    const auto &PooledType = Types.getIntegerType(static_cast<IntegerType::Width>(ValueType.bitWidth()), ValueType.isSigned() ? IntegerType::Signedness::Signed : IntegerType::Signedness::Unsigned);
    return &*Integers.emplace(PooledType, Bits).first;
  }

  const IntegerConstant *ConstantPool::getSignedInteger(const IntegerType &ValueType, std::int64_t Value)
  {
    const auto Width = ValueType.bitWidth();
    if (!isSupportedIntegerWidth(Width) || (!ValueType.isSigned() && Value < 0))
    {
      return nullptr;
    }
    if (ValueType.isSigned() && Width < 64)
    {
      const auto SignBit = std::int64_t{1} << (Width - 1);
      if (Value < -SignBit || Value > SignBit - 1)
      {
        return nullptr;
      }
    }
    auto Bits = static_cast<std::uint64_t>(Value);
    if (ValueType.isSigned() && Width < 64)
    {
      Bits &= (std::uint64_t{1} << Width) - 1;
    }
    return getInteger(ValueType, Bits);
  }

  const IntegerConstant *ConstantPool::getUnsignedInteger(const IntegerType &ValueType, std::uint64_t Value)
  {
    if (!isSupportedIntegerWidth(ValueType.bitWidth()))
    {
      return nullptr;
    }
    if (ValueType.isSigned() && Value > (std::uint64_t{1} << (ValueType.bitWidth() - 1)) - 1)
    {
      return nullptr;
    }
    return getInteger(ValueType, Value);
  }

  const FloatConstant &ConstantPool::getFloat(float Value)
  {
    return *getFloatBits(Types.getFloatType(FloatType::Width::Bits32), std::bit_cast<std::uint32_t>(Value));
  }

  const FloatConstant &ConstantPool::getFloat(double Value)
  {
    return *getFloatBits(Types.getFloatType(FloatType::Width::Bits64), std::bit_cast<std::uint64_t>(Value));
  }

  const FloatConstant *ConstantPool::getFloatBits(const FloatType &ValueType, std::uint64_t Bits)
  {
    if ((ValueType.bitWidth() != 32 && ValueType.bitWidth() != 64) || (ValueType.bitWidth() == 32 && Bits > std::numeric_limits<std::uint32_t>::max()))
    {
      return nullptr;
    }
    const FloatConstant::Key Key{ValueType.bitWidth(), Bits};
    const auto Existing = Floats.find(Key);
    if (Existing != Floats.end())
    {
      return &*Existing;
    }
    const auto &PooledType = Types.getFloatType(static_cast<FloatType::Width>(ValueType.bitWidth()));
    return &*Floats.emplace(PooledType, Bits).first;
  }

  const StringConstant &ConstantPool::getString(std::string_view Bytes)
  {
    const auto Existing = Strings.find(Bytes);
    if (Existing != Strings.end())
    {
      return *Existing;
    }
    const auto &ElementType = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const auto &ValueType = Types.getArrayType(ElementType, static_cast<std::uint64_t>(Bytes.size()));
    return *Strings.emplace(ValueType, Bytes).first;
  }
} // namespace ink::ir
