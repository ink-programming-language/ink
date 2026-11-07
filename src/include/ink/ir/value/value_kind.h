#pragma once

#include <cstdint>

namespace ink::ir
{
  enum class ValueKind : std::uint8_t
  {
    Unknown,
    Type,
    Argument,
    Instruction,
    IntegerConstant,
    FloatConstant,
    StringConstant,
    Function,
    GlobalVariable,
  };
} // namespace ink::ir
