#include "ink/ir/constant/float_constant.h"

namespace ink::ir
{
  FloatConstant::FloatConstant(const FloatType *FloatTypeValue, std::uint64_t Bits) noexcept
      : Constant(ValueKind::FloatConstant, FloatTypeValue),
        Bits(Bits)
  {
  }

  std::uint64_t FloatConstant::bits() const noexcept
  {
    return Bits;
  }

  bool FloatConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::FloatConstant;
  }
} // namespace ink::ir
