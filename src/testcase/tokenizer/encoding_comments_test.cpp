#include "tokenizer_test_support.h"

namespace ink::tokenizer::test
{
  // Empty input and each supported physical newline yield exactly one zero-length EOF.
  TEST_F(TokenizerTest, EmptyInputWhitespaceAndPhysicalNewlines)
  {
    for (const std::string Source : {"", " ", "\t", "\n", "\r\n", "\r", " \t\r\n\r\n\r"})
    {
      SCOPED_TRACE(Source);
      expectKinds(lex(Source), {});
    }
    expectKinds(lex("a\r\nb\rc\nd\t e"), {TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier});
  }

  // Unsupported whitespace, formatting marks, backslashes and punctuation never disappear.
  TEST_F(TokenizerTest, InvalidCharactersOutsideLiterals)
  {
    for (const char32_t Scalar : {U'\v', U'\f', U'\u0085', U'\u00A0', U'\u1680', U'\u2000', U'\u2028', U'\u2029', U'\u3000', U'\uFEFF', U'\\', U'@', U'#', U'\u0301', U'\U0001F600'})
    {
      expectError(" " + utf8(Scalar), DiagnosticKind::InvalidCharacter);
    }
    expectError("\\\n", DiagnosticKind::InvalidCharacter);
  }

  // Invalid UTF-8 is rejected in every lexical region, including raw literals and both comment forms.
  TEST_F(TokenizerTest, StrictUtf8InEveryRegion)
  {
    const std::string InvalidSequences[] = {
        "\x80",
        "\xBF",
        "\xC0\x80",
        "\xC1\xBF",
        "\xC2",
        "\xC2\x41",
        "\xE0\x80\x80",
        "\xE2\x82",
        "\xED\xA0\x80",
        "\xED\xBF\xBF",
        "\xF0\x80\x80\x80",
        "\xF0\x9F\x98",
        "\xF4\x90\x80\x80",
        "\xF5\x80\x80\x80",
        "\xFE",
        "\xFF",
    };
    for (const auto &Bytes : InvalidSequences)
    {
      for (const auto &Prefix : {"", "\"", "r\"", "\"\"\"", "'", "r'", "//", "/*"})
      {
        expectError(std::string(Prefix) + Bytes, DiagnosticKind::InvalidUtf8);
        EXPECT_TRUE(Diagnostics.diagnostics()[0].Span.getBegin().getByteOffset() == std::string_view(Prefix).size());
      }
    }
  }

  // Raw NUL is forbidden everywhere while a leading BOM is rejected before scanning.
  TEST_F(TokenizerTest, GlobalNullAndBomRestrictions)
  {
    expectError("\xEF\xBB\xBF", DiagnosticKind::UnexpectedBom);
    expectError("\xEF\xBB\xBF\"a\"", DiagnosticKind::UnexpectedBom);
    for (const auto &Prefix : {"", "\"", "r\"", "\"\"\"", "'", "r'", "//", "/*"})
    {
      expectError(std::string(Prefix) + std::string(1, '\0'), DiagnosticKind::NullInSource);
    }
  }

  // Valid Unicode controls and noncharacters are legal content except for raw NUL and a file-leading BOM.
  TEST_F(TokenizerTest, UnicodeContentHasNoExtraInvisibleCharacterBan)
  {
    const std::string Content = utf8(0xFEFF) + utf8(0x0001) + utf8(0xFFFF) + utf8(0x10FFFF);
    auto Buffer = lex("\"" + Content + "\"");
    expectKinds(Buffer, {TokenKind::StringLiteral});
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, Content);
    expectKinds(lex("/*" + Content + "*/"), {});
    expectKinds(lex("//" + Content), {});
    expectKinds(lex("'" + utf8(0xFEFF) + "'"), {TokenKind::CharLiteral});
  }

  // Comments are separators, line comments stop at all physical newline forms, and backslashes do not continue them.
  TEST_F(TokenizerTest, CommentsAreSkippedWithoutJoiningNeighbors)
  {
    expectKinds(lex("a/**/b // ignored\\\r\nc//ignored\rd//ignored\ne"), {TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier});
    expectKinds(lex("a// end of file"), {TokenKind::Identifier});
    expectKinds(lex("1/**/2"), {TokenKind::IntegerLiteral, TokenKind::IntegerLiteral});
    expectKinds(lex("+/**/+"), {TokenKind::Plus, TokenKind::Plus});
  }

  // The first closing delimiter ends a block comment even if its body contains another opener.
  TEST_F(TokenizerTest, BlockCommentsNeverNest)
  {
    expectKinds(lex("/* outer /* inner */ name */"), {TokenKind::Identifier, TokenKind::Star, TokenKind::Slash});
    expectKinds(lex("/**/ /***/ /*//\r\n*/"), {});
    for (const auto &Source : {"/*", "/*/", "/* abc", "/* /* */ /*"})
    {
      expectError(Source, DiagnosticKind::UnterminatedBlockComment);
    }
  }

  // Comment delimiters inside literals remain ordinary literal content.
  TEST_F(TokenizerTest, LiteralContentsDoNotStartComments)
  {
    const auto Buffer = lex("\"// /* */\" r\"/*\" '/'");
    expectKinds(Buffer, {TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::CharLiteral});
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, "// /* */");
  }
} // namespace ink::tokenizer::test
