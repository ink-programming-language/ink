#pragma once

#include "ink/ir/value/value_kind.h"

#include <functional>

namespace ink::ir
{
  class Type;

  // The borrowed type value must remain alive whenever this value or a derived value is used.
  // Destruction does not access borrowed type values.
  class Value
  {
    public:
      constexpr ValueKind valueKind() const noexcept
      {
        return Kind;
      }

      constexpr const Value &getType() const noexcept
      {
        return *ValueType;
      }

      bool isTypeValue() const noexcept;

      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;

    protected:
      struct SelfTypedTag
      {
      };

      constexpr explicit Value(std::reference_wrapper<const Value> ValueType, ValueKind Kind = ValueKind::Unknown) noexcept
          : ValueType(&ValueType.get()),
            Kind(Kind)
      {
      }

      // Values are destroyed through their concrete class, never through Value.
      ~Value() = default;

    private:
      friend class Type;

      // Only Type's metatype constructor may establish the self-typed root.
      constexpr explicit Value(SelfTypedTag) noexcept
          : ValueType(this),
            Kind(ValueKind::Type)
      {
      }

      const Value *ValueType;
      ValueKind Kind;
  };

  // Borrows a read-only value; implicit construction accepts lvalues and rejects temporary values.
  using ValueRef = std::reference_wrapper<const Value>;
} // namespace ink::ir
