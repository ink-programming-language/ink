#include "ink/ir/constant/integer_constant.h"

namespace ink::ir
{
  IntegerConstant::IntegerConstant(const IntegerType *IntegerTypeValue, const core::IntegerBits &Bits)
      : Constant(ValueKind::IntegerConstant, IntegerTypeValue),
        Bits(Bits)
  {
  }

  const core::IntegerBits &IntegerConstant::bits() const noexcept
  {
    return Bits;
  }

  bool IntegerConstant::isNegative() const noexcept
  {
    const auto &Integer = static_cast<const IntegerType &>(*type());
    return Integer.isSigned() && Bits.isNegative();
  }

  std::optional<std::uint64_t> IntegerConstant::tryGetNonNegativeUInt64() const noexcept
  {
    if (isNegative())
    {
      return std::nullopt;
    }
    return Bits.tryGetUInt64();
  }

  bool IntegerConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::IntegerConstant;
  }
} // namespace ink::ir
