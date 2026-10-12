#include "ink/ir/type/null_pointer_type.h"
#include "ink/ir/type/type_context.h"

namespace ink::ir
{
  NullPointerType::NullPointerType(const TypeContext &Context) noexcept
      : Type(ValueKind::NullPointer),
        Context(Context),
        PointerByteWidth(Context.targetContext().pointerByteWidth())
  {
  }

  const TypeContext &NullPointerType::typeContext() const noexcept
  {
    return Context;
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
