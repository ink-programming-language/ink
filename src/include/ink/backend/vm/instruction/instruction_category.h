#pragma once

#include <cstdint>

namespace ink::vm
{
  enum class InstructionCategory : std::uint8_t
  {
    Load,
    Store,
    Add,
    Compare,
    Branch,
    Return,
    Call,
  };
} // namespace ink::vm
