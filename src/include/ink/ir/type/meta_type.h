#pragma once

#include "ink/ir/type/type.h"

namespace ink::ir
{
  // The canonical type of all type values, including this metatype itself.
  class MetaType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Meta;

      static constexpr const MetaType &instance() noexcept;

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

    private:
      static const MetaType Instance;

      constexpr MetaType() noexcept
          : Type(Kind, SelfTypedTag{})
      {
      }
  };

  inline constexpr MetaType MetaType::Instance{};

  constexpr const MetaType &MetaType::instance() noexcept
  {
    return Instance;
  }

  constexpr Type::Type(TypeKind Kind) noexcept
      : Value(ValueRef{MetaType::instance()}, ValueKind::Type),
        Kind(Kind)
  {
  }
} // namespace ink::ir
