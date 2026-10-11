#include "ink/ir/type/string_type.h"

namespace ink::ir
{
  StringType::StringType(const target::TargetContext &Target) noexcept
      : Type(ValueKind::String),
        PointerByteWidth(Target.pointerByteWidth()),
        MaximumByteLength(Target.maximumPointerSizeValue())
  {
  }

  std::uint64_t StringType::maximumByteLength() const noexcept
  {
    return MaximumByteLength;
  }

  std::uint64_t StringType::size() const noexcept
  {
    return 2 * PointerByteWidth;
  }

  std::uint64_t StringType::alignment() const noexcept
  {
    return PointerByteWidth;
  }

  bool StringType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::String;
  }
} // namespace ink::ir
