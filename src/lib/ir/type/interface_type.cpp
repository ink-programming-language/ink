#include "ink/ir/type/interface_type.h"

namespace ink::ir
{
  InterfaceType::InterfaceType(std::uint64_t Size, std::uint64_t Alignment) noexcept
      : Type(ValueKind::Interface),
        Size(Size),
        Alignment(Alignment)
  {
  }

  std::uint64_t InterfaceType::size() const noexcept
  {
    return Size;
  }

  std::uint64_t InterfaceType::alignment() const noexcept
  {
    return Alignment;
  }

  bool InterfaceType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Interface;
  }
} // namespace ink::ir
