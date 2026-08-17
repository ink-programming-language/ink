#include "ink/ir/model/constant.h"

namespace ink::ir
{
  bool isConstantKind(ValueKind Kind) noexcept
  {
    switch (Kind)
    {
    case ValueKind::IntegerConstant:
    case ValueKind::FloatConstant:
    case ValueKind::StringConstant:
    case ValueKind::NullConstant:
    case ValueKind::ZeroInitializer:
    case ValueKind::AggregateConstant:
      return true;
    case ValueKind::ValueOperand:
    case ValueKind::GlobalAddressOperand:
    case ValueKind::GlobalVariableAddressOperand:
      return false;
    }
    return false;
  }

  bool constantsEqual(const Constant &Left, const Constant &Right) noexcept
  {
    if (Left.kind() != Right.kind() || &Left.type() != &Right.type())
    {
      return false;
    }
    switch (Left.kind())
    {
    case ValueKind::IntegerConstant:
    {
      const IntegerConstant &LeftInteger = static_cast<const IntegerConstant &>(Left);
      const IntegerConstant &RightInteger = static_cast<const IntegerConstant &>(Right);
      return LeftInteger.unsignedValue() == RightInteger.unsignedValue() && LeftInteger.isNegative() == RightInteger.isNegative();
    }
    case ValueKind::FloatConstant:
    {
      const FloatConstant &LeftFloat = static_cast<const FloatConstant &>(Left);
      const FloatConstant &RightFloat = static_cast<const FloatConstant &>(Right);
      return LeftFloat.format() == RightFloat.format() && LeftFloat.bitPattern() == RightFloat.bitPattern();
    }
    case ValueKind::StringConstant:
      return static_cast<const StringConstant &>(Left).data() == static_cast<const StringConstant &>(Right).data();
    case ValueKind::NullConstant:
    case ValueKind::ZeroInitializer:
      return true;
    case ValueKind::AggregateConstant:
    {
      const AggregateConstant &LeftAggregate = static_cast<const AggregateConstant &>(Left);
      const AggregateConstant &RightAggregate = static_cast<const AggregateConstant &>(Right);
      if (LeftAggregate.elements().size() != RightAggregate.elements().size())
      {
        return false;
      }
      for (std::size_t ElementIndex = 0; ElementIndex < LeftAggregate.elements().size(); ++ElementIndex)
      {
        if (!constantsEqual(LeftAggregate.elements()[ElementIndex].get(), RightAggregate.elements()[ElementIndex].get()))
        {
          return false;
        }
      }
      return true;
    }
    case ValueKind::ValueOperand:
    case ValueKind::GlobalAddressOperand:
    case ValueKind::GlobalVariableAddressOperand:
      return false;
    }
    return false;
  }

  const char *floatFormatName(FloatFormat Format) noexcept
  {
    switch (Format)
    {
    case FloatFormat::F16:
      return "f16";
    case FloatFormat::F32:
      return "f32";
    case FloatFormat::F64:
      return "f64";
    }
    return "unknown";
  }

  std::size_t floatFormatBitWidth(FloatFormat Format) noexcept
  {
    switch (Format)
    {
    case FloatFormat::F16:
      return 16;
    case FloatFormat::F32:
      return 32;
    case FloatFormat::F64:
      return 64;
    }
    return 0;
  }
} // namespace ink::ir
