#include "tokenizer_test_support.h"

namespace ink::tokenizer::test
{
  // Simple escapes, octal widths, fixed hexadecimal widths and scalar boundaries decode to Unicode.
  TEST_F(TokenizerTest, EscapeValuesAndExactDigitCounts)
  {
    struct Case
    {
        const char *Source;
        std::string Value;
    };
    const Case Cases[] = {
        {R"ink("\\\'\"\a\b\f\n\r\t\v")ink", "\\'\"\a\b\f\n\r\t\v"},
        {R"ink("\0")ink", std::string(1, '\0')},
        {R"ink("\00")ink", std::string(1, '\0')},
        {R"ink("\000")ink", std::string(1, '\0')},
        {R"ink("\377")ink", utf8(0xFF)},
        {R"ink("\1234")ink", "S4"},
        {R"ink("\78")ink", "\a8"},
        {R"ink("\x414")ink", "A4"},
        {R"ink("\xff")ink", utf8(0xFF)},
        {R"ink("\u00414")ink", "A4"},
        {R"ink("\U000000414")ink", "A4"},
        {R"ink("\uD7FF\uE000\U0010FFFF")ink", utf8(0xD7FF) + utf8(0xE000) + utf8(0x10FFFF)},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      const auto Buffer = lex(Entry.Source);
      expectKinds(Buffer, {TokenKind::StringLiteral});
      EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[0].Payload).Decoded, Entry.Value);
    }
  }

  // Escape introducers are case-sensitive and non-ASCII digits never satisfy numeric escapes.
  TEST_F(TokenizerTest, UnknownAndIncompleteEscapesFail)
  {
    for (const auto &Source : {"\"\\", "\"\\q\"", "\"\\8\"", "\"\\9\"", "\"\\X41\"", "\"\\A\"", "\"\\x\"", "\"\\x4\"", "\"\\xGG\"", "\"\\u123\"", "\"\\U0000000\"", "\"\\u{41}\""})
    {
      expectError(Source, DiagnosticKind::InvalidEscape);
    }
    expectError("\"\\x" + utf8(0xFF14) + "1\"", DiagnosticKind::InvalidEscape);
    expectError("\"\\" + utf8(0x4E2D) + "\"", DiagnosticKind::InvalidEscape);
    expectError("'\\q'", DiagnosticKind::InvalidEscape);
  }

  // Octal values above FF and escaped surrogates or values beyond 10FFFF fail in both literal kinds.
  TEST_F(TokenizerTest, EscapeScalarAndOctalLimits)
  {
    for (const std::string Quote : {"\"", "'"})
    {
      for (const std::string Escape : {"\\400", "\\777"})
      {
        expectError(Quote + Escape + Quote, DiagnosticKind::OctalEscapeOutOfRange);
      }
      for (const std::string Escape : {"\\uD800", "\\uDFFF", "\\U0000D800", "\\U00110000", "\\UFFFFFFFF"})
      {
        expectError(Quote + Escape + Quote, DiagnosticKind::InvalidUnicodeScalar);
      }
    }
  }

  // Formal names, control aliases and algorithmic names accept ASCII case folding only.
  TEST_F(TokenizerTest, NamedUnicodeEscapes)
  {
    struct Case
    {
        const char *Name;
        char32_t Value;
    };
    const Case Cases[] = {
        {"LATIN CAPITAL LETTER A", U'A'},
        {"latin capital letter a", U'A'},
        {"sNoWmAn", U'\u2603'},
        {"NUL", U'\0'},
        {"NULL", U'\0'},
        {"LINE FEED", U'\n'},
        {"LF", U'\n'},
        {"BYTE ORDER MARK", U'\uFEFF'},
        {"BELL", U'\U0001F514'},
        {"BEL", U'\a'},
        {"HANGUL SYLLABLE GA", U'\uAC00'},
        {"CJK UNIFIED IDEOGRAPH-4E00", U'\u4E00'},
        {"CJK UNIFIED IDEOGRAPH-2EBF0", U'\U0002EBF0'},
        {"IDEOGRAPHIC DESCRIPTION CHARACTER HORIZONTAL REFLECTION", U'\u2FFE'},
        {"IDEOGRAPHIC DESCRIPTION CHARACTER SUBTRACTION", U'\u31EF'},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      const auto Buffer = lex("'\\N{" + std::string(Entry.Name) + "}'");
      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::CharLiteral);
      EXPECT_EQ(std::get<CharInfo>(Buffer.tokens()[0].Payload).Value, Entry.Value);
    }
  }

  // Names and aliases preserve exact spacing and hyphens and never recognize named sequences.
  TEST_F(TokenizerTest, NamedEscapeSyntaxAndExactSeparators)
  {
    for (const std::string Escape : {"\\N", "\\NA", "\\N{}", "\\N{", "\\N{LATIN CAPITAL LETTER A"})
    {
      expectError("\"" + Escape + "\"", DiagnosticKind::InvalidNamedEscape);
    }
    for (const std::string Name : {"UNKNOWN CHARACTER", "KEYCAP DIGIT ONE", " LATIN CAPITAL LETTER A", "LATIN  CAPITAL LETTER A", "LATIN CAPITAL LETTER A ", "LATIN-CAPITAL LETTER A", "CJK UNIFIED IDEOGRAPH 4E00", "CJK UNIFIED IDEOGRAPH-04E00"})
    {
      expectError("\"\\N{" + Name + "}\"", DiagnosticKind::UnknownUnicodeName);
    }
    expectError("\"\\N{" + utf8(0x4E2D) + "}\"", DiagnosticKind::UnknownUnicodeName);
  }
} // namespace ink::tokenizer::test
