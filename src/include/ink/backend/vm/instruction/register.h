#pragma once

#include <cstdint>
#include <string_view>

namespace ink::vm
{
    enum class RegisterId : std::uint8_t
    {
#define REGISTER(Kind, Encoding, Spelling) Kind = Encoding,
#include "ink/backend/vm/instruction/register.def"
#undef REGISTER
    };

    constexpr std::string_view getRegisterName(RegisterId Register) noexcept
    {
        switch (Register)
        {
#define REGISTER(Kind, Encoding, Spelling) \
    case RegisterId::Kind:                 \
        return Spelling;
#include "ink/backend/vm/instruction/register.def"
#undef REGISTER
        }
        return "InvalidRegister";
    }

    constexpr bool isGeneralPurposeRegister(RegisterId Register) noexcept
    {
        return Register >= RegisterId::R0 && Register <= RegisterId::R9;
    }

    constexpr bool isAddressRegister(RegisterId Register) noexcept
    {
        return Register == RegisterId::BP || Register == RegisterId::SP || Register == RegisterId::PC || Register == RegisterId::ZERO || isGeneralPurposeRegister(Register);
    }

    constexpr bool isWritableAddressRegister(RegisterId Register) noexcept
    {
        return Register == RegisterId::BP || Register == RegisterId::SP || isGeneralPurposeRegister(Register);
    }

    constexpr bool isArithmeticRegister(RegisterId Register) noexcept
    {
        return isAddressRegister(Register);
    }
} // namespace ink::vm
