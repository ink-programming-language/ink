#pragma once

#include "ink/ir/value/value.h"

namespace ink::ir
{
  class Instruction : public Value
  {
    protected:
      constexpr explicit Instruction(const Type &ValueType) noexcept
          : Value(ValueType)
      {
      }

      Instruction(const Type &&) = delete;

      ~Instruction() = default;
  };
} // namespace ink::ir
