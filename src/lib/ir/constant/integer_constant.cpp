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

  bool IntegerConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::IntegerConstant;
  }
} // namespace ink::ir
