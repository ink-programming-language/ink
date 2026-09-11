#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    std::string nestedArraySource(std::size_t Depth)
    {
      return "let Deep: Data = " + std::string(Depth, '[') + "0" + std::string(Depth, ']') + ";";
    }

    std::string nestedTupleTypeSource(std::size_t Depth)
    {
      std::string Type = "int32";
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Type = "(" + Type + ", int32)";
      }
      return "func deep(Value: " + Type + ") -> void;";
    }

    std::string nestedTuplePatternSource(std::size_t Depth)
    {
      std::string Pattern = "Value";
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Pattern = "(" + Pattern + ", _)";
      }
      return "let " + Pattern + ": Data = Input;";
    }

    std::string repeatedNestedSource(std::string_view Prefix, std::string_view Middle, std::string_view Suffix, std::size_t Depth)
    {
      std::string Source;
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Source += Prefix;
      }
      Source += Middle;
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Source += Suffix;
      }
      return Source;
    }

    // Verifies an exact array nesting boundary and the next rejected level under the configured budget.
    TEST(ParserNestingLimitTest, HonorsConfiguredExpressionBoundary)
    {
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 32;
      const ParsedFile WithinLimit = test::parseSource(nestedArraySource(30), Options);
      const ParsedFile BeyondLimit = test::parseSource(nestedArraySource(31), Options);
      EXPECT_TRUE(WithinLimit.succeeded());
      EXPECT_FALSE(BeyondLimit.succeeded());
      EXPECT_TRUE(test::hasDiagnostic(BeyondLimit, core::DiagnosticKind::SyntaxNestingLimit));
      test::expectAstIntegrity(WithinLimit);
      test::expectAstIntegrity(BeyondLimit);
    }

    // Verifies genuine nesting in arrays, tuples, patterns, blocks, declarations, comptime regions, and generics produces bounded deterministic recovery.
    TEST(ParserNestingLimitTest, RecoversDeepSyntaxDeterministically)
    {
      constexpr std::size_t Depth = 768;
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 32;
      const std::vector<std::string> Sources = {
          nestedArraySource(Depth),
          nestedTupleTypeSource(Depth),
          nestedTuplePatternSource(Depth),
          repeatedNestedSource("{", "", "}", Depth),
          repeatedNestedSource("class Nested {", "", "}", Depth),
          repeatedNestedSource("comptime {", "", "}", Depth),
          "let Deep: type = " + repeatedNestedSource("Generic::[", "Value", "]", Depth) + ";",
          repeatedNestedSource("func f() -> void {", "", "}", Depth),
      };
      for (std::size_t Index = 0; Index < Sources.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        const ParsedFile First = test::parseSource(Sources[Index], Options);
        const ParsedFile Second = test::parseSource(Sources[Index], Options);
        EXPECT_FALSE(First.succeeded());
        EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
        EXPECT_TRUE(test::hasDiagnostic(First, core::DiagnosticKind::SyntaxNestingLimit));
        EXPECT_EQ(test::astSnapshot(First), test::astSnapshot(Second));
        EXPECT_TRUE(test::diagnosticsEqual(First, Second));
        test::expectAstIntegrity(First);
      }
    }

    // Verifies a long else-if list consumes no recursive syntax budget and retains its complete branch structure.
    TEST(ParserNestingLimitTest, LongElseIfListsUseIteration)
    {
      constexpr std::size_t BranchCount = 1000;
      std::string Source = "comptime if (Ready) {}";
      for (std::size_t Index = 1; Index < BranchCount; ++Index)
      {
        Source += " else if (Ready) {}";
      }
      Source += " else {}";
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 8;
      const ParsedFile File = test::parseSource(Source, Options);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(test::countKind(File, AstKind::IfStatement), BranchCount);
      EXPECT_EQ(test::countKind(File, AstKind::ComptimeStatement), 1u);
      test::expectAstIntegrity(File);
    }

    // Verifies ten thousand comptime prefixes do not recurse through parseStatement.
    TEST(ParserNestingLimitTest, LongComptimePrefixListsUseIteration)
    {
      constexpr std::size_t PrefixCount = 10000;
      std::string Source = repeatedNestedSource("comptime ", "Value;", "", PrefixCount);
      const ParsedFile File = test::parseSource(Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(test::countKind(File, AstKind::ComptimeStatement), PrefixCount);
      test::expectAstIntegrity(File);
    }

    // Verifies long function-type result chains and postfix call chains stay stack safe.
    TEST(ParserNestingLimitTest, LongTypeAndPostfixChainsUseIteration)
    {
      constexpr std::size_t ChainLength = 5000;
      const std::string TypeSource = "let Deep: type = " + repeatedNestedSource("func() -> ", "int32", "", ChainLength) + ";";
      const ParsedFile Type = test::parseSource(TypeSource);
      ASSERT_TRUE(Type.succeeded());
      EXPECT_EQ(test::countKind(Type, AstKind::FunctionTypeExpression), ChainLength);
      test::expectAstIntegrity(Type);
      const std::string CallSource = "Factory" + repeatedNestedSource("()", "", "", ChainLength) + ";";
      const ParsedFile Call = test::parseSource(CallSource);
      ASSERT_TRUE(Call.succeeded());
      EXPECT_EQ(test::countKind(Call, AstKind::CallExpression), ChainLength);
      test::expectAstIntegrity(Call);
    }

    // Verifies a zero nesting budget reports failure while retaining source bytes and a well-formed error AST.
    TEST(ParserNestingLimitTest, ZeroBudgetReturnsStructuredFailure)
    {
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 0;
      const ParsedFile File = test::parseSource("let Value: int32 = 1;", Options);
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(test::hasDiagnostic(File, core::DiagnosticKind::SyntaxNestingLimit));
      test::expectAstIntegrity(File);
    }
  } // namespace
} // namespace ink::parser
