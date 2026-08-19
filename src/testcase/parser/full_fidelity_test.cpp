#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>

namespace ink::parser
{
  namespace
  {
    using test::countKind;
    using test::expectFullFidelity;
    using test::hasKind;
    using test::nodeText;
    using test::parseSource;

    // Verifies that an empty file still owns its EOF token exactly once and reports a zero-width root span.
    TEST(ParserFullFidelityTest, EmptySourceRetainsTheEofSentinel)
    {
      const ParsedFile File = parseSource("");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      ASSERT_EQ(File.cst().nodes().size(), 1u);
      EXPECT_EQ(File.cst().node(File.cst().root()).TokenCount, 1u);
      EXPECT_EQ(File.cst().node(File.cst().root()).TextLength, 0u);
      EXPECT_EQ(File.cst().node(File.cst().root()).Flags, CstNodeFlags::None);
      expectFullFidelity(File);
    }

    // Verifies exact token ownership, byte reconstruction, metadata, and trivia preservation in a representative valid file.
    TEST(ParserFullFidelityTest, ValidFilePreservesEverySourceByteAndRecomputableNodeMetadata)
    {
      const std::string Source = "\xEF\xBB\xBF"
                                 "/* leading */\r\nimport core.io;\r\npublic const Answer: i32 = 42;\r\nfunc compute(value: i32) -> i32\r\n{\r\n  // body\r\n  var copy = value;\r\n  copy += 1;\r\n  return copy;\r\n}\r\n// trailing\r\n";
      const ParsedFile File = parseSource(Source);

      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(test::testDiagnostics(File).empty());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      EXPECT_TRUE(hasKind(File, CstKind::ModuleImportDeclaration));
      EXPECT_TRUE(hasKind(File, CstKind::TopLevelBindingDeclaration));
      EXPECT_TRUE(hasKind(File, CstKind::FunctionDeclaration));
      EXPECT_TRUE(hasKind(File, CstKind::AssignmentStatement));
      expectFullFidelity(File);
    }

    // Verifies that recovered unexpected and synthesized missing syntax propagate independent flags without losing real tokens.
    TEST(ParserFullFidelityTest, RecoveryPropagatesErrorAndMissingFlagsWhileRetainingAllTokens)
    {
      const ParsedFile File = parseSource("func broken(value: i32 { ++value; return value }");

      ASSERT_FALSE(File.succeeded());
      ASSERT_FALSE(test::testDiagnostics(File).empty());
      const CstNode &Root = File.cst().node(File.cst().root());
      EXPECT_TRUE(hasFlag(Root.Flags, CstNodeFlags::HasError));
      EXPECT_TRUE(hasFlag(Root.Flags, CstNodeFlags::HasMissing));
      ASSERT_TRUE(hasKind(File, CstKind::Error));
      for (CstNodeId Id : test::nodesOfKind(File, CstKind::Error))
      {
        EXPECT_TRUE(hasFlag(File.cst().node(Id).Flags, CstNodeFlags::HasError));
      }
      expectFullFidelity(File);
    }

    // Verifies that every node's public span is ordered, bounded, and encloses all of its real descendant tokens.
    TEST(ParserFullFidelityTest, NodeSpansStayWithinSourceByteBounds)
    {
      const ParsedFile File = parseSource("const first = (1, 2);\nfunc second() { return first[0]; }\n");

      ASSERT_TRUE(File.succeeded());
      for (CstNodeId Id = 0; Id < File.cst().nodes().size(); ++Id)
      {
        const core::SourceRange Span = File.span(Id);
        SCOPED_TRACE(Id);
        EXPECT_LE(Span.Start, Span.End);
        EXPECT_LE(Span.End, File.lexedFile().source().size());
        if (File.cst().node(Id).TextLength != 0)
        {
          EXPECT_GE(Span.size(), File.cst().node(Id).TextLength);
        }
      }
      expectFullFidelity(File);
    }

    // Verifies that ten thousand nested unary prefixes parse, measure, and reconstruct without recursive call-stack exhaustion.
    TEST(ParserFullFidelityTest, DeepUnaryPrefixChainUsesStackSafeCstTraversal)
    {
      constexpr std::size_t PrefixCount = 10000;
      std::string Source = "const Deep = ";
      for (std::size_t Index = 0; Index < PrefixCount; ++Index)
      {
        Source.append("+ ");
      }
      Source.append("Value;");

      const ParsedFile File = parseSource(Source);

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::UnaryExpression), PrefixCount);
      EXPECT_EQ(nodeText(File, File.cst().root()), Source);
      expectFullFidelity(File);
    }

    // Verifies that a five-thousand-operand additive chain parses and validates its deeply nested CST without recursive call-stack exhaustion.
    TEST(ParserFullFidelityTest, DeepAdditiveChainUsesStackSafeCstTraversal)
    {
      constexpr std::size_t OperandCount = 5000;
      std::string Source = "const Deep = Value";
      for (std::size_t Index = 1; Index < OperandCount; ++Index)
      {
        Source.append(" + Value");
      }
      Source.push_back(';');

      const ParsedFile File = parseSource(Source);

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::BinaryExpression), OperandCount - 1);
      EXPECT_EQ(nodeText(File, File.cst().root()), Source);
      expectFullFidelity(File);
    }
  } // namespace
} // namespace ink::parser
