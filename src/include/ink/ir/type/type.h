#pragma once

#include "ink/ir/type/type_kind.h"
#include "ink/ir/value/value.h"

#include <cstdint>
#include <string_view>

namespace ink::ir
{
  class MetaType;

  enum class TypeSemantics : std::uint8_t
  {
    Value,
    Reference,
  };

  class Type : public Value
  {
    public:
      static constexpr bool classof(const Value *Value) noexcept
      {
        return Value && Value->valueKind() == ValueKind::Type;
      }

      constexpr TypeKind getKind() const noexcept
      {
        return Kind;
      }

      std::string_view getName() const noexcept
      {
        return typeKindName(Kind);
      }

      virtual constexpr TypeSemantics getSemantics() const noexcept
      {
        return TypeSemantics::Value;
      }

      Type(const Type &) = delete;
      Type &operator=(const Type &) = delete;

    protected:
      constexpr explicit Type(TypeKind Kind) noexcept;

      // Types are destroyed through their concrete class, never through Type.
      ~Type() = default;

    private:
      friend class MetaType;

      constexpr Type(TypeKind Kind, SelfTypedTag Tag) noexcept
          : Value(Tag),
            Kind(Kind)
      {
      }

      TypeKind Kind;
  };
} // namespace ink::ir

// Complete the metatype before defining the ordinary constexpr Type constructor.
#include "ink/ir/type/meta_type.h"
