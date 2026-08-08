#include "ink/core/diagnostic.h"

#include <gtest/gtest.h>

namespace ink::core
{
  namespace
  {
    // Verifies that SourceRange models a half-open source-byte range.
    TEST(SourceRangeTest, ReportsSizeAndEmptyState)
    {
      constexpr SourceRange Range{4, 9};
      constexpr SourceRange EmptyRange{7, 7};

      EXPECT_EQ(Range.size(), 5u);
      EXPECT_FALSE(Range.empty());
      EXPECT_EQ(EmptyRange.size(), 0u);
      EXPECT_TRUE(EmptyRange.empty());
    }

    // Verifies that a default diagnostic has a neutral kind and an empty source range.
    TEST(DiagnosticTest, DefaultsToUnknownAtAnEmptyRange)
    {
      const Diagnostic Value;

      EXPECT_EQ(Value.Kind, DiagnosticKind::Unknown);
      EXPECT_EQ(Value.Span, (SourceRange{}));
      EXPECT_TRUE(Value.Message.empty());
    }

    // Verifies diagnostic value equality and stable human-readable kind names.
    TEST(DiagnosticTest, ComparesValuesAndNamesKinds)
    {
      const Diagnostic Left{DiagnosticKind::InvalidUtf8, {2, 4}, "invalid UTF-8"};
      const Diagnostic Equal{DiagnosticKind::InvalidUtf8, {2, 4}, "invalid UTF-8"};
      const Diagnostic Different{DiagnosticKind::UnexpectedBom, {2, 4}, "invalid UTF-8"};

      EXPECT_EQ(Left, Equal);
      EXPECT_NE(Left, Different);
      EXPECT_STREQ(diagnosticKindName(DiagnosticKind::Unknown), "unknown diagnostic");
      EXPECT_STREQ(diagnosticKindName(DiagnosticKind::InvalidUtf8), "invalid UTF-8");
    }
  } // namespace
} // namespace ink::core
