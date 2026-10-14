#include "ink/ir/type/nominal_type/struct_type.h"

namespace ink::ir
{
  StructType::StructType(const Module &Owner, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept
      : NominalType(ValueKind::Struct, Owner, Size, Alignment, Name)
  {
  }

  bool StructType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Struct;
  }
} // namespace ink::ir
