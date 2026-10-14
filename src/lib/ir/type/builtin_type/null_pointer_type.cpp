#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/module/module.h"

namespace ink::ir
{
  NullPointerType::NullPointerType(const Module &Owner) noexcept
      : Type(ValueKind::NullPointer),
        Owner(Owner),
        PointerByteWidth(Owner.targetContext().pointerByteWidth())
  {
  }

  const Module &NullPointerType::module() const noexcept
  {
    return Owner;
  }

  std::uint64_t NullPointerType::size() const noexcept
  {
    return PointerByteWidth;
  }

  std::uint64_t NullPointerType::alignment() const noexcept
  {
    return PointerByteWidth;
  }

  bool NullPointerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::NullPointer;
  }
} // namespace ink::ir
