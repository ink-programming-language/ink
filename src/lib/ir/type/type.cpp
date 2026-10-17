#include "ink/ir/type/type.h"
#include "ink/ir/type/builtin_type/meta_type.h"

namespace ink::ir
{
  Type::Type(ValueKind Kind, const MetaType &Meta) noexcept
      : Value(Kind),
        Meta(Meta)
  {
  }

  Type::~Type() noexcept = default;

  const Type *Type::type() const noexcept
  {
    return &Meta;
  }

  bool Type::hasLayout() const noexcept
  {
    return true;
  }

  bool Type::classof(const Value *Value) noexcept
  {
    if (!Value)
    {
      return false;
    }
    switch (Value->kind())
    {
      case ValueKind::Void:
      case ValueKind::Bool:
      case ValueKind::Integer:
      case ValueKind::Float:
      case ValueKind::Array:
      case ValueKind::Class:
      case ValueKind::Interface:
      case ValueKind::Struct:
      case ValueKind::Pointer:
      case ValueKind::NullPointer:
      case ValueKind::Reference:
      case ValueKind::FunctionType:
      case ValueKind::GenericClass:
      case ValueKind::GenericFunction:
      case ValueKind::GenericVar:
      case ValueKind::Variadic:
      case ValueKind::MetaType:
        return true;
      default:
        return false;
    }
  }
} // namespace ink::ir
