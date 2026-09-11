#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;

    // Verifies apostrophes are invalid characters and no character-literal lexical mode consumes their contents.
    TEST(LiteralBoundaryTest, SingleQuotesDoNotIntroduceScalarLiterals)
    {
      const TokenizedBuffer File = tokenize("'a' ''");
      ASSERT_FALSE(File.succeeded());
      expectRaws(File, {"'", "a", "'", "'", "'"});
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(File.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(File.tokens()[2].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(testDiagnostics(File).size(), 4U);
    }

    // Verifies a lone quote does not hide a following keyword or physical line.
    TEST(LiteralBoundaryTest, SingleQuoteErrorDoesNotConsumeFollowingStatement)
    {
      const TokenizedBuffer File = tokenize("'\rlet value: int32 = 1;");
      ASSERT_FALSE(File.succeeded());
      expectRaws(File, {"'", "let", "value", ":", "int32", "=", "1", ";"});
      EXPECT_EQ(File.tokens()[1].Kind, TokenKind::Keyword);
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 2}));
    }

    // Verifies strings, identifiers and numbers remain distinct without intervening whitespace.
    TEST(LiteralBoundaryTest, AdjacentLiteralKindsDoNotRequireWhitespace)
    {
      const TokenizedBuffer File = tokenize("\"x\"name 1\"y\" 2r\"z\"");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"\"x\"", "name", "1", "\"y\"", "2", "r\"z\""});
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::StringLiteral);
      EXPECT_EQ(File.tokens()[5].Kind, TokenKind::StringLiteral);
    }

    // Verifies raw prefixes are recognized only at token starts and uppercase R remains an identifier.
    TEST(LiteralBoundaryTest, RawPrefixMustBeLowercaseAndAdjacent)
    {
      const TokenizedBuffer File = tokenize("R\"x\" r \"y\" ar\"z\" r\"w\"");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"R", "\"x\"", "r", "\"y\"", "ar", "\"z\"", "r\"w\""});
      const StringInfo *Payload = std::get_if<StringInfo>(&File.tokens()[6].Payload);
      ASSERT_NE(Payload, nullptr);
      EXPECT_EQ(Payload->Mode, StringMode::RawSingleLine);
    }
  } // namespace
} // namespace ink::tokenizer