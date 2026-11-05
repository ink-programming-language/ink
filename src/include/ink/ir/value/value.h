#pragma once

#include "ink/ir/type/type.h"

#include <functional>

namespace ink::ir
{
  // The borrowed type must outlive the value and any derived value.
  class Value
  {
    public:
      constexpr const Type &getType() const noexcept
      {
        return ValueType;
      }

      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;

    protected:
      constexpr explicit Value(const Type &ValueType) noexcept
          : ValueType(ValueType)
      {
      }

      Value(const Type &&) = delete;

      // Values are destroyed through their concrete class, never through Value.
      ~Value() = default;

    private:
      const Type &ValueType;
  };

  // Borrows a read-only value; implicit construction accepts lvalues and rejects temporary values.
  using ValueRef = std::reference_wrapper<const Value>;
} // namespace ink::ir
