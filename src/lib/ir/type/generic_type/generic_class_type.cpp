#include "ink/ir/type/generic_type/generic_class_type.h"

#include <utility>

namespace ink::ir
{
  GenericClassType::GenericClassType(const MetaType &Meta, core::Name Name, std::vector<const Type *> GenericParameters) noexcept
      : GenericType(ValueKind::GenericClass, Meta, Name, std::move(GenericParameters))
  {
  }

  bool GenericClassType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::GenericClass;
  }
} // namespace ink::ir
