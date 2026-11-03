#include "ink/core/support/version.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ink::core::test
{
  static_assert(Version() == Version(0, 0, 0));
  static_assert(Version(1, 2, 3).getMajor() == 1);
  static_assert(Version(1, 2, 3).getMinor() == 2);
  static_assert(Version(1, 2, 3).getPatch() == 3);
  static_assert(std::is_same_v<decltype(std::declval<Version &>().getMajor()), std::uint32_t &>);
  static_assert(std::is_same_v<decltype(std::declval<const Version &>().getMajor()), const std::uint32_t &>);

  // Formatting always emits three decimal components, including zero defaults and uint32_t endpoints.
  TEST(CoreVersionTest, FormatsThreeNumericComponents)
  {
    constexpr auto Maximum = std::numeric_limits<std::uint32_t>::max();
    struct Case
    {
        Version Value;
        std::string_view Expected;
    };
    const Case Cases[] = {
        {Version(), "0.0.0"},
        {Version(7), "7.0.0"},
        {Version(1, 2, 3), "1.2.3"},
        {Version(Maximum, 0, Maximum), "4294967295.0.4294967295"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Expected);
      EXPECT_EQ(Entry.Value.toString(), Entry.Expected);
    }
  }

  // Parsing accepts exactly three decimal components across the full uint32_t range and permits leading zeros.
  TEST(CoreVersionTest, ParsesThreeDecimalComponents)
  {
    constexpr auto Maximum = std::numeric_limits<std::uint32_t>::max();
    struct Case
    {
        std::string_view Text;
        Version Expected;
    };
    const Case Cases[] = {
        {"0.0.0", Version()},
        {"1.2.3", Version(1, 2, 3)},
        {"4294967295.4294967295.4294967295", Version(Maximum, Maximum, Maximum)},
        {"0001.02.003", Version(1, 2, 3)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      const auto Parsed = Version::parse(Entry.Text);
      ASSERT_TRUE(Parsed.has_value());
      EXPECT_EQ(*Parsed, Entry.Expected);
    }
  }

  // Missing or extra components, partial numeric input, signs, whitespace, and overflowing components are rejected.
  TEST(CoreVersionTest, RejectsInvalidOrOverflowingText)
  {
    const std::string_view Cases[] = {
        "",
        "1",
        "1.2",
        "1.2.3.4",
        "1.2.3.",
        ".1.2",
        "1..3",
        "1.2.",
        "a.2.3",
        "1.2.3x",
        "0x1.2.3",
        "+1.2.3",
        "1.-2.3",
        "1.2.+3",
        " 1.2.3",
        "1. 2.3",
        "1.2.3 ",
        "1.2.3\n",
        "4294967296.0.0",
        "0.4294967296.0",
        "0.0.4294967296",
        "999999999999999999999999999999.0.0",
        std::string_view("1.2.3\0", 6),
    };
    for (const auto Text : Cases)
    {
      SCOPED_TRACE(Text);
      EXPECT_FALSE(Version::parse(Text));
    }
  }

  // Parsed versions own numeric values and copying or changing the original text cannot mutate another version.
  TEST(CoreVersionTest, KeepsParsedValuesIndependentOfInputAndCopies)
  {
    std::string Text = "1.2.3";
    const auto Parsed = Version::parse(Text);
    ASSERT_TRUE(Parsed.has_value());
    Text.assign("8.9.10");
    EXPECT_EQ(*Parsed, Version(1, 2, 3));
    Version Copy = *Parsed;
    Copy.getMajor() = 4;
    Copy.getMinor() = 5;
    Copy.getPatch() = 6;
    EXPECT_EQ(Copy, Version(4, 5, 6));
    EXPECT_EQ(*Parsed, Version(1, 2, 3));
    EXPECT_EQ(Parsed->toString(), "1.2.3");
  }
} // namespace ink::core::test
