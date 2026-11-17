#include "ink/backend/vm/instruction/register.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace ink::vm::test
{
    namespace
    {
        constexpr std::array GeneralPurposeRegisters = {
            RegisterId::R0,
            RegisterId::R1,
            RegisterId::R2,
            RegisterId::R3,
            RegisterId::R4,
            RegisterId::R5,
            RegisterId::R6,
            RegisterId::R7,
            RegisterId::R8,
            RegisterId::R9,
        };

        constexpr std::array<std::string_view, 10> GeneralPurposeSpellings = {
            "r0",
            "r1",
            "r2",
            "r3",
            "r4",
            "r5",
            "r6",
            "r7",
            "r8",
            "r9",
        };
    } // namespace

    // Register encodings are fixed byte-sized IDs rather than object-local symbol reference indices.
    TEST(VMRegisterTest, HasStableIndependentByteEncodings)
    {
        static_assert(std::is_same_v<std::underlying_type_t<RegisterId>, std::uint8_t>);
        EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::BP), 0x00);
        EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::SP), 0x01);
        EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::PC), 0x02);
        EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::ZERO), 0x03);
        for (std::size_t Index = 0; Index < GeneralPurposeRegisters.size(); ++Index)
        {
            EXPECT_EQ(static_cast<std::uint8_t>(GeneralPurposeRegisters[Index]), Index + 4);
        }
    }

    // Every defined logical register has a lowercase spelling available during constant evaluation.
    TEST(VMRegisterTest, ExposesConstantRegisterSpellings)
    {
        static_assert(getRegisterName(RegisterId::BP) == "bp");
        static_assert(getRegisterName(RegisterId::SP) == "sp");
        static_assert(getRegisterName(RegisterId::PC) == "pc");
        static_assert(getRegisterName(RegisterId::ZERO) == "zero");
        static_assert(getRegisterName(RegisterId::R0) == "r0");
        static_assert(getRegisterName(RegisterId::R9) == "r9");
        EXPECT_EQ(getRegisterName(RegisterId::BP), std::string_view("bp"));
        EXPECT_EQ(getRegisterName(RegisterId::SP), std::string_view("sp"));
        EXPECT_EQ(getRegisterName(RegisterId::PC), std::string_view("pc"));
        EXPECT_EQ(getRegisterName(RegisterId::ZERO), std::string_view("zero"));
        for (std::size_t Index = 0; Index < GeneralPurposeRegisters.size(); ++Index)
        {
            EXPECT_EQ(getRegisterName(GeneralPurposeRegisters[Index]), GeneralPurposeSpellings[Index]);
        }
    }

    // Undefined byte encodings never acquire a valid register spelling.
    TEST(VMRegisterTest, RejectsUndefinedRegisterSpellings)
    {
        EXPECT_EQ(getRegisterName(static_cast<RegisterId>(0x0E)), std::string_view("InvalidRegister"));
        EXPECT_EQ(getRegisterName(static_cast<RegisterId>(0x0F)), std::string_view("InvalidRegister"));
        EXPECT_EQ(getRegisterName(static_cast<RegisterId>(0xFF)), std::string_view("InvalidRegister"));
    }

    // Register parsing resolves exact lowercase spellings and rejects unknown or uppercase names.
    TEST(VMRegisterTest, ParsesRegisteredSpellingsAndRejectsUnknownNames)
    {
        static_assert(parseRegister("bp") == RegisterId::BP);
        static_assert(parseRegister("sp") == RegisterId::SP);
        static_assert(parseRegister("pc") == RegisterId::PC);
        static_assert(parseRegister("zero") == RegisterId::ZERO);
        static_assert(parseRegister("r0") == RegisterId::R0);
        static_assert(parseRegister("r9") == RegisterId::R9);
        static_assert(!parseRegister("r10").has_value());
        static_assert(!parseRegister("BP").has_value());
        for (std::size_t Index = 0; Index < GeneralPurposeRegisters.size(); ++Index)
        {
            EXPECT_EQ(parseRegister(GeneralPurposeSpellings[Index]), GeneralPurposeRegisters[Index]);
        }
        EXPECT_FALSE(parseRegister("").has_value());
        EXPECT_FALSE(parseRegister("r10").has_value());
        EXPECT_FALSE(parseRegister("InvalidRegister").has_value());
    }

    // Only BP, SP, and general-purpose registers are ordinary writable targets; undefined IDs are never operands.
    TEST(VMRegisterTest, DistinguishesWritableAndUndefinedRegisters)
    {
        for (const auto Register : {RegisterId::BP, RegisterId::SP, RegisterId::PC, RegisterId::ZERO})
        {
            EXPECT_TRUE(isAddressRegister(Register));
            EXPECT_TRUE(isArithmeticRegister(Register));
            EXPECT_FALSE(isGeneralPurposeRegister(Register));
        }
        for (const auto Register : GeneralPurposeRegisters)
        {
            EXPECT_TRUE(isGeneralPurposeRegister(Register));
            EXPECT_TRUE(isAddressRegister(Register));
            EXPECT_TRUE(isArithmeticRegister(Register));
            EXPECT_TRUE(isWritableAddressRegister(Register));
        }
        EXPECT_TRUE(isWritableAddressRegister(RegisterId::BP));
        EXPECT_TRUE(isWritableAddressRegister(RegisterId::SP));
        EXPECT_FALSE(isWritableAddressRegister(RegisterId::PC));
        EXPECT_FALSE(isWritableAddressRegister(RegisterId::ZERO));
        for (const auto Register : {static_cast<RegisterId>(0x0E), static_cast<RegisterId>(0x0F), static_cast<RegisterId>(0xFF)})
        {
            EXPECT_FALSE(isGeneralPurposeRegister(Register));
            EXPECT_FALSE(isAddressRegister(Register));
            EXPECT_FALSE(isArithmeticRegister(Register));
            EXPECT_FALSE(isWritableAddressRegister(Register));
        }
    }
} // namespace ink::vm::test
