#include "ink/ir/type/nominal_type/interface_type.h"

namespace ink::ir
{
  InterfaceType::InterfaceType(const Module &Owner, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept
      : NominalType(ValueKind::Interface, Owner, Size, Alignment, Name)
  {
  }

  InterfaceType::InterfaceType(const InterfaceType &Declaration, bool Mutable) noexcept
      : NominalType(Declaration, Mutable)
  {
  }

  bool InterfaceType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Interface;
  }
} // namespace ink::ir
