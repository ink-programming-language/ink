#pragma once

#include "ink/ir/value/value.h"

namespace ink::ir
{
  // Common base for immutable constants; the borrowed type value must outlive the concrete constant.
  class Constant : public Value
  {
    protected:
      constexpr explicit Constant(ValueRef ValueType) noexcept
          : Value(ValueType)
      {
      }

      // Constants are destroyed through their concrete class, never through Constant.
      ~Constant() = default;
  };
} // namespace ink::ir
