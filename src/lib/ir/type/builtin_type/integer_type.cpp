#include "ink/ir/type/builtin_type/integer_type.h"

#include <limits>
#include <vector>

namespace ink::ir
{
  IntegerType::IntegerType(const MetaType &Meta, const target::TargetContext &Target, std::uint32_t Width, bool Signed, bool Mutable) noexcept
      : Type(ValueKind::Integer, Meta, Mutable),
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

  core::IntegerBits IntegerType::maximumValue() const
  {
    std::vector<std::uint64_t> Words((Width + 63U) / 64U, std::numeric_limits<std::uint64_t>::max());
    if (Width % 64U != 0)
    {
      Words.back() = (std::uint64_t{1} << (Width % 64U)) - 1;
    }
    if (Signed)
    {
      Words.back() &= ~(std::uint64_t{1} << ((Width - 1U) % 64U));
    }
    return *core::IntegerBits::fromWords(Width, Words);
  }

  core::IntegerBits IntegerType::minimumValue() const
  {
    std::vector<std::uint64_t> Words((Width + 63U) / 64U, 0);
    if (Signed)
    {
      Words.back() = std::uint64_t{1} << ((Width - 1U) % 64U);
    }
    return *core::IntegerBits::fromWords(Width, Words);
  }

  bool IntegerType::canRepresentSignedValue(std::int64_t Value) const noexcept
  {
    if (Value >= 0)
    {
      return canRepresentUnsignedValue(static_cast<std::uint64_t>(Value));
    }
    return Signed && (Width >= 64U || Value >= -(std::int64_t{1} << (Width - 1U)));
  }

  bool IntegerType::canRepresentUnsignedValue(std::uint64_t Value) const noexcept
  {
    const std::uint32_t ValueWidth = Width - (Signed ? 1U : 0U);
    return ValueWidth >= 64U || Value < (std::uint64_t{1} << ValueWidth);
  }

  std::uint64_t IntegerType::size() const noexcept
  {
    return (static_cast<std::uint64_t>(Width) + 7U) / 8U;
  }

  std::uint64_t IntegerType::alignment() const noexcept
  {
    return Alignment;
  }

  std::string IntegerType::toString() const
  {
    return std::string(isMutable() ? "" : "const ") + (Signed ? "i" : "u") + std::to_string(Width);
  }

  bool IntegerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Integer;
  }
} // namespace ink::ir
