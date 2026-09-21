#include "parser_test_support.h"
#include "ink/tokenizer/unicode.h"
#include <iterator>
#include <random>

namespace ink::parser::test
{
  namespace
  {
    struct SourceRegion
    {
        const char *Name;
        const char *Prefix;
        const char *Suffix;
    };

    constexpr SourceRegion Regions[] = {
        {"module", "", " var kept = 42;"},
        {"after declaration", "var before = 1; ", " var kept = 42;"},
        {"identifier", "var before", " = 1; var kept = 42;"},
        {"string", "var text = \"", "\"; var kept = 42;"},
        {"raw string", "var text = r\"", "\"; var kept = 42;"},
        {"uppercase raw string", "var text = R\"", "\"; var kept = 42;"},
        {"triple string", "var text = \"\"\"", "\"\"\"; var kept = 42;"},
        {"raw triple string", "var text = r\"\"\"", "\"\"\"; var kept = 42;"},
        {"character", "var text = '", "'; var kept = 42;"},
        {"raw character", "var text = r'", "'; var kept = 42;"},
        {"line comment", "var before = 1; //", "\nvar kept = 42;"},
        {"block comment", "var before = 1; /*", "*/ var kept = 42;"},
        {"multiline prefix", "var \u53D8\u91CF = 1;\r\n//", "\r\nvar kept = 42;"},
    };

    std::string encodeScalar(char32_t Scalar)
    {
      std::string Result;
      tokenizer::unicode::appendUtf8(Result, Scalar);
      return Result;
    }
  } // namespace

