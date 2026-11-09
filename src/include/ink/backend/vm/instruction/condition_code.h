#pragma once

#include <cstdint>

namespace ink::vm
{
  enum class ConditionCode : std::uint8_t
  {
    None,
    Always,
    Equal,
    NotEqual,
    Below,
    BelowOrEqual,
    Above,
    AboveOrEqual,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
    Overflow,
    NotOverflow,
    Sign,
    NotSign,
  };
} // namespace ink::vm
