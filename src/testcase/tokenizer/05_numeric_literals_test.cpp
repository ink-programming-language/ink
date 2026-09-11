#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;

    // Verifies decimal and both cases of every base prefix with exact raw spellings and radix metadata.
    TEST(NumericLiteralsTest, AcceptsAllIntegerBasesAndLeadingZeroForms)
    {
      const std::vector<std::pair<std::string, unsigned>> Cases = {
          {"0", 10},
          {"0009", 10},
          {"1234567890", 10},
          {"0b101", 2},
          {"0B101", 2},
          {"0o765", 8},
          {"0O765", 8},
          {"0xabcDEF09", 16},
          {"0XABCdef09", 16},
      };
      for (const auto &Entry : Cases)
      {
        const TokenizedBuffer File = tokenize(Entry.first);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Entry.first});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::IntegerLiteral);
        const NumericInfo *Payload = std::get_if<NumericInfo>(&File.tokens()[0].Payload);
        ASSERT_NE(Payload, nullptr);
        EXPECT_EQ(Payload->Base, Entry.second);
      }
    }

    // Verifies decimal floats require fractional digits or a complete exponent and retain their source text.
    TEST(NumericLiteralsTest, AcceptsAllDecimalFloatShapes)
    {
      for (const std::string &Source : {"0.0", "01.20", "1e0", "1E+2", "1e-2", "2.5e+100", "2.5E-100"})
      {
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::FloatLiteral);
        const NumericInfo *Payload = std::get_if<NumericInfo>(&File.tokens()[0].Payload);
        ASSERT_NE(Payload, nullptr);
        EXPECT_EQ(Payload->Base, 10U);
      }
    }

    // Verifies maximal munch chooses the longest valid number rather than rejecting a larger invalid candidate.
    TEST(NumericLiteralsTest, IncompleteNumericExtensionsLeaveIndependentTokens)
    {
      const std::vector<std::pair<std::string, std::vector<std::string>>> Cases = {
          {"123abc", {"123", "abc"}},
          {"0x", {"0", "x"}},
          {"0B", {"0", "B"}},
          {"0o8", {"0", "o8"}},
          {"0b102", {"0b10", "2"}},
          {"0o789", {"0o7", "89"}},
          {"0xFG", {"0xF", "G"}},
          {"1e+", {"1", "e", "+"}},
          {"1.2e-", {"1.2", "e", "-"}},
          {"1e2foo", {"1e2", "foo"}},
          {"1e2.3", {"1e2", ".", "3"}},
          {"0x1.2", {"0x1", ".", "2"}},
          {"0x1p2", {"0x1", "p2"}},
          {"0x1e+2", {"0x1e", "+", "2"}},
      };
      for (const auto &Entry : Cases)
      {
        SCOPED_TRACE(Entry.first);
        const TokenizedBuffer File = tokenize(Entry.first);
        ASSERT_TRUE(File.succeeded());
        EXPECT_TRUE(testDiagnostics(File).empty());
        expectRaws(File, Entry.second);
      }
    }

    // Verifies numeric grouping and old type suffixes are separate identifiers under the new lexical contract.
    TEST(NumericLiteralsTest, NumericSeparatorsAndSuffixesAreNotPartOfNumberTokens)
    {
      const TokenizedBuffer File = tokenize("1_000 5i32 5int32 1.0f32 0xFFu8");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"1", "_000", "5", "i32", "5", "int32", "1.0", "f32", "0xFF", "u8"});
      EXPECT_EQ(File.tokens()[5].Kind, TokenKind::Keyword);
    }

    // Verifies leading signs, incomplete decimal dots and ellipsis operators preserve their lexical boundaries.
    TEST(NumericLiteralsTest, SignsDotsAndEllipsesRemainIndependent)
    {
      const TokenizedBuffer File = tokenize("-1 +2 .5 1. 1..2 1...2 1.2...3");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"-", "1", "+", "2", ".", "5", "1", ".", "1", ".", ".", "2", "1", "...", "2", "1.2", "...", "3"});
    }

    // Verifies arbitrarily long coefficients and exponents are lexed without host integer or floating-point conversion.
    TEST(NumericLiteralsTest, LongNumbersDoNotOverflowHostStorage)
    {
      const std::string Integer(100000, '9');
      const std::string Float = Integer + ".0e+" + Integer;
      const TokenizedBuffer File = tokenize(Integer + " " + Float);
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {Integer, Float});
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(File.tokens()[1].Kind, TokenKind::FloatLiteral);
    }

    // Verifies trivia cannot occur inside a numeric prefix, fraction or exponent.
    TEST(NumericLiteralsTest, TriviaBreaksNumericContinuations)
    {
      const TokenizedBuffer File = tokenize("0 x10 1 . 2 1/*gap*/e2 1e /*gap*/ +2");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"0", "x10", "1", ".", "2", "1", "e2", "1", "e", "+", "2"});
    }
  } // namespace
} // namespace ink::tokenizer