  class ParserUnicodeTest : public ParserTest
  {
    protected:
      void expectGlobalRejection(const std::string &Source, core::DiagnosticKind Kind, std::size_t Offset, std::size_t Length)
      {
        const auto Result = read(Source);
        EXPECT_EQ(Result.Status, ParseStatus::Completed);
        EXPECT_FALSE(Result.succeeded());
        EXPECT_FALSE(Result.HasSyntaxErrors);
        EXPECT_FALSE(Result.Unit->input().lexedFile().succeeded());
        EXPECT_TRUE(Result.Unit->input().lexedFile().tokens().empty());
        EXPECT_TRUE(Result.Unit->root()->statements().empty());
        EXPECT_TRUE(Result.Unit->recoveryInfo().Entries.empty());
        EXPECT_EQ(Result.Unit->input().lexedFile().source(), Source);
        ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
        EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Kind);
        EXPECT_EQ(Diagnostics.diagnostics()[0].Span, SourceRange::fromByteOffsets(Offset, Offset + Length));
      }
  };

  // Invalid UTF-8 in every source region stops tokenization before AST construction and produces one exact lexical diagnostic.
  TEST_F(ParserUnicodeTest, MalformedUtf8InEveryRegion)
  {
    struct Case
    {
        const char *Name;
        const char *Bytes;
        std::size_t ErrorLength;
    };
    const Case Cases[] = {
        {"lone continuation low", "\x80", 1},
        {"lone continuation high", "\xBF", 1},
        {"overlong NUL", "\xC0\x80", 2},
        {"overlong slash", "\xC0\xAF", 2},
        {"overlong ASCII", "\xC1\xBF", 2},
        {"truncated two bytes", "\xC2", 1},
        {"bad second byte", "\xC2!", 1},
        {"overlong three bytes", "\xE0\x80\x80", 3},
        {"below three byte minimum", "\xE0\x9F\xBF", 3},
        {"truncated three byte lead", "\xE2", 1},
        {"truncated three byte tail", "\xE2\x82", 2},
        {"bad third byte", "\xE2\x82!", 2},
        {"high surrogate", "\xED\xA0\x80", 3},
        {"low surrogate", "\xED\xB0\x80", 3},
        {"last surrogate", "\xED\xBF\xBF", 3},
        {"CESU-8 surrogate pair", "\xED\xA0\xBD\xED\xB8\x80", 3},
        {"overlong four bytes", "\xF0\x80\x80\x80", 4},
        {"below four byte minimum", "\xF0\x8F\xBF\xBF", 4},
        {"truncated four byte lead", "\xF0", 1},
        {"truncated four byte middle", "\xF0\x9F", 2},
        {"truncated four byte tail", "\xF0\x9F\x98", 3},
        {"bad fourth byte", "\xF0\x9F\x98!", 3},
        {"above scalar maximum", "\xF4\x90\x80\x80", 4},
        {"invalid four byte lead", "\xF5\x80\x80\x80", 4},
        {"largest invalid four byte lead", "\xF7\xBF\xBF\xBF", 4},
        {"obsolete five byte sequence", "\xF8\x88\x80\x80\x80", 1},
        {"obsolete six byte sequence", "\xFC\x84\x80\x80\x80\x80", 1},
        {"forbidden FE", "\xFE", 1},
        {"forbidden FF", "\xFF", 1},
    };
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Name);
      for (const auto &Region : Regions)
      {
        SCOPED_TRACE(Region.Name);
        const std::string Prefix(Region.Prefix);
        expectGlobalRejection(Prefix + Case.Bytes + Region.Suffix, core::DiagnosticKind::InvalidUtf8, Prefix.size(), Case.ErrorLength);
      }
    }
  }

  // All non-ASCII single bytes are invalid at EOF, including every continuation and unfinished multibyte lead.
  TEST_F(ParserUnicodeTest, EveryHighByteAtEndOfFile)
  {
    for (unsigned Byte = 0x80; Byte <= 0xFF; ++Byte)
    {
      SCOPED_TRACE(Byte);
      for (const auto *Prefix : {"", "var kept = 42; ", "var text = \"", "//", "/*"})
      {
        const std::string SourcePrefix(Prefix);
        expectGlobalRejection(SourcePrefix + static_cast<char>(Byte), core::DiagnosticKind::InvalidUtf8, SourcePrefix.size(), 1);
      }
    }
  }

  // Every forbidden continuation byte in two-, three- and four-byte encodings is rejected at the lead byte with a bounded range.
  TEST_F(ParserUnicodeTest, ExhaustiveInvalidContinuationBytes)
  {
    for (const std::string Valid : {"\xC2\xA2", "\xE2\x82\xAC", "\xF0\x9F\x98\x80", "\xF4\x8F\xBF\xBF"})
    {
      for (std::size_t Index = 1; Index < Valid.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        for (unsigned Byte = 0; Byte <= 0xFF; ++Byte)
        {
          if (Byte >= 0x80 && Byte <= 0xBF)
          {
            continue;
          }
          SCOPED_TRACE(Byte);
          std::string Mutated = Valid;
          Mutated[Index] = static_cast<char>(Byte);
          const std::string Prefix = "var kept = 42; /*";
          expectGlobalRejection(Prefix + Mutated + "*/", core::DiagnosticKind::InvalidUtf8, Prefix.size(), Index);
        }
      }
    }
  }

  // UTF-8 scalar boundary encodings are accepted intact but every incomplete prefix fails safely at physical EOF.
  TEST_F(ParserUnicodeTest, ScalarBoundariesAndTruncation)
  {
    const std::string Encodings[] = {
        "\xC2\x80",
        "\xDF\xBF",
        "\xE0\xA0\x80",
        "\xED\x9F\xBF",
        "\xEE\x80\x80",
        "\xEF\xBF\xBF",
        "\xF0\x90\x80\x80",
        "\xF4\x8F\xBF\xBF",
    };
    for (const auto &Encoding : Encodings)
    {
      const auto Valid = read("var text = \"" + Encoding + "\"; var kept = 42;");
      ASSERT_TRUE(Valid.succeeded());
      const auto *Literal = cast<LiteralExpr>(cast<VarDecl>(declaration(Valid))->initializer());
      EXPECT_EQ(std::get<tokenizer::StringInfo>(Valid.Unit->input().token(Literal->token()).Payload).Decoded, Encoding);
      for (std::size_t Length = 1; Length < Encoding.size(); ++Length)
      {
        SCOPED_TRACE(Length);
        const std::string Prefix = "var kept = 42; //";
        expectGlobalRejection(Prefix + Encoding.substr(0, Length), core::DiagnosticKind::InvalidUtf8, Prefix.size(), Length);
      }
    }
  }

  // Overlong encodings of ASCII syntax cannot smuggle delimiters, newlines or NUL into any lexical region.
  TEST_F(ParserUnicodeTest, OverlongSyntaxSmuggling)
  {
    constexpr char Syntax[] = "\0\n\r\"'/*;{}()[]\\";
    for (const unsigned char Character : std::string_view(Syntax, sizeof(Syntax) - 1))
    {
      SCOPED_TRACE(static_cast<unsigned>(Character));
      const std::string Encodings[] = {
          std::string{static_cast<char>(0xC0 | (Character >> 6)), static_cast<char>(0x80 | (Character & 0x3F))},
          std::string{static_cast<char>(0xE0), static_cast<char>(0x80 | (Character >> 6)), static_cast<char>(0x80 | (Character & 0x3F))},
          std::string{static_cast<char>(0xF0), static_cast<char>(0x80), static_cast<char>(0x80 | (Character >> 6)), static_cast<char>(0x80 | (Character & 0x3F))},
      };
      for (const auto &Encoding : Encodings)
      {
        for (const auto &Region : Regions)
        {
          SCOPED_TRACE(Region.Name);
          const std::string Prefix(Region.Prefix);
          expectGlobalRejection(Prefix + Encoding + Region.Suffix, core::DiagnosticKind::InvalidUtf8, Prefix.size(), Encoding.size());
        }
      }
    }
  }

  // Literal NUL is rejected even after valid declarations and inside comments, raw strings and character literals.
  TEST_F(ParserUnicodeTest, EmbeddedNullInEveryRegion)
  {
    for (const auto &Region : Regions)
    {
      SCOPED_TRACE(Region.Name);
      const std::string Prefix(Region.Prefix);
      expectGlobalRejection(Prefix + '\0' + Region.Suffix, core::DiagnosticKind::NullInSource, Prefix.size(), 1);
    }
    expectGlobalRejection(std::string(4096, '\0'), core::DiagnosticKind::NullInSource, 0, 1);
    EXPECT_TRUE(read("var kept = 42;").succeeded());
  }

  // Leading and duplicate UTF-8 BOMs and UTF-16/UTF-32 byte streams are never silently reinterpreted as Ink source.
  TEST_F(ParserUnicodeTest, BomAndForeignEncodings)
  {
    expectGlobalRejection("\xEF\xBB\xBFvar kept = 42;", core::DiagnosticKind::UnexpectedBom, 0, 3);
    expectGlobalRejection("\xEF\xBB\xBF\xEF\xBB\xBF", core::DiagnosticKind::UnexpectedBom, 0, 3);
    for (const std::string Source : {std::string("\xFF\xFEv\0", 4), std::string("\xFE\xFF\0v", 4), std::string("\xFF\xFE\0\0v\0\0\0", 8)})
    {
      expectGlobalRejection(Source, core::DiagnosticKind::InvalidUtf8, 0, 1);
    }
    expectGlobalRejection(std::string("\0\0\xFE\xFF\0\0\0v", 8), core::DiagnosticKind::NullInSource, 0, 1);
    expectGlobalRejection(std::string("v\0a\0r\0", 6), core::DiagnosticKind::NullInSource, 1, 1);
    expectGlobalRejection(std::string("\0v\0a\0r", 6), core::DiagnosticKind::NullInSource, 0, 1);
  }

  // Bidi controls, invisible characters, Unicode spaces, noncharacters and confusable punctuation cannot act as syntax or whitespace.
  TEST_F(ParserUnicodeTest, SuspiciousScalarsOutsideContent)
  {
    constexpr char32_t Scalars[] = {
        0x01,
        0x08,
        0x0B,
        0x0C,
        0x1B,
        0x7F,
        0x85,
        0xA0,
        0xAD,
        0x301,
        0x34F,
        0x61C,
        0x1680,
        0x2000,
        0x2001,
        0x2002,
        0x2003,
        0x2004,
        0x2005,
        0x2006,
        0x2007,
        0x2008,
        0x2009,
        0x200A,
        0x200B,
        0x200C,
        0x200D,
        0x200E,
        0x200F,
        0x2028,
        0x2029,
        0x202A,
        0x202B,
        0x202C,
        0x202D,
        0x202E,
        0x202F,
        0x205F,
        0x2060,
        0x2066,
        0x2067,
        0x2068,
        0x2069,
        0x2212,
        0x3000,
        0xFDD0,
        0xFEFF,
        0xFF08,
        0xFF09,
        0xFF1B,
        0xFF1D,
        0xFF5B,
        0xFF5D,
        0xFFFD,
        0xFFFF,
        0x1F600,
        0x10FFFF,
    };
    for (const char32_t Scalar : Scalars)
    {
      SCOPED_TRACE(static_cast<unsigned>(Scalar));
      const std::string Prefix = "var before = 1; ";
      const std::string Bytes = encodeScalar(Scalar);
      const auto Result = read(Prefix + Bytes + " var after = 2;");
      EXPECT_EQ(Result.Status, ParseStatus::Completed);
      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.HasSyntaxErrors);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::InvalidCharacter);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, SourceRange::fromByteOffsets(Prefix.size(), Prefix.size() + Bytes.size()));
      ASSERT_EQ(Result.Unit->root()->statements().size(), 1U);
      EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Result))->binding())->name().Text, "before");
    }
  }

  // Legal invisible and noncharacter scalars remain literal/comment content with decoded values and the following declaration preserved.
  TEST_F(ParserUnicodeTest, SuspiciousScalarsInsideContent)
  {
    for (const char32_t Scalar : {0x01, 0x7F, 0x85, 0xA0, 0x61C, 0x200B, 0x200C, 0x200D, 0x200E, 0x200F, 0x2028, 0x2029, 0x202A, 0x202B, 0x202C, 0x202D, 0x202E, 0x2066, 0x2067, 0x2068, 0x2069, 0xFDD0, 0xFEFF, 0xFFFF, 0x10FFFF})
    {
      SCOPED_TRACE(static_cast<unsigned>(Scalar));
      const std::string Bytes = encodeScalar(Scalar);
      for (const auto *Quote : {"\"", "r\"", "R\"", "\"\"\"", "r\"\"\"", "'", "r'"})
      {
        const std::string Open(Quote);
        const std::string Close = Open[0] == 'r' || Open[0] == 'R' ? Open.substr(1) : Open;
        const auto Result = read("var text = " + Open + Bytes + Close + "; var kept = 42;");
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
        const auto *Literal = cast<LiteralExpr>(cast<VarDecl>(declaration(Result))->initializer());
        const auto &Payload = Result.Unit->input().token(Literal->token()).Payload;
        if (Literal->literalKind() == TokenKind::StringLiteral)
        {
          EXPECT_EQ(std::get<tokenizer::StringInfo>(Payload).Decoded, Bytes);
        }
        else
        {
          EXPECT_EQ(std::get<tokenizer::CharInfo>(Payload).Value, Scalar);
        }
      }
      EXPECT_TRUE(read("var before = 1; /*" + Bytes + "*/ var kept = 42;").succeeded());
      const auto Comment = read("var before = 1; //" + Bytes + " var hidden = 0;\nvar kept = 42;");
      ASSERT_TRUE(Comment.succeeded());
      ASSERT_EQ(Comment.Unit->root()->statements().size(), 2U);
      EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Comment, 1))->binding())->name().Text, "kept");
    }
  }

  // Bidi text and delimiter lookalikes inside strings and comments cannot introduce hidden declarations into the AST.
  TEST_F(ParserUnicodeTest, BidiAndConfusableContentCannotInjectSyntax)
  {
    const std::string Content = "\u202E } var injected = 0; { \u2066\uFF0F\uFF0A\u2069\u202C";
    for (const std::string Source : {"var text = \"" + Content + "\"; var kept = 42;", "var before = 1; /*" + Content + "*/ var kept = 42;", "var before = 1; //" + Content + "\nvar kept = 42;"})
    {
      const auto Result = read(Source);
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Result, 1))->binding())->name().Text, "kept");
    }
  }

  // Confusable identifier spellings remain identifiers; NFC normalization never applies compatibility folding or keyword spoofing.
  TEST_F(ParserUnicodeTest, ConfusableIdentifiersAndNfc)
  {
    struct Case
    {
        const char *Raw;
        const char *Normalized;
    };
    const Case Cases[] = {
        {"v\u0430r", "v\u0430r"},
        {"\uFF56\uFF41\uFF52", "\uFF56\uFF41\uFF52"},
        {"\uFB00", "\uFB00"},
        {"e\u0301", "\u00E9"},
        {"\u00E9", "\u00E9"},
        {"\u212A", "K"},
        {"\u1100\u1161", "\uAC00"},
        {"\U0002EBF0", "\U0002EBF0"},
    };
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Raw);
      const std::string Source = std::string("var ") + Case.Raw + " = 1; " + Case.Raw + ";";
      const auto Result = read(Source);
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      const auto Binding = cast<NameBindingPattern>(cast<VarDecl>(declaration(Result))->binding())->name();
      const auto Reference = cast<NameExpr>(expression(Result, 1))->name();
      EXPECT_EQ(Binding.Text, Case.Raw);
      EXPECT_EQ(Reference.Text, Case.Raw);
      for (const auto &Token : Result.Unit->input().lexedFile().tokens())
      {
        if (Token.Kind == TokenKind::Identifier)
        {
          EXPECT_EQ(std::get<tokenizer::IdentifierInfo>(Token.Payload).Name, Case.Normalized);
          EXPECT_EQ(Token.Span.getEnd().getByteOffset() - Token.Span.getBegin().getByteOffset(), std::string_view(Case.Raw).size());
        }
      }
    }
  }

  // Recovery offsets count original UTF-8 bytes, including decomposed identifiers and CRLF, without corrupting the next Unicode name.
  TEST_F(ParserUnicodeTest, RecoveryUsesOriginalByteOffsets)
  {
    const std::string Source = "var e\u0301 = 1\r\nvar \u4FDD\u7559 = f(\"\U0001F600\", , 3);";
    const auto Result = read(Source);
    ASSERT_EQ(Result.Status, ParseStatus::Completed);
    ASSERT_TRUE(Result.HasSyntaxErrors);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    const std::size_t DeclarationOffset = Source.find("var", 1);
    const std::size_t MissingOffset = Source.find(", ,") + 2;
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, SourceRange::fromByteOffsets(DeclarationOffset, DeclarationOffset));
    EXPECT_EQ(Diagnostics.diagnostics()[1].Span, SourceRange::fromByteOffsets(MissingOffset, MissingOffset));
    EXPECT_EQ(Result.Unit->input().lexedFile().lineNumber(DeclarationOffset), 2U);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    const auto *Variable = cast<VarDecl>(declaration(Result, 1));
    EXPECT_EQ(cast<NameBindingPattern>(Variable->binding())->name().Text, "\u4FDD\u7559");
    ASSERT_EQ(cast<CallExpr>(Variable->initializer())->arguments().size(), 3U);
    EXPECT_TRUE(isa<MissingExpr>(cast<CallExpr>(Variable->initializer())->arguments()[1].value()));
  }

  // Long combining-mark sequences normalize and preserve byte ranges without breaking parsing or the following declaration.
  TEST_F(ParserUnicodeTest, LongCombiningIdentifiers)
  {
    std::string Name = "a";
    for (unsigned Index = 0; Index < 1024; ++Index)
    {
      Name += "\u0315\u0300\u0327";
    }
    const auto Result = read("var " + Name + " = 1; var kept = 42;");
    ASSERT_TRUE(Result.succeeded());
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    const auto Binding = cast<NameBindingPattern>(cast<VarDecl>(declaration(Result))->binding())->name();
    EXPECT_EQ(Binding.Text, Name);
    EXPECT_EQ(Binding.Range, SourceRange::fromByteOffsets(4, 4 + Name.size()));
    EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Result, 1))->binding())->name().Text, "kept");
  }

  // Invalid Unicode escapes remain tokenizer errors in normal strings, triple strings and character literals, with no parser cascade.
  TEST_F(ParserUnicodeTest, MaliciousUnicodeEscapes)
  {
    struct Case
    {
        const char *Text;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"\\uD800", core::DiagnosticKind::InvalidUnicodeScalar},
        {"\\uDFFF", core::DiagnosticKind::InvalidUnicodeScalar},
        {"\\U0000D800", core::DiagnosticKind::InvalidUnicodeScalar},
        {"\\U00110000", core::DiagnosticKind::InvalidUnicodeScalar},
        {"\\UFFFFFFFF", core::DiagnosticKind::InvalidUnicodeScalar},
        {"\\uD83D\\uDE00", core::DiagnosticKind::InvalidUnicodeScalar},
        {"\\u", core::DiagnosticKind::InvalidEscape},
        {"\\u123", core::DiagnosticKind::InvalidEscape},
        {"\\U0001F60", core::DiagnosticKind::InvalidEscape},
        {"\\u\uFF11\uFF12\uFF13\uFF14", core::DiagnosticKind::InvalidEscape},
        {"\\x\u0661\u0662", core::DiagnosticKind::InvalidEscape},
        {"\\N{}", core::DiagnosticKind::InvalidNamedEscape},
        {"\\N{LATIN \u202ECAPITAL LETTER A}", core::DiagnosticKind::UnknownUnicodeName},
        {"\\N{CJK UNIFIED IDEOGRAPH-00004E00}", core::DiagnosticKind::UnknownUnicodeName},
    };
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Text);
      for (const std::string Quote : {"\"", "\"\"\"", "'"})
      {
        const auto Result = read("var text = " + Quote + Case.Text + Quote + "; var kept = 42;");
        EXPECT_EQ(Result.Status, ParseStatus::Completed);
        EXPECT_FALSE(Result.succeeded());
        EXPECT_FALSE(Result.HasSyntaxErrors);
        ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
        EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Case.Kind);
      }
    }
  }

  // Escaped NUL, delimiters and backslashes are literal values and are never reparsed as source or recursively unescaped.
  TEST_F(ParserUnicodeTest, EscapedSyntaxAndNullAreLiteralData)
  {
    const auto Result = read(R"ink(var text = "\0\u003B\u007D\u0022\u000A\\u0041"; var kept = 42;)ink");
    ASSERT_TRUE(Result.succeeded());
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    const auto *Literal = cast<LiteralExpr>(cast<VarDecl>(declaration(Result))->initializer());
    const auto &Decoded = std::get<tokenizer::StringInfo>(Result.Unit->input().token(Literal->token()).Payload).Decoded;
    EXPECT_EQ(Decoded, std::string(1, '\0') + ";}\"\n\\u0041");
    EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Result, 1))->binding())->name().Text, "kept");
  }

  // A lexical failure after a recoverable syntax error retains the valid prefix and reports each error exactly once.
  TEST_F(ParserUnicodeTest, LexicalFailureAfterSyntaxRecovery)
  {
    const auto Result = read("var broken = ; var kept = 42; \u200B");
    EXPECT_EQ(Result.Status, ParseStatus::Completed);
    EXPECT_FALSE(Result.succeeded());
    EXPECT_TRUE(Result.HasSyntaxErrors);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::InvalidCharacter);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::ParserExpectedExpression);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Result, 1))->binding())->name().Text, "kept");
  }

  // Deterministic byte replacement and deletion at every byte of Unicode-rich syntax terminates without invalid ASTs or diagnostics.
  TEST_F(ParserUnicodeTest, UnicodeByteMutationCorpus)
  {
    const std::string Source = "[\u6807\u7B7E] func \u51FD\u6570(e\u0301: T): T { var \u503C = \"\U0001F600\u202E\"; /*\u2066*/ return e\u0301; }";
    ASSERT_TRUE(read(Source).succeeded());
    ParseLimits Limits;
    Limits.MaxWork = 50000;
    Limits.MaxNestingDepth = 48;
    for (std::size_t Offset = 0; Offset < Source.size(); ++Offset)
    {
      SCOPED_TRACE(Offset);
      EXPECT_EQ(read(Source.substr(0, Offset), Limits).Status, ParseStatus::Completed);
      EXPECT_EQ(read(Source.substr(0, Offset) + Source.substr(Offset + 1), Limits).Status, ParseStatus::Completed);
      for (const unsigned Byte : {0x00, 0x22, 0x27, 0x2F, 0x3B, 0x7D, 0x80, 0xBF, 0xC0, 0xC2, 0xED, 0xF0, 0xF4, 0xF5, 0xFF})
      {
        SCOPED_TRACE(Byte);
        std::string Mutated = Source;
        Mutated[Offset] = static_cast<char>(Byte);
        EXPECT_EQ(read(std::move(Mutated), Limits).Status, ParseStatus::Completed);
      }
    }
  }

  // Fixed-seed binary noise and mixtures of valid non-ASCII scalars exercise the complete tokenizer/parser boundary reproducibly.
  TEST_F(ParserUnicodeTest, DeterministicBinaryAndUnicodeInputs)
  {
    std::mt19937 Random(0x554E4943);
    constexpr char32_t Scalars[] = {
        0x00,
        0x41,
        0x5F,
        0x3B,
        0x85,
        0xA0,
        0xE9,
        0x301,
        0x3B1,
        0x200B,
        0x2028,
        0x202E,
        0x2066,
        0x4E2D,
        0xFDD0,
        0xFEFF,
        0xFF1B,
        0xFFFF,
        0x1F600,
        0x2EBF0,
        0x10FFFF,
    };
    ParseLimits Limits;
    Limits.MaxWork = 50000;
    Limits.MaxNestingDepth = 48;
    for (unsigned Iteration = 0; Iteration < 512; ++Iteration)
    {
      SCOPED_TRACE(Iteration);
      std::string Bytes;
      std::string Unicode;
      for (unsigned Index = 0, Count = 1 + Random() % 64; Index < Count; ++Index)
      {
        Bytes.push_back(static_cast<char>(Random() & 0xFF));
        Unicode += encodeScalar(Scalars[Random() % std::size(Scalars)]);
      }
      const auto &Region = Regions[Iteration % std::size(Regions)];
      EXPECT_EQ(read(std::string(Region.Prefix) + Bytes + Region.Suffix, Limits).Status, ParseStatus::Completed);
      EXPECT_EQ(read(std::string(Region.Prefix) + Unicode + Region.Suffix, Limits).Status, ParseStatus::Completed);
    }
  }
} // namespace ink::parser::test
