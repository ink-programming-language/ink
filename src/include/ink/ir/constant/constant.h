#pragma once

#include "ink/ir/value/value.h"

namespace ink::ir
{
  // Common base for immutable constants; the borrowed type must outlive the concrete constant.
  class Constant : public Value
  {
    protected:
      constexpr explicit Constant(const Type &ValueType) noexcept
          : Value(ValueType)
      {
      }

      Constant(const Type &&) = delete;

      // Constants are destroyed through their concrete class, never through Constant.
      ~Constant() = default;
  };
} // namespace ink::ir
