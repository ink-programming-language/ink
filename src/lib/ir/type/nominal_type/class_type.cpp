#include "ink/ir/type/nominal_type/class_type.h"

namespace ink::ir
{
  ClassType::ClassType(const Module &Owner, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept
      : NominalType(ValueKind::Class, Owner, Size, Alignment, Name)
  {
  }

  bool ClassType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Class;
  }
} // namespace ink::ir
