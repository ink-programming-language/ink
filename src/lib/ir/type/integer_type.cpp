#include "ink/ir/type/integer_type.h"

namespace ink::ir
{
  std::unique_ptr<IntegerType> IntegerType::create(const core::TargetContext &Target, std::uint32_t Width, bool Signed)
  {
    if (Width != 32 && Width != 64)
    {
      return nullptr;
    }
    return std::unique_ptr<IntegerType>(new IntegerType(Target, Width, Signed));
  }

  IntegerType::IntegerType(const core::TargetContext &Target, std::uint32_t Width, bool Signed) noexcept
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
