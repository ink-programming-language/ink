#pragma once

#include <cstdint>

namespace ink::vm
{
    enum class InstructionDomain : std::uint8_t
    {
        None,
        Integer,
        FloatingPoint,
    };
} // namespace ink::vm
