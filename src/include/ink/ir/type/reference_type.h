#pragma once

#include "ink/ir/type/type.h"

#include <cstddef>

namespace ink::ir
{
  // The referenced type is borrowed and must outlive this type.
  class ReferenceType final : public Type
  {
    public:
      std::size_t hash() const noexcept;
      static std::size_t hash(const Type *Value) noexcept;
      bool operator==(const ReferenceType &Other) const noexcept;
      bool operator==(const Type *Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Reference;

      constexpr explicit ReferenceType(const Type &ReferencedType) noexcept
          : Type(Kind),
            ReferencedType(ReferencedType)
      {
      }

      ReferenceType(const Type &&) = delete;

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr TypeSemantics getSemantics() const noexcept override
      {
        return TypeSemantics::Reference;
      }

      constexpr const Type &referencedType() const noexcept
      {
        return ReferencedType;
      }

    private:
      const Type &ReferencedType;
  };
} // namespace ink::ir
