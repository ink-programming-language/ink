#pragma once

#include "ink/ir/type/type.h"

#include <cstddef>

namespace ink::ir
{
  // Borrowed referenced type values must remain alive during use; destruction does not access them.
  // The caller must provide a value whose type is MetaType.
  class ReferenceType final : public Type
  {
    public:
      std::size_t hash() const noexcept;
      static std::size_t hash(const Value *Value) noexcept;
      bool operator==(const ReferenceType &Other) const noexcept;
      bool operator==(const Value *Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Reference;

      constexpr explicit ReferenceType(ValueRef ReferencedType) noexcept
          : Type(Kind),
            ReferencedType(ReferencedType.get())
      {
      }

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

      constexpr TypeSemantics getSemantics() const noexcept override
      {
        return TypeSemantics::Reference;
      }

      constexpr const Value &referencedType() const noexcept
      {
        return ReferencedType;
      }

    private:
      const Value &ReferencedType;
  };
} // namespace ink::ir
