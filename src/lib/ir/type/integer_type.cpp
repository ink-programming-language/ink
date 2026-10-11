#include "ink/ir/type/integer_type.h"

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

  std::uint64_t IntegerType::size() const noexcept
  {
    return Width / 8;
  }

  std::uint64_t IntegerType::alignment() const noexcept
  {
    return Alignment;
  }

  bool IntegerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Integer;
  }
} // namespace ink::ir
