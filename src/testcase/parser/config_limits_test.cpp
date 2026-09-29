#include "parser_test_support.h"
#include "../core/environment_test_support.h"

#include "ink/parser/ast_serialization.h"

namespace ink::parser::test
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

  // Every archive budget has an independent configured default without replacing caller-assigned values.
  TEST(ConfigLimitsTest, ArchiveDefaultsFollowEnvironment)
  {
    const LimitConfigCase<ASTArchiveLimits> Cases[] = {
        {"INK_AST_ARCHIVE_MAX_BYTES", &ASTArchiveLimits::MaxArchiveBytes, 256 * 1024 * 1024},
        {"INK_AST_ARCHIVE_MAX_SOURCE_BYTES", &ASTArchiveLimits::MaxSourceBytes, 64 * 1024 * 1024},
        {"INK_AST_ARCHIVE_MAX_NODES", &ASTArchiveLimits::MaxNodes, 1000000},
        {"INK_AST_ARCHIVE_MAX_TOKENS", &ASTArchiveLimits::MaxTokens, 2000000},
        {"INK_AST_ARCHIVE_MAX_ARRAY_ELEMENTS", &ASTArchiveLimits::MaxArrayElements, 1000000},
        {"INK_AST_ARCHIVE_MAX_ALLOCATION_BYTES", &ASTArchiveLimits::MaxAllocationBytes, 256 * 1024 * 1024},
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

  // Both archive entry points enforce configured size limits and accept an explicit exact-size budget.
  TEST_F(ParserTest, ConfiguredArchiveBudgetAndExplicitOverride)
  {
    const auto Parsed = read("var x = 1;");
    ASSERT_TRUE(Parsed.succeeded());
    core::test::ScopedEnvironmentVariable Environment("INK_AST_ARCHIVE_MAX_BYTES");
    ASSERT_TRUE(Environment.set(nullptr));
    const auto Archive = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Archive.succeeded());
    ASSERT_TRUE(Environment.set("0"));
    EXPECT_DEATH(serializeAST(Frontend, Parsed), "internal compiler error\\[INK-P0013\\]");
    EXPECT_DEATH(deserializeAST(Frontend, Archive.Bytes), "internal compiler error\\[INK-P0013\\]");
    ASTArchiveLimits Limits;
    Limits.MaxArchiveBytes = Archive.Bytes.size();
    const auto ExplicitArchive = serializeAST(Frontend, Parsed, Limits);
    ASSERT_TRUE(ExplicitArchive.succeeded());
    EXPECT_EQ(ExplicitArchive.Bytes, Archive.Bytes);
    EXPECT_TRUE(deserializeAST(Frontend, Archive.Bytes, Limits).succeeded());
  }
} // namespace ink::parser::test
