#include "ink/backend/vm/instruction/eflags.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>

namespace ink::vm::test
{
  // Arithmetic flag definitions retain stable bit positions and exclude reserved or unsupported bits.
  TEST(VMEFlagsTest, DefinesStableArithmeticFlagMasks)
  {
    static_assert(std::is_same_v<EFlags, std::uint32_t>);
    static_assert(eflagMask(EFlag::CF) == 0x001U);
    static_assert(eflagMask(EFlag::ZF) == 0x040U);
    static_assert(eflagMask(EFlag::SF) == 0x080U);
    static_assert(eflagMask(EFlag::OF) == 0x800U);
    static_assert(ArithmeticEFlagsMask == 0x8C1U);
    EXPECT_EQ(ArithmeticEFlagsMask & 0x14U, 0U);
  }

  // Every condition describes exactly which flags it reads without evaluating a runtime flag value.
  TEST(VMEFlagsTest, DescribesConditionFlagDependencies)
  {
    struct ConditionEntry
    {
        ConditionCode Condition;
        EFlags ReadMask;
    };
    constexpr ConditionEntry Conditions[] = {
        {ConditionCode::None, 0x000},
        {ConditionCode::Always, 0x000},
        {ConditionCode::Equal, 0x040},
        {ConditionCode::NotEqual, 0x040},
        {ConditionCode::Below, 0x001},
        {ConditionCode::BelowOrEqual, 0x041},
        {ConditionCode::Above, 0x041},
        {ConditionCode::AboveOrEqual, 0x001},
        {ConditionCode::Less, 0x880},
        {ConditionCode::LessOrEqual, 0x8C0},
        {ConditionCode::Greater, 0x8C0},
        {ConditionCode::GreaterOrEqual, 0x880},
        {ConditionCode::Overflow, 0x800},
        {ConditionCode::NotOverflow, 0x800},
        {ConditionCode::Sign, 0x080},
        {ConditionCode::NotSign, 0x080},
    };
    static_assert(conditionEFlagsMask(ConditionCode::Always) == 0);
    static_assert(conditionEFlagsMask(ConditionCode::Less) == 0x880U);
    for (const ConditionEntry &Entry : Conditions)
    {
      SCOPED_TRACE(static_cast<unsigned>(Entry.Condition));
      EXPECT_EQ(conditionEFlagsMask(Entry.Condition), Entry.ReadMask);
      EXPECT_EQ(Entry.ReadMask & ~ArithmeticEFlagsMask, 0U);
    }
    EXPECT_EQ(conditionEFlagsMask(static_cast<ConditionCode>(0xFF)), 0U);
  }
} // namespace ink::vm::test
