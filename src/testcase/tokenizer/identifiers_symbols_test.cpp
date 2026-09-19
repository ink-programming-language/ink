#include "tokenizer_test_support.h"

#include <fstream>
#include <iterator>
#include <set>

namespace ink::tokenizer::test
{
  // Identifier boundaries use XID_Start plus underscore and the union of both Clang XID tables.
  TEST_F(TokenizerTest, UnicodeIdentifierBoundaries)
  {
    const auto Buffer = lex(u8"_ __ _1 a1 a\u0301 \u03B1\u03B2 \u4E2D\u6587 \U0002EBF0");
    expectKinds(Buffer, {TokenKind::Underscore, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier});
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[4].Payload).Name, u8"\u00E1");
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[5].Payload).Name, u8"\u03B1\u03B2");
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[6].Payload).Name, u8"\u4E2D\u6587");
    expectError(utf8(0x0301) + "a", DiagnosticKind::InvalidCharacter);
    expectError(utf8(0x0661), DiagnosticKind::InvalidCharacter);
  }

  // Canonically equivalent names share NFC payloads without applying compatibility folding or changing case.
  TEST_F(TokenizerTest, IdentifierNormalizationAndCase)
  {
    const auto Buffer = lex(u8"e\u0301 \u00E9 \u212A K k \uFB00 \uFF21");
    expectKinds(Buffer, {TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier});
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[0].Payload).Name, std::get<IdentifierInfo>(Buffer.tokens()[1].Payload).Name);
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[2].Payload).Name, "K");
    EXPECT_NE(std::get<IdentifierInfo>(Buffer.tokens()[3].Payload).Name, std::get<IdentifierInfo>(Buffer.tokens()[4].Payload).Name);
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[5].Payload).Name, u8"\uFB00");
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[6].Payload).Name, u8"\uFF21");
    EXPECT_NE(Buffer.raw(Buffer.tokens()[0]), Buffer.raw(Buffer.tokens()[1]));
  }

  // Removed keywords and prefixes of real keywords are ordinary identifiers.
  TEST_F(TokenizerTest, OnlyCompleteNewKeywordsAreReserved)
  {
    const auto Buffer = lex("true false null bool int32 let self class_field class_method extends extern asx _as AS Match field2 yield_");
    ASSERT_TRUE(Buffer.succeeded());
    ASSERT_EQ(Buffer.tokens().size(), 18U);
    for (std::size_t Index = 0; Index + 1 < Buffer.tokens().size(); ++Index)
    {
      EXPECT_EQ(Buffer.tokens()[Index].Kind, TokenKind::Identifier) << Buffer.raw(Buffer.tokens()[Index]);
    }
    expectKinds(lex("case default do field match switch yield"), {TokenKind::KwCase, TokenKind::KwDefault, TokenKind::KwDo, TokenKind::KwField, TokenKind::KwMatch, TokenKind::KwSwitch, TokenKind::KwYield});
  }

  // Every keyword and symbol spelling is checked against the normative document, independently of token.def.
  TEST_F(TokenizerTest, SpellingsMatchLexicalDocument)
  {
    std::ifstream Input(INK_LEXICAL_RULES_PATH, std::ios::binary);
    ASSERT_TRUE(Input);
    std::string Line;
    std::set<std::string> Names;
    while (std::getline(Input, Line))
    {
      const std::size_t Separator = Line.find(" ::= \"");
      if (Separator == std::string::npos)
      {
        continue;
      }
      const std::string Name = Line.substr(0, Separator);
      const std::size_t Begin = Separator + 6;
      const std::size_t End = Line.find('"', Begin);
      ASSERT_NE(End, std::string::npos);
      const std::string Spelling = Line.substr(Begin, End - Begin);
      SCOPED_TRACE(Name);
      const auto Buffer = lex(Spelling);
      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      const auto Kind = Buffer.tokens()[0].Kind;
      EXPECT_EQ(tokenKindName(Kind), Name);
      EXPECT_EQ(tokenSpelling(Kind), Spelling);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Spelling);
      EXPECT_TRUE(Names.insert(Name).second);
      if (Name.compare(0, 3, "KW_") == 0)
      {
        EXPECT_TRUE(isKeyword(Kind));
        EXPECT_FALSE(isSymbol(Kind));
        EXPECT_EQ(lookupKeyword(Spelling), Kind);
        expectKinds(lex(Spelling + "_tail"), {TokenKind::Identifier});
      }
      else
      {
        EXPECT_TRUE(isSymbol(Kind));
        EXPECT_FALSE(isKeyword(Kind));
        EXPECT_EQ(lookupSymbol(Spelling), Kind);
      }
    }
    EXPECT_EQ(Names.size(), 78U);
    EXPECT_FALSE(lookupKeyword("true"));
    EXPECT_FALSE(lookupSymbol("@"));
    EXPECT_TRUE(tokenSpelling(TokenKind::Identifier).empty());
    EXPECT_STREQ(tokenKindName(TokenKind::EndOfFile), "END_OF_FILE");
    EXPECT_STREQ(tokenKindName(TokenKind::CharLiteral), "CHAR_LITERAL");
  }

  // Overlapping operator prefixes always choose the longest supported token.
  TEST_F(TokenizerTest, LongestOperatorMatch)
  {
    expectKinds(lex("<<=<< < >>=>> > ... .. . ++ + -- - ?? ?. ? => = -> - :: :"), {TokenKind::ShiftLeftAssign, TokenKind::ShiftLeft, TokenKind::Less, TokenKind::ShiftRightAssign, TokenKind::ShiftRight, TokenKind::Greater, TokenKind::Ellipsis, TokenKind::Dot, TokenKind::Dot, TokenKind::Dot, TokenKind::PlusPlus, TokenKind::Plus, TokenKind::MinusMinus, TokenKind::Minus, TokenKind::NullCoalesce, TokenKind::QuestionDot, TokenKind::Question, TokenKind::FatArrow, TokenKind::Assign, TokenKind::Arrow, TokenKind::Minus, TokenKind::DoubleColon, TokenKind::Colon});
    expectKinds(lex(">>>= ==== &&& ||| +++ ---"), {TokenKind::ShiftRight, TokenKind::GreaterEqual, TokenKind::EqualEqual, TokenKind::EqualEqual, TokenKind::AmpAmp, TokenKind::Amp, TokenKind::PipePipe, TokenKind::Pipe, TokenKind::PlusPlus, TokenKind::Plus, TokenKind::MinusMinus, TokenKind::Minus});
  }
} // namespace ink::tokenizer::test
