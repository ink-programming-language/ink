#include "ink/ir/type/builtin_type/float_type.h"

namespace ink::ir
{
  FloatType::FloatType(const MetaType &Meta, const target::TargetContext &Target, std::uint32_t Width) noexcept
      : Type(ValueKind::Float, Meta),
        Width(Width),
        Alignment(Target.floatAlignment(Width))
  {
  }

  std::uint32_t FloatType::width() const noexcept
  {
    return Width;
  }

  std::uint64_t FloatType::size() const noexcept
  {
    return Width / 8;
  }

  std::uint64_t FloatType::alignment() const noexcept
  {
    return Alignment;
  }

  std::string FloatType::toString() const
  {
    return "f" + std::to_string(Width);
  }

  bool FloatType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Float;
  }
} // namespace ink::ir
