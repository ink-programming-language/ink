#include "ink/ir/constant/integer_constant.h"

namespace ink::ir
{
  IntegerConstant::IntegerConstant(const IntegerType *IntegerTypeValue, std::uint64_t Bits) noexcept
      : Constant(ValueKind::IntegerConstant, IntegerTypeValue),
        Bits(Bits)
  {
  }

  std::uint64_t IntegerConstant::bits() const noexcept
  {
    return Bits;
  }

  bool IntegerConstant::isNegative() const noexcept
  {
    const auto &Integer = static_cast<const IntegerType &>(*type());
    return Integer.isSigned() && (Bits & (std::uint64_t{1} << (Integer.width() - 1U))) != 0;
  }

  std::optional<std::uint64_t> IntegerConstant::tryGetNonNegativeUInt64() const noexcept
  {
    if (isNegative())
    {
      return std::nullopt;
    }
    return Bits;
  }

  bool IntegerConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::IntegerConstant;
  }
} // namespace ink::ir
