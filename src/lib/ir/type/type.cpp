#include "ink/ir/type/type.h"

namespace ink::ir
{
  Type::Type(ValueKind Kind) noexcept
      : Value(Kind)
  {
  }

  Type::~Type() noexcept = default;

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
      case ValueKind::Bool:
      case ValueKind::Integer:
      case ValueKind::Float:
      case ValueKind::Array:
      case ValueKind::Class:
      case ValueKind::Interface:
      case ValueKind::Struct:
      case ValueKind::Pointer:
      case ValueKind::Reference:
      case ValueKind::Function:
      case ValueKind::GenericClass:
      case ValueKind::GenericFunction:
      case ValueKind::GenericVar:
      case ValueKind::String:
        return true;
      default:
        return false;
    }
  }
} // namespace ink::ir
