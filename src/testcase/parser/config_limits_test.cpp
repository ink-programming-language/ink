#include "parser_test_support.h"
#include "../core/support/environment_test_support.h"

namespace ink::frontend::parser::test
{
  namespace
  {
    template <typename Limits>
    struct LimitConfigCase
    {
        const char *EnvironmentName;
        std::size_t Limits::*Field;
        std::size_t DefaultValue;
    };

    template <typename Limits>
    void expectConfiguredLimit(const LimitConfigCase<Limits> &Entry)
    {
      SCOPED_TRACE(Entry.EnvironmentName);
      core::test::ScopedEnvironmentVariable Environment(Entry.EnvironmentName);
      ASSERT_TRUE(Environment.set(nullptr));
      EXPECT_EQ(Limits{}.*Entry.Field, Entry.DefaultValue);
      ASSERT_TRUE(Environment.set("17"));
      Limits Snapshot;
      EXPECT_EQ(Snapshot.*Entry.Field, 17U);
      ASSERT_TRUE(Environment.set("29"));
      EXPECT_EQ(Limits{}.*Entry.Field, 29U);
      EXPECT_EQ(Snapshot.*Entry.Field, 17U);
      Snapshot.*Entry.Field = 41;
      ASSERT_TRUE(Environment.set("invalid"));
      EXPECT_EQ(Limits{}.*Entry.Field, Entry.DefaultValue);
      EXPECT_EQ(Snapshot.*Entry.Field, 41U);
      ASSERT_TRUE(Environment.set("0"));
      EXPECT_EQ(Limits{}.*Entry.Field, 0U);
    }
  } // namespace

  // Every parser budget uses its own environment key, snapshots overrides and falls back for invalid values.
  TEST(ConfigLimitsTest, ParserDefaultsFollowEnvironment)
  {
    const LimitConfigCase<ParseLimits> Cases[] = {
        {"INK_PARSER_MAX_NESTING_DEPTH", &ParseLimits::MaxNestingDepth, 128},
        {"INK_PARSER_MAX_DIAGNOSTICS", &ParseLimits::MaxDiagnostics, 100},
        {"INK_PARSER_MAX_WORK", &ParseLimits::MaxWork, 10000000},
        {"INK_PARSER_MAX_ALLOCATION_BYTES", &ParseLimits::MaxAllocationBytes, 64 * 1024 * 1024},
    };
    for (const auto &Entry : Cases)
    {
      expectConfiguredLimit(Entry);
    }
  }

  // parse() observes the configured work budget, while an explicit limit still permits the same input.
  TEST_F(ParserTest, ConfiguredWorkBudgetAndExplicitOverride)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_PARSER_MAX_WORK");
    ASSERT_TRUE(Environment.set("0"));
    EXPECT_DEATH(read("var x = 1;"), "internal compiler error\\[INK-P0004\\]");
    ParseLimits Limits;
    Limits.MaxWork = 1000;
    EXPECT_TRUE(read("var x = 1;", Limits).succeeded());
  }

  // A configured zero diagnostic budget suppresses user diagnostics but preserves the syntax error status.
  TEST_F(ParserTest, ConfiguredDiagnosticBudgetAndExplicitOverride)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_PARSER_MAX_DIAGNOSTICS");
    ASSERT_TRUE(Environment.set("0"));
    EXPECT_TRUE(read("; ;").HasSyntaxErrors);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    ParseLimits Limits;
    Limits.MaxDiagnostics = 1;
    EXPECT_TRUE(read("; ;", Limits).HasSyntaxErrors);
    EXPECT_EQ(Diagnostics.diagnostics().size(), 1U);
  }
} // namespace ink::frontend::parser::test
