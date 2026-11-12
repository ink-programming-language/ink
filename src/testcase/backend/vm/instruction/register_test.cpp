#include "ink/backend/vm/instruction/register.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace ink::vm::test
{
  // Register encodings are fixed byte-sized IDs rather than object-local symbol reference indices.
  TEST(VMRegisterTest, HasStableIndependentByteEncodings)
  {
    static_assert(std::is_same_v<std::underlying_type_t<RegisterId>, std::uint8_t>);
    EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::BP), 0x00);
    EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::SP), 0x01);
    EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::PC), 0x02);
    EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::EFLAGS), 0x03);
    EXPECT_EQ(static_cast<std::uint8_t>(RegisterId::ZERO), 0x04);
  }

  // Every defined logical register has a lowercase spelling available during constant evaluation.
  TEST(VMRegisterTest, ExposesConstantRegisterSpellings)
  {
    static_assert(getRegisterName(RegisterId::BP) == "bp");
    static_assert(getRegisterName(RegisterId::SP) == "sp");
    static_assert(getRegisterName(RegisterId::PC) == "pc");
    static_assert(getRegisterName(RegisterId::EFLAGS) == "eflags");
    static_assert(getRegisterName(RegisterId::ZERO) == "zero");
    EXPECT_EQ(getRegisterName(RegisterId::BP), std::string_view("bp"));
    EXPECT_EQ(getRegisterName(RegisterId::SP), std::string_view("sp"));
    EXPECT_EQ(getRegisterName(RegisterId::PC), std::string_view("pc"));
    EXPECT_EQ(getRegisterName(RegisterId::EFLAGS), std::string_view("eflags"));
    EXPECT_EQ(getRegisterName(RegisterId::ZERO), std::string_view("zero"));
  }

  // Undefined byte encodings never acquire a valid register spelling.
  TEST(VMRegisterTest, RejectsUndefinedRegisterSpellings)
  {
    EXPECT_EQ(getRegisterName(static_cast<RegisterId>(0x05)), std::string_view("InvalidRegister"));
    EXPECT_EQ(getRegisterName(static_cast<RegisterId>(0xFF)), std::string_view("InvalidRegister"));
  }
  // Status and unknown registers cannot become memory bases or ordinary arithmetic operands.
  TEST(VMRegisterTest, DistinguishesAddressAndImplicitStatusRegisters)
  {
    for (const auto Register : {RegisterId::BP, RegisterId::SP, RegisterId::PC, RegisterId::ZERO})
    {
      EXPECT_TRUE(isAddressRegister(Register));
      EXPECT_TRUE(isArithmeticRegister(Register));
    }
    EXPECT_TRUE(isWritableAddressRegister(RegisterId::BP));
    EXPECT_TRUE(isWritableAddressRegister(RegisterId::SP));
    EXPECT_FALSE(isWritableAddressRegister(RegisterId::PC));
    EXPECT_FALSE(isWritableAddressRegister(RegisterId::ZERO));
    for (const auto Register : {RegisterId::EFLAGS, static_cast<RegisterId>(0xff)})
    {
      EXPECT_FALSE(isAddressRegister(Register));
      EXPECT_FALSE(isArithmeticRegister(Register));
      EXPECT_FALSE(isWritableAddressRegister(Register));
    }
  }
} // namespace ink::vm::test
