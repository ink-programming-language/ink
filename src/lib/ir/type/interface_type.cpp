#include "ink/ir/type/interface_type.h"

namespace ink::ir
{
  InterfaceType::InterfaceType(const TypeContext &Context, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept
      : NominalType(ValueKind::Interface, Context, Size, Alignment, Name)
  {
  }

  bool InterfaceType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Interface;
  }
} // namespace ink::ir
