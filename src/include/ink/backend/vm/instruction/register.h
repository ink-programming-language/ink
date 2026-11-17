#pragma once

#include <cstdint>
#include <optional>
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

    constexpr std::optional<RegisterId> parseRegister(std::string_view Text) noexcept
    {
        struct RegisterEntry
        {
                RegisterId Id;
                std::string_view Spelling;
        };

        constexpr RegisterEntry Entries[] = {
#define REGISTER(Kind, Encoding, Spelling) {RegisterId::Kind, Spelling},
#include "ink/backend/vm/instruction/register.def"
#undef REGISTER
        };

        for (const RegisterEntry& Entry : Entries)
        {
            if (Entry.Spelling == Text)
            {
                return Entry.Id;
            }
        }
        return std::nullopt;
    }

    constexpr bool isGeneralPurposeRegister(RegisterId Register) noexcept
    {
        return Register >= RegisterId::R0 && Register <= RegisterId::R9;
    }

    constexpr bool isAddressRegister(RegisterId Register) noexcept
    {
        return Register == RegisterId::BP || Register == RegisterId::SP || Register == RegisterId::PC || Register == RegisterId::ZERO || isGeneralPurposeRegister(Register);
    }

    constexpr bool isFloatingPointSourceRegister(RegisterId Register) noexcept
    {
        return isGeneralPurposeRegister(Register) || Register == RegisterId::ZERO;
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
