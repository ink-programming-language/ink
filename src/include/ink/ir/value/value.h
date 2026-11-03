#pragma once

#include "ink/ir/type/type.h"

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
} // namespace ink::ir
