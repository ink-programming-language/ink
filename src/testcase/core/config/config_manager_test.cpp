#include "../support/environment_test_support.h"

#include "ink/core/config/config_manager.h"

#include <limits>
#include <type_traits>

namespace ink::core::test
{
  // Config readers return concrete values; callers do not handle a missing-value state.
  static_assert(std::is_same_v<decltype(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>()), std::string>);
  static_assert(std::is_same_v<decltype(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>()), std::size_t>);

  // Both readers print the missing key diagnostic and terminate for unregistered enum values.
  TEST(ConfigManagerDeathTest, MissingKeysReportICEAndPanic)
  {
    EXPECT_DEATH(ConfigManager::get<ConfigKind::Count>(), "internal compiler error\\[INK-C0001\\]: configuration key .* is not registered");
    EXPECT_DEATH(ConfigManager::getSize<ConfigKind::Count>(), "internal compiler error\\[INK-C0001\\]");
    EXPECT_DEATH(ConfigManager::get<static_cast<ConfigKind>(-1)>(), "internal compiler error\\[INK-C0001\\]");
    EXPECT_DEATH(ConfigManager::getSize<static_cast<ConfigKind>(-1)>(), "internal compiler error\\[INK-C0001\\]");
  }

  // An absent environment override returns the default registered in config.def.
  TEST(ConfigManagerTest, UsesDefaultWhenEnvironmentIsAbsent)
  {
    ScopedEnvironmentVariable Environment("INK_PARSER_MAX_NESTING_DEPTH");
    ASSERT_TRUE(Environment.set(nullptr));
    EXPECT_EQ(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>(), "128");
    EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), 128U);
  }

  // Reads observe changed environment values while previously returned strings retain their own storage.
  TEST(ConfigManagerTest, CopiesOverridesAndReadsCurrentEnvironment)
  {
    ScopedEnvironmentVariable Environment("INK_PARSER_MAX_NESTING_DEPTH");
    ASSERT_TRUE(Environment.set("32"));
    const auto First = ConfigManager::get<ConfigKind::ParserMaxNestingDepth>();
    EXPECT_EQ(First, "32");
    EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), 32U);
    ASSERT_TRUE(Environment.set("64"));
    EXPECT_EQ(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>(), "64");
    EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), 64U);
    EXPECT_EQ(First, "32");
    ASSERT_TRUE(Environment.set(nullptr));
    EXPECT_EQ(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>(), "128");
  }

  // Unsigned size conversion preserves both zero and the largest representable size.
  TEST(ConfigManagerTest, AcceptsUnsignedSizeBoundaries)
  {
    ScopedEnvironmentVariable Environment("INK_PARSER_MAX_NESTING_DEPTH");
    ASSERT_TRUE(Environment.set("0"));
    EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), 0U);
    const std::string Maximum = std::to_string(std::numeric_limits<std::size_t>::max());
    ASSERT_TRUE(Environment.set(Maximum.c_str()));
    EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), std::numeric_limits<std::size_t>::max());
  }

  // Invalid decimal syntax and overflow preserve raw overrides but use the default for size reads.
  TEST(ConfigManagerTest, FallsBackForInvalidSizeOverrides)
  {
    ScopedEnvironmentVariable Environment("INK_PARSER_MAX_NESTING_DEPTH");
    const std::string InvalidValues[] = {
        "invalid",
        "-1",
        "+1",
        "1.5",
        "32extra",
        " 32",
        "32 ",
        std::to_string(std::numeric_limits<std::size_t>::max()) + "0",
    };
    for (const std::string &Value : InvalidValues)
    {
      SCOPED_TRACE(Value);
      ASSERT_TRUE(Environment.set(Value.c_str()));
      EXPECT_EQ(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>(), Value);
      EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), 128U);
    }
  }

#ifndef _WIN32
  // POSIX permits present-but-empty environment values, unlike the Windows CRT environment setter.
  TEST(ConfigManagerTest, PreservesEmptyOverrideAndFallsBackForSize)
  {
    ScopedEnvironmentVariable Environment("INK_PARSER_MAX_NESTING_DEPTH");
    ASSERT_TRUE(Environment.set(""));
    EXPECT_EQ(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>(), "");
    EXPECT_EQ(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>(), 128U);
  }
#endif
} // namespace ink::core::test
