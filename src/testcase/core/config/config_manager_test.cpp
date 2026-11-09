#include "../support/environment_test_support.h"

#include "ink/core/config/config_manager.h"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ink::core::test
{
  // Config readers return concrete values; callers do not handle a missing-value state.
  static_assert(std::is_same_v<decltype(ConfigManager::get<ConfigKind::ParserMaxNestingDepth>()), std::string>);
  static_assert(std::is_same_v<decltype(ConfigManager::getSize<ConfigKind::ParserMaxNestingDepth>()), std::size_t>);
  static_assert(std::is_same_v<decltype(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>()), Version>);
  static_assert(std::is_same_v<decltype(ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>()), Version>);

  // All readers print the missing key diagnostic and terminate for unregistered enum values.
  TEST(ConfigManagerDeathTest, MissingKeysReportICEAndPanic)
  {
    EXPECT_DEATH(ConfigManager::get<ConfigKind::Count>(), "internal compiler error\\[INK-C0001\\]: configuration key .* is not registered");
    EXPECT_DEATH(ConfigManager::getSize<ConfigKind::Count>(), "internal compiler error\\[INK-C0001\\]");
    EXPECT_DEATH(ConfigManager::getVersion<ConfigKind::Count>(), "internal compiler error\\[INK-C0001\\]");
    EXPECT_DEATH(ConfigManager::get<static_cast<ConfigKind>(-1)>(), "internal compiler error\\[INK-C0001\\]");
    EXPECT_DEATH(ConfigManager::getSize<static_cast<ConfigKind>(-1)>(), "internal compiler error\\[INK-C0001\\]");
    EXPECT_DEATH(ConfigManager::getVersion<static_cast<ConfigKind>(-1)>(), "internal compiler error\\[INK-C0001\\]");
  }

  // Reading a non-version definition through the version reader reports the invalid default and terminates.
  TEST(ConfigManagerDeathTest, InvalidVersionDefaultsReportICEAndPanic)
  {
    ScopedEnvironmentVariable Environment("INK_PARSER_MAX_NESTING_DEPTH");
    ASSERT_TRUE(Environment.set(nullptr));
    EXPECT_DEATH(ConfigManager::getVersion<ConfigKind::ParserMaxNestingDepth>(), "internal compiler error\\[INK-C0003\\]: configuration .* has invalid version default '128'");
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

  // Both object versions use the registered triplet defaults when their environment variables are absent.
  TEST(ConfigManagerTest, UsesDefaultObjectVersionsWhenEnvironmentIsAbsent)
  {
    ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set(nullptr));
    ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
    EXPECT_EQ(ConfigManager::get<ConfigKind::ObjectFormatVersion>(), "2.0.0");
    EXPECT_EQ(ConfigManager::get<ConfigKind::ObjectInstructionSetVersion>(), "1.1.0");
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(2, 0, 0));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>(), Version(1, 1, 0));
  }

  // Each version read observes the current environment while earlier values and copies remain independent.
  TEST(ConfigManagerTest, CopiesVersionOverridesAndReadsCurrentEnvironment)
  {
    ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set("2.3.4"));
    ASSERT_TRUE(InstructionSetEnvironment.set("5.6.7"));
    auto FirstFormat = ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>();
    const auto FirstInstructionSet = ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>();
    const auto FormatCopy = FirstFormat;
    ASSERT_TRUE(FormatEnvironment.set("9.8.7"));
    ASSERT_TRUE(InstructionSetEnvironment.set("6.5.4"));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(9, 8, 7));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>(), Version(6, 5, 4));
    EXPECT_EQ(FirstFormat, Version(2, 3, 4));
    EXPECT_EQ(FirstInstructionSet, Version(5, 6, 7));
    FirstFormat = Version(10, 20, 30);
    EXPECT_EQ(FormatCopy, Version(2, 3, 4));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(9, 8, 7));
    ASSERT_TRUE(FormatEnvironment.set(nullptr));
    ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(2, 0, 0));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>(), Version(1, 1, 0));
  }

  // Version conversion accepts zero and the full unsigned 32-bit range in all three components.
  TEST(ConfigManagerTest, AcceptsVersionComponentBoundaries)
  {
    ScopedEnvironmentVariable Environment("INK_OBJECT_FORMAT_VERSION");
    ASSERT_TRUE(Environment.set("0.0.0"));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(0, 0, 0));
    ASSERT_TRUE(Environment.set("4294967295.4294967295.4294967295"));
    constexpr auto Maximum = std::numeric_limits<std::uint32_t>::max();
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(Maximum, Maximum, Maximum));
  }

  // Invalid triplets and overflowing components preserve raw overrides but fall back to both registered version defaults.
  TEST(ConfigManagerTest, FallsBackForInvalidVersionOverrides)
  {
    ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    const char *InvalidValues[] = {
        "invalid",
        "1",
        "1.2",
        "1.2.3.4",
        ".2.3",
        "1..3",
        "1.2.",
        "-1.2.3",
        "+1.2.3",
        "1.-2.3",
        "1.2.+3",
        " 1.2.3",
        "1.2.3 ",
        "1.2.3extra",
        "4294967296.0.0",
        "0.4294967296.0",
        "0.0.4294967296",
        "18446744073709551616.0.0",
    };
    for (const char *Value : InvalidValues)
    {
      SCOPED_TRACE(Value);
      ASSERT_TRUE(FormatEnvironment.set(Value));
      ASSERT_TRUE(InstructionSetEnvironment.set(Value));
      EXPECT_EQ(ConfigManager::get<ConfigKind::ObjectFormatVersion>(), Value);
      EXPECT_EQ(ConfigManager::get<ConfigKind::ObjectInstructionSetVersion>(), Value);
      EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(2, 0, 0));
      EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>(), Version(1, 1, 0));
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

  // A present-but-empty override remains visible as text while typed version reads use their defaults.
  TEST(ConfigManagerTest, PreservesEmptyOverrideAndFallsBackForVersion)
  {
    ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set(""));
    ASSERT_TRUE(InstructionSetEnvironment.set(""));
    EXPECT_EQ(ConfigManager::get<ConfigKind::ObjectFormatVersion>(), "");
    EXPECT_EQ(ConfigManager::get<ConfigKind::ObjectInstructionSetVersion>(), "");
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectFormatVersion>(), Version(2, 0, 0));
    EXPECT_EQ(ConfigManager::getVersion<ConfigKind::ObjectInstructionSetVersion>(), Version(1, 1, 0));
  }
#endif
} // namespace ink::core::test
