#include "ink/ir/type/generic_class_type.h"

#include <utility>

namespace ink::ir
{
  GenericClassType::GenericClassType(core::Name Name, std::vector<const Type *> GenericParameters) noexcept
      : GenericType(ValueKind::GenericClass, Name, std::move(GenericParameters))
  {
  }

  bool GenericClassType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::GenericClass;
  }
} // namespace ink::ir
