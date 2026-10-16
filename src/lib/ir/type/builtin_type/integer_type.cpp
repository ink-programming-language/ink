#include "ink/ir/type/builtin_type/integer_type.h"

#include <limits>

namespace ink::ir
{
  IntegerType::IntegerType(const target::TargetContext &Target, std::uint32_t Width, bool Signed) noexcept
      : Type(ValueKind::Integer),
        Width(Width),
        Signed(Signed),
        Alignment(Target.integerAlignment(Width))
  {
  }

  std::uint32_t IntegerType::width() const noexcept
  {
    return Width;
  }

  bool IntegerType::isSigned() const noexcept
  {
    return Signed;
  }

  bool IntegerType::isUnsigned() const noexcept
  {
    return !Signed;
  }

  std::uint64_t IntegerType::maximumValue() const noexcept
  {
    const std::uint32_t ValueWidth = Width - (Signed ? 1U : 0U);
    return ValueWidth == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << ValueWidth) - 1;
  }

  std::int64_t IntegerType::minimumValue() const noexcept
  {
    if (!Signed)
    {
      return 0;
    }
    return Width == 64 ? std::numeric_limits<std::int64_t>::min() : -(std::int64_t{1} << (Width - 1));
  }

  std::uint64_t IntegerType::size() const noexcept
  {
    return Width / 8;
  }

  std::uint64_t IntegerType::alignment() const noexcept
  {
    return Alignment;
  }

  std::string IntegerType::toString() const
  {
    return std::string(Signed ? "i" : "u") + std::to_string(Width);
  }

  bool IntegerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Integer;
  }
} // namespace ink::ir
