#include "tokenizer_test_support.h"

namespace ink::tokenizer::test
{
  // Only an r/R immediately at a token start forms a raw literal prefix.
  TEST_F(TokenizerTest, LiteralPrefixBoundaries)
  {
    expectKinds(lex("r\"abc\" R'c'"), {TokenKind::StringLiteral, TokenKind::CharLiteral});
    expectKinds(lex("r \"abc\" xr\"abc\" f\"abc\" r/**/\"abc\""), {TokenKind::Identifier, TokenKind::StringLiteral, TokenKind::Identifier, TokenKind::StringLiteral, TokenKind::Identifier, TokenKind::StringLiteral, TokenKind::Identifier, TokenKind::StringLiteral});
    expectKinds(lex("f'c' rr'c' R 'c'"), {TokenKind::Identifier, TokenKind::CharLiteral, TokenKind::Identifier, TokenKind::CharLiteral, TokenKind::Identifier, TokenKind::CharLiteral});
  }

  // Adjacent strings remain separate regardless of whitespace or comments between them.
  TEST_F(TokenizerTest, AdjacentStringsNeverMerge)
  {
    for (const auto &Source : {"\"a\" \"b\"", "\"a\"\"b\"", "\"a\"/**/\"b\""})
    {
      const auto Buffer = lex(Source);
      expectKinds(Buffer, {TokenKind::StringLiteral, TokenKind::StringLiteral});
      EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, "a");
      EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[1].Payload).Decoded, "b");
    }
  }

  // Triple delimiters take precedence and may contain isolated one- or two-quote runs.
  TEST_F(TokenizerTest, TripleQuotesAndFirstMatchingDelimiter)
  {
    const auto Buffer = lex(R"ink("""a"b""c""" """a\"""b""" "" r"""""" """""")ink");
    expectKinds(Buffer, {TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::StringLiteral});
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, "a\"b\"\"c");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[1].Payload).Decoded, "a\"\"\"b");
    EXPECT_TRUE(std::get<StringInfo>(Buffer.tokens()[2].Payload).Decoded.empty());
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[3].Payload).Mode, StringMode::RawMultiline);
    EXPECT_TRUE(std::get<StringInfo>(Buffer.tokens()[4].Payload).Decoded.empty());
    expectKinds(lex(R"ink("""x"""name)ink"), {TokenKind::StringLiteral, TokenKind::Identifier});
  }

  // Triple openers must close as triple strings; single quotes always introduce a character.
  TEST_F(TokenizerTest, UnterminatedLiteralsDoNotBacktrack)
  {
    for (const auto &Source : {"\"", "\"abc", "\"\"\"", "\"\"\"\"", "\"\"\"\"\"", "r\"\"\"", "R\"\"\"abc\"\"", "r\"abc"})
    {
      expectError(Source, DiagnosticKind::UnterminatedString);
    }
    for (const auto &Source : {"'", "'a", "r'", "R'a"})
    {
      expectError(Source, DiagnosticKind::UnterminatedCharacter);
    }
    expectError("'''", DiagnosticKind::InvalidCharacterLength);
    expectError("'''a'''", DiagnosticKind::InvalidCharacterLength);
  }

  // Raw backslashes never escape quotes and can appear in any number before a closing delimiter.
  TEST_F(TokenizerTest, RawBackslashesDoNotAffectDelimiters)
  {
    for (std::size_t Count = 0; Count < 5; ++Count)
    {
      const auto Buffer = lex("r\"a" + std::string(Count, '\\') + "\"");
      expectKinds(Buffer, {TokenKind::StringLiteral});
      EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, "a" + std::string(Count, '\\'));
    }
    const auto Buffer = lex(R"ink(r"\n\x41" R"""a\""" r'\')ink");
    expectKinds(Buffer, {TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::CharLiteral});
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, "\\n\\x41");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[1].Payload).Decoded, "a\\");
    EXPECT_EQ(std::get<CharInfo>(Buffer.tokens()[2].Payload).Value, U'\\');
    EXPECT_TRUE(std::get<CharInfo>(Buffer.tokens()[2].Payload).Raw);
  }

  // Physical newline normalization precedes escapes; line continuations preserve following indentation.
  TEST_F(TokenizerTest, PhysicalNewlinesNormalizeBeforeEscapeProcessing)
  {
    for (const std::string Newline : {"\n", "\r\n", "\r"})
    {
      const auto Ordinary = lex("\"\"\"a" + Newline + "b\\r\"\"\"");
      expectKinds(Ordinary, {TokenKind::StringLiteral});
      EXPECT_EQ(std::get<StringInfo>(Ordinary.tokens()[0].Payload).Decoded, "a\nb\r");
      const auto Raw = lex("r\"\"\"a\\" + Newline + "b\"\"\"");
      expectKinds(Raw, {TokenKind::StringLiteral});
      EXPECT_EQ(std::get<StringInfo>(Raw.tokens()[0].Payload).Decoded, "a\\\nb");
      const auto Continued = lex("\"a\\" + Newline + "  b\"");
      expectKinds(Continued, {TokenKind::StringLiteral});
      EXPECT_EQ(std::get<StringInfo>(Continued.tokens()[0].Payload).Decoded, "a  b");
      const auto Character = lex("'\\" + Newline + "a'");
      expectKinds(Character, {TokenKind::CharLiteral});
      EXPECT_EQ(std::get<CharInfo>(Character.tokens()[0].Payload).Value, U'a');
    }
  }

  // Short raw literals reject every physical newline; ordinary literals require an immediate backslash.
  TEST_F(TokenizerTest, ShortLiteralNewlineRestrictions)
  {
    for (const std::string Newline : {"\n", "\r\n", "\r"})
    {
      for (const std::string Prefix : {"\"", "r\"", "'", "r'"})
      {
        expectError(Prefix + "a" + Newline, DiagnosticKind::NewlineInLiteral);
      }
      expectError("r\"a\\" + Newline, DiagnosticKind::NewlineInLiteral);
      expectError("r'\\" + Newline, DiagnosticKind::NewlineInLiteral);
      expectError("\"a\\ " + Newline, DiagnosticKind::InvalidEscape);
      expectError("'\\" + Newline + "'", DiagnosticKind::InvalidCharacterLength);
    }
  }

  // Character length counts Unicode scalars, not UTF-8 bytes, UTF-16 code units or grapheme clusters.
  TEST_F(TokenizerTest, CharacterLiteralsContainOneScalar)
  {
    struct Case
    {
        const char *Source;
        char32_t Value;
    };
    const Case Cases[] = {
        {"'a'", U'a'},
        {u8"'\u4E2D'", U'\u4E2D'},
        {u8"'\U0001F600'", U'\U0001F600'},
        {"'\\U0001F600'", U'\U0001F600'},
        {"'\\xFF'", U'\u00FF'},
        {"'\\0'", U'\0'},
        {"'\\u0301'", U'\u0301'},
        {"'\\n'", U'\n'},
        {"'\\''", U'\''},
        {"r'\\'", U'\\'},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      const auto Buffer = lex(Entry.Source);
      expectKinds(Buffer, {TokenKind::CharLiteral});
      EXPECT_EQ(std::get<CharInfo>(Buffer.tokens()[0].Payload).Value, Entry.Value);
    }
    for (const auto &Source : {"''", "r''", "'ab'", "r'\\n'", "'e\\u0301'", "'\\x414'", u8"'\U0001F1E8\U0001F1F3'"})
    {
      expectError(Source, DiagnosticKind::InvalidCharacterLength);
    }
    expectError("'\\uD83D\\uDE00'", DiagnosticKind::InvalidUnicodeScalar);
  }

  // Literal values remain unnormalized and decoded escapes are never reinterpreted as source.
  TEST_F(TokenizerTest, LiteralValuesAreNeitherNormalizedNorRelexed)
  {
    const auto Buffer = lex(R"ink("e\u0301" "\x5Cn" "\x22" "\x2F\x2A" "\x00")ink");
    expectKinds(Buffer, {TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::StringLiteral});
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, u8"e\u0301");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[1].Payload).Decoded, "\\n");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[2].Payload).Decoded, "\"");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[3].Payload).Decoded, "/*");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[4].Payload).Decoded, std::string(1, '\0'));
  }
} // namespace ink::tokenizer::test
