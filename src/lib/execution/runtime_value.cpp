#include "ink/execution/runtime_value.h"

namespace ink::execution
{
  RuntimeValue::RuntimeValue(ir::IrTypeId TypeValue, std::uint64_t BitsValue) noexcept : Type(TypeValue), Bits(BitsValue)
  {
  }

  RuntimeValue RuntimeValue::fromBits(ir::IrTypeId Type, std::uint64_t Bits) noexcept
  {
    return RuntimeValue(Type, Bits);
  }

  RuntimeValue RuntimeValue::fromBool(ir::IrTypeId Type, bool Value) noexcept
  {
    return RuntimeValue(Type, Value ? 1 : 0);
  }

  ir::IrTypeId RuntimeValue::type() const noexcept
  {
    return Type;
  }

  std::uint64_t RuntimeValue::bits() const noexcept
  {
    return Bits;
  }

  bool operator==(RuntimeValue Left, RuntimeValue Right) noexcept
  {
    return Left.type() == Right.type() && Left.bits() == Right.bits();
  }

  bool operator!=(RuntimeValue Left, RuntimeValue Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::execution
