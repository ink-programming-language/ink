#pragma once

#include <cstdint>

namespace ink::vm
{
    enum class InstructionCategory : std::uint8_t
    {
        Load,
        Store,
        Add,
        Branch,
        ConditionalBranch,
        Return,
        Call,
        FloatingPoint,
    };
} // namespace ink::vm
