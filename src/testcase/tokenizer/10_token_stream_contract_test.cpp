#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;
    using core::SourceRange;

    static_assert(!std::is_default_constructible_v<TokenizedBuffer>);

    void expectPartition(const TokenizedBuffer &File)
    {
      ASSERT_FALSE(File.tokens().empty());
      std::size_t Cursor = 0;
      std::size_t EofCount = 0;
      std::string Rebuilt;
      for (std::size_t Index = 0; Index < File.tokens().size(); ++Index)
      {
        const Token &CurrentToken = File.tokens()[Index];
        if (CurrentToken.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(Index, File.tokens().size() - 1);
          EXPECT_EQ(CurrentToken.Span, (SourceRange{File.source().size(), File.source().size()}));
          EXPECT_TRUE(File.raw(CurrentToken).empty());
          continue;
        }
        EXPECT_LT(CurrentToken.Span.Start, CurrentToken.Span.End);
        EXPECT_EQ(CurrentToken.Span.Start, Cursor);
        EXPECT_EQ(CurrentToken.Span.size(), File.raw(CurrentToken).size());
        Rebuilt.append(File.raw(CurrentToken).data(), File.raw(CurrentToken).size());
        Cursor = CurrentToken.Span.End;
      }
      EXPECT_EQ(Cursor, File.source().size());
      EXPECT_EQ(Rebuilt, File.source());
      EXPECT_EQ(EofCount, 1u);
    }

    void expectToken(const TokenizedBuffer &File, std::size_t Index, TokenKind Kind, std::string_view Raw, SourceRange Span)
    {
      ASSERT_LT(Index, File.tokens().size());
      EXPECT_EQ(File.tokens()[Index].Kind, Kind);
      EXPECT_EQ(std::string(File.raw(File.tokens()[Index])), std::string(Raw));
      EXPECT_EQ(File.tokens()[Index].Span, Span);
    }

    bool hasDiagnostic(const TokenizedBuffer &File, DiagnosticKind Kind)
    {
      return std::any_of(File.diagnostics().begin(), File.diagnostics().end(), [Kind](const Diagnostic &CurrentDiagnostic)
                         {
                           return CurrentDiagnostic.Kind == Kind;
                         });
    }

    std::vector<const Token *> syntaxTokens(const TokenizedBuffer &File)
    {
      std::vector<const Token *> Result;
      for (const Token &CurrentToken : File.tokens())
      {
        if (!CurrentToken.isTrivia() && CurrentToken.Kind != TokenKind::EndOfFile)
        {
          Result.push_back(&CurrentToken);
        }
      }
      return Result;
    }

    // Verifies the sentinel-only token stream produced for an empty source buffer.
    TEST(TokenStreamContractTest, EmptySourceHasExactlyOneEofToken)
    {
      const TokenizedBuffer File = tokenize("");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 1u);
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::EndOfFile);
      EXPECT_EQ(File.tokens()[0].Span, (SourceRange{0, 0}));
      EXPECT_TRUE(File.raw(File.tokens()[0]).empty());
      EXPECT_FALSE(File.tokens()[0].isTrivia());
      EXPECT_FALSE(File.tokens()[0].isError());
      expectPartition(File);
    }

    // Verifies that a mixed source is represented as an exact contiguous sequence of byte spans.
    TEST(TokenStreamContractTest, MixedSourceIsAnExactContiguousBytePartition)
    {
      const std::string Source = "\xEF\xBB\xBFlet x = \"v\";\r\n//tail";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 12u);
      expectToken(File, 0, TokenKind::Utf8Bom, "\xEF\xBB\xBF", SourceRange{0, 3});
      expectToken(File, 1, TokenKind::Keyword, "let", SourceRange{3, 6});
      expectToken(File, 2, TokenKind::SpacesAndTabs, " ", SourceRange{6, 7});
      expectToken(File, 3, TokenKind::Identifier, "x", SourceRange{7, 8});
      expectToken(File, 4, TokenKind::SpacesAndTabs, " ", SourceRange{8, 9});
      expectToken(File, 5, TokenKind::Symbol, "=", SourceRange{9, 10});
      expectToken(File, 6, TokenKind::SpacesAndTabs, " ", SourceRange{10, 11});
      expectToken(File, 7, TokenKind::StringLiteral, "\"v\"", SourceRange{11, 14});
      expectToken(File, 8, TokenKind::Symbol, ";", SourceRange{14, 15});
      expectToken(File, 9, TokenKind::LineBreak, "\r\n", SourceRange{15, 17});
      expectToken(File, 10, TokenKind::LineComment, "//tail", SourceRange{17, 23});
      expectToken(File, 11, TokenKind::EndOfFile, "", SourceRange{23, 23});
      expectPartition(File);
    }

    // Verifies that TokenizedBuffer retains ownership of temporary source storage used by raw views.
    TEST(TokenStreamContractTest, TokenizedBufferOwnsSourceUsedByRawViews)
    {
      TokenizedBuffer File = tokenize(std::string("persistent identifier"));

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.source(), "persistent identifier");
      expectToken(File, 0, TokenKind::Identifier, "persistent", SourceRange{0, 10});
      expectToken(File, 2, TokenKind::Identifier, "identifier", SourceRange{11, 21});
      expectPartition(File);
    }

    // Verifies defensive raw-view handling for externally supplied tokens whose spans are not valid source slices.
    TEST(TokenStreamContractTest, RawViewRejectsReversedAndOutOfBoundsExternalSpans)
    {
      const TokenizedBuffer File = tokenize("abc");
      const Token Reversed{TokenKind::Identifier, SourceRange{2, 1}, {}};
      const Token PastEnd{TokenKind::Identifier, SourceRange{1, 4}, {}};

      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(File.raw(Reversed).empty());
      EXPECT_TRUE(File.raw(PastEnd).empty());
      expectPartition(File);
    }

    // Verifies line lookup for empty input, trailing logical lines, CRLF byte positions, and offsets beyond end of file.
    TEST(TokenStreamContractTest, LineLookupClampsOffsetsAndRetainsATrailingEmptyLine)
    {
      const TokenizedBuffer Empty = tokenize("");
      const TokenizedBuffer File = tokenize("a\r\nb\n");

      ASSERT_TRUE(Empty.succeeded());
      EXPECT_EQ(Empty.lineStarts(), (std::vector<std::size_t>{0}));
      EXPECT_EQ(Empty.lineNumber(std::numeric_limits<std::size_t>::max()), 1U);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 3, 5}));
      EXPECT_EQ(File.lineNumber(1), 1U);
      EXPECT_EQ(File.lineNumber(2), 1U);
      EXPECT_EQ(File.lineNumber(3), 2U);
      EXPECT_EQ(File.lineNumber(5), 3U);
      EXPECT_EQ(File.lineNumber(std::numeric_limits<std::size_t>::max()), 3U);
      expectPartition(File);
    }

    // Verifies that physical line starts remain queryable inside block comments and multiline string tokens.
    TEST(TokenStreamContractTest, LineLookupIncludesBreaksInsideOpaqueMultilineTokens)
    {
      const std::string Source = "/* first\nsecond */\n\"\"\"\nbody\n\"\"\"";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 9, 19, 23, 28}));
      EXPECT_EQ(File.lineNumber(File.tokens()[0].Span.Start), 1U);
      EXPECT_EQ(File.lineNumber(File.tokens()[0].Span.End - 1), 2U);
      EXPECT_EQ(File.lineNumber(File.tokens()[2].Span.Start), 3U);
      EXPECT_EQ(File.lineNumber(File.tokens()[2].Span.End - 1), 5U);
      expectPartition(File);
    }

    // Verifies that one Tokenizer instance can be reused and preserves its configured comment-depth limit across calls.
    TEST(TokenStreamContractTest, TokenizerInstanceIsReusableAndRetainsOptions)
    {
      const Tokenizer Scanner(TokenizerOptions{1});
      const TokenizedBuffer First = Scanner.tokenize("let");
      const TokenizedBuffer Limited = Scanner.tokenize("/* outer /* inner */ outer */");
      const TokenizedBuffer Second = Scanner.tokenize("let");

      ASSERT_TRUE(First.succeeded());
      ASSERT_FALSE(Limited.succeeded());
      ASSERT_TRUE(Second.succeeded());
      EXPECT_EQ(First.tokens(), Second.tokens());
      EXPECT_TRUE(hasDiagnostic(Limited, DiagnosticKind::BlockCommentNestingLimit));
      expectPartition(First);
      expectPartition(Limited);
      expectPartition(Second);
    }

    // Verifies exhaustive, disjoint trivia and error classification for every token kind.
    TEST(TokenStreamContractTest, TriviaAndErrorClassificationsAreOrthogonalAndExhaustive)
    {
      const std::array<TokenKind, 23> AllKinds = {
          TokenKind::Utf8Bom,
          TokenKind::SpacesAndTabs,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::BlockComment,
          TokenKind::Identifier,
          TokenKind::Keyword,
          TokenKind::BuiltinType,
          TokenKind::BoolLiteral,
          TokenKind::NullLiteral,
          TokenKind::IntegerLiteral,
          TokenKind::FloatLiteral,
          TokenKind::ScalarLiteral,
          TokenKind::StringLiteral,
          TokenKind::Symbol,
          TokenKind::InvalidEncoding,
          TokenKind::InvalidCharacter,
          TokenKind::InvalidIdentifier,
          TokenKind::InvalidNumber,
          TokenKind::InvalidScalarLiteral,
          TokenKind::InvalidStringLiteral,
          TokenKind::UnterminatedBlockComment,
          TokenKind::EndOfFile,
      };
      const std::array<TokenKind, 5> TriviaKinds = {
          TokenKind::Utf8Bom,
          TokenKind::SpacesAndTabs,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::BlockComment,
      };
      const std::array<TokenKind, 7> ErrorKinds = {
          TokenKind::InvalidEncoding,
          TokenKind::InvalidCharacter,
          TokenKind::InvalidIdentifier,
          TokenKind::InvalidNumber,
          TokenKind::InvalidScalarLiteral,
          TokenKind::InvalidStringLiteral,
          TokenKind::UnterminatedBlockComment,
      };

      for (TokenKind Kind : AllKinds)
      {
        const bool ExpectedTrivia = std::find(TriviaKinds.begin(), TriviaKinds.end(), Kind) != TriviaKinds.end();
        const bool ExpectedError = std::find(ErrorKinds.begin(), ErrorKinds.end(), Kind) != ErrorKinds.end();
        SCOPED_TRACE(tokenKindName(Kind));
        EXPECT_EQ(isTrivia(Kind), ExpectedTrivia);
        EXPECT_EQ(isError(Kind), ExpectedError);
        EXPECT_FALSE(isTrivia(Kind) && isError(Kind));
      }
    }

    // Verifies the stable public name returned for every token kind and for an out-of-range defensive value.
    TEST(TokenStreamContractTest, TokenKindNamesMatchEveryPublicEnumerationValue)
    {
      const std::array<std::pair<TokenKind, const char *>, 23> Cases = {
          std::pair{TokenKind::Utf8Bom, "Utf8Bom"},
          std::pair{TokenKind::SpacesAndTabs, "SpacesAndTabs"},
          std::pair{TokenKind::LineBreak, "LineBreak"},
          std::pair{TokenKind::LineComment, "LineComment"},
          std::pair{TokenKind::BlockComment, "BlockComment"},
          std::pair{TokenKind::Identifier, "Identifier"},
          std::pair{TokenKind::Keyword, "Keyword"},
          std::pair{TokenKind::BuiltinType, "BuiltinType"},
          std::pair{TokenKind::BoolLiteral, "BoolLiteral"},
          std::pair{TokenKind::NullLiteral, "NullLiteral"},
          std::pair{TokenKind::IntegerLiteral, "IntegerLiteral"},
          std::pair{TokenKind::FloatLiteral, "FloatLiteral"},
          std::pair{TokenKind::ScalarLiteral, "ScalarLiteral"},
          std::pair{TokenKind::StringLiteral, "StringLiteral"},
          std::pair{TokenKind::Symbol, "Symbol"},
          std::pair{TokenKind::InvalidEncoding, "InvalidEncoding"},
          std::pair{TokenKind::InvalidCharacter, "InvalidCharacter"},
          std::pair{TokenKind::InvalidIdentifier, "InvalidIdentifier"},
          std::pair{TokenKind::InvalidNumber, "InvalidNumber"},
          std::pair{TokenKind::InvalidScalarLiteral, "InvalidScalarLiteral"},
          std::pair{TokenKind::InvalidStringLiteral, "InvalidStringLiteral"},
          std::pair{TokenKind::UnterminatedBlockComment, "UnterminatedBlockComment"},
          std::pair{TokenKind::EndOfFile, "EndOfFile"},
      };

      for (const auto &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.second);
        EXPECT_STREQ(tokenKindName(TestCase.first), TestCase.second);
      }
      EXPECT_STREQ(tokenKindName(static_cast<TokenKind>(-1)), "Unknown");
    }

    // Verifies that each syntax token's typed payload agrees with its raw lexical spelling.
    TEST(TokenStreamContractTest, DerivedPayloadsMatchTheirRawTokens)
    {
      const std::string Source = "let i32 true false null 0xFFu8 1.5f32 'A' \"x\" +";
      const TokenizedBuffer File = tokenize(Source);
      const std::vector<const Token *> Tokens = syntaxTokens(File);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(Tokens.size(), 10u);
      EXPECT_EQ(Tokens[0]->Kind, TokenKind::Keyword);
      EXPECT_EQ(std::get<KeywordKind>(Tokens[0]->Payload), KeywordKind::Let);
      EXPECT_EQ(Tokens[1]->Kind, TokenKind::BuiltinType);
      EXPECT_EQ(std::get<BuiltinTypeKind>(Tokens[1]->Payload), BuiltinTypeKind::I32);
      EXPECT_EQ(Tokens[2]->Kind, TokenKind::BoolLiteral);
      EXPECT_TRUE(std::get<bool>(Tokens[2]->Payload));
      EXPECT_EQ(Tokens[3]->Kind, TokenKind::BoolLiteral);
      EXPECT_FALSE(std::get<bool>(Tokens[3]->Payload));
      EXPECT_EQ(Tokens[4]->Kind, TokenKind::NullLiteral);
      EXPECT_TRUE(std::holds_alternative<std::monostate>(Tokens[4]->Payload));
      EXPECT_EQ(Tokens[5]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(std::get<NumericInfo>(Tokens[5]->Payload).Base, 16u);
      EXPECT_EQ(std::get<NumericInfo>(Tokens[5]->Payload).Suffix, NumericSuffix::U8);
      EXPECT_EQ(Tokens[6]->Kind, TokenKind::FloatLiteral);
      EXPECT_EQ(std::get<NumericInfo>(Tokens[6]->Payload).Base, 10u);
      EXPECT_EQ(std::get<NumericInfo>(Tokens[6]->Payload).Suffix, NumericSuffix::F32);
      EXPECT_EQ(Tokens[7]->Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(std::get<char32_t>(Tokens[7]->Payload), U'A');
      EXPECT_EQ(Tokens[8]->Kind, TokenKind::StringLiteral);
      EXPECT_EQ(std::get<StringInfo>(Tokens[8]->Payload).Mode, StringMode::EscapedSingleLine);
      EXPECT_EQ(std::get<StringInfo>(Tokens[8]->Payload).Decoded, "x");
      EXPECT_EQ(Tokens[9]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[9]->Payload), '+');
      expectPartition(File);
    }

    // Verifies that every error-token category causes failure and contributes a diagnostic.
    TEST(TokenStreamContractTest, EveryLexicalErrorKindMakesTheResultFail)
    {
      std::string InvalidUtf8(1, static_cast<char>(0x80));
      const std::string DecomposedIdentifier = "cafe\xCC\x81";
      const std::vector<std::pair<std::string, TokenKind>> Cases = {
          {InvalidUtf8, TokenKind::InvalidEncoding},
          {"?", TokenKind::InvalidCharacter},
          {DecomposedIdentifier, TokenKind::InvalidIdentifier},
          {"0x", TokenKind::InvalidNumber},
          {"''", TokenKind::InvalidScalarLiteral},
          {"\"", TokenKind::InvalidStringLiteral},
          {"/*", TokenKind::UnterminatedBlockComment},
      };
      for (const auto &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.first);
        const TokenizedBuffer File = tokenize(TestCase.first);
        ASSERT_FALSE(File.succeeded());
        ASSERT_FALSE(File.diagnostics().empty());
        ASSERT_GE(File.tokens().size(), 2u);
        EXPECT_EQ(File.tokens()[0].Kind, TestCase.second);
        EXPECT_TRUE(File.tokens()[0].isError());
        EXPECT_FALSE(File.tokens()[0].isTrivia());
        expectPartition(File);
      }
    }

    // Verifies recovery from an invalid UTF-8 byte without consuming a following keyword.
    TEST(TokenStreamContractTest, InvalidUtf8DoesNotConsumeFollowingKeyword)
    {
      std::string Source(1, static_cast<char>(0x80));
      Source += "let";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 3u);
      expectToken(File, 0, TokenKind::InvalidEncoding, std::string(1, static_cast<char>(0x80)), SourceRange{0, 1});
      expectToken(File, 1, TokenKind::Keyword, "let", SourceRange{1, 4});
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
      expectPartition(File);
    }

    // Verifies recovery from a truncated UTF-8 sequence without consuming a following identifier.
    TEST(TokenStreamContractTest, TruncatedUtf8SequenceDoesNotConsumeFollowingIdentifier)
    {
      std::string Source;
      Source.push_back(static_cast<char>(0xE2));
      Source.push_back(static_cast<char>(0x82));
      Source += "x";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 3u);
      expectToken(File, 0, TokenKind::InvalidEncoding, Source.substr(0, 2), SourceRange{0, 2});
      expectToken(File, 1, TokenKind::Identifier, "x", SourceRange{2, 3});
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
      expectPartition(File);
    }

    // Verifies that an invalid character between identifiers does not prevent scanning the suffix.
    TEST(TokenStreamContractTest, InvalidCharacterBetweenIdentifiersDoesNotPreventRecovery)
    {
      const TokenizedBuffer File = tokenize("before?after");

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4u);
      expectToken(File, 0, TokenKind::Identifier, "before", SourceRange{0, 6});
      expectToken(File, 1, TokenKind::InvalidCharacter, "?", SourceRange{6, 7});
      expectToken(File, 2, TokenKind::Identifier, "after", SourceRange{7, 12});
      expectPartition(File);
    }

    // Verifies that repeated invalid punctuation always advances and emits nonempty error tokens.
    TEST(TokenStreamContractTest, ScannerAlwaysAdvancesAcrossRepeatedInvalidInput)
    {
      const std::string Source = "????$$$$####````\\\\";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_GE(File.tokens().size(), 2u);
      ASSERT_LE(File.tokens().size(), Source.size() + 1);
      for (const Token &CurrentToken : File.tokens())
      {
        if (CurrentToken.Kind != TokenKind::EndOfFile)
        {
          EXPECT_FALSE(CurrentToken.Span.empty());
          EXPECT_TRUE(CurrentToken.isError());
        }
      }
      expectPartition(File);
    }

    // Verifies progress and exact byte partitioning across deterministic arbitrary byte buffers.
    TEST(TokenStreamContractTest, ArbitraryByteInputsAlwaysAdvanceAndPreserveTheExactPartition)
    {
      std::mt19937 Generator(0x1A2B3C4D);
      std::uniform_int_distribution<int> LengthDistribution(0, 64);
      std::uniform_int_distribution<int> ByteDistribution(0, 255);
      for (std::size_t CaseIndex = 0; CaseIndex < 1000; ++CaseIndex)
      {
        std::string Source(static_cast<std::size_t>(LengthDistribution(Generator)), '\0');
        for (char &Value : Source)
        {
          Value = static_cast<char>(ByteDistribution(Generator));
        }
        SCOPED_TRACE(CaseIndex);
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_LE(File.tokens().size(), Source.size() + 1);
        expectPartition(File);
      }
    }

    // Verifies that every diagnostic span is ordered and bounded in source-byte coordinates.
    TEST(TokenStreamContractTest, DiagnosticsUseSourceByteSpans)
    {
      const std::string Source = "ok ? 0x \"unterminated";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_FALSE(File.diagnostics().empty());
      for (const Diagnostic &CurrentDiagnostic : File.diagnostics())
      {
        EXPECT_LE(CurrentDiagnostic.Span.Start, CurrentDiagnostic.Span.End);
        EXPECT_LE(CurrentDiagnostic.Span.End, Source.size());
      }
      expectPartition(File);
    }

    // Verifies that repeated tokenization produces identical complete tokens, raw slices, and diagnostics.
    TEST(TokenStreamContractTest, RepeatedTokenizationIsDeterministic)
    {
      const std::string Source = "let value: i32 = 0xFFu8; // comment\r\n\"text\\n\"";
      const TokenizedBuffer First = tokenize(Source);
      const TokenizedBuffer Second = tokenize(Source);

      ASSERT_EQ(First.succeeded(), Second.succeeded());
      ASSERT_EQ(First.tokens().size(), Second.tokens().size());
      ASSERT_EQ(First.diagnostics().size(), Second.diagnostics().size());
      for (std::size_t Index = 0; Index < First.tokens().size(); ++Index)
      {
        EXPECT_EQ(First.tokens()[Index], Second.tokens()[Index]);
        EXPECT_EQ(First.raw(First.tokens()[Index]), Second.raw(Second.tokens()[Index]));
      }
      for (std::size_t Index = 0; Index < First.diagnostics().size(); ++Index)
      {
        EXPECT_EQ(First.diagnostics()[Index], Second.diagnostics()[Index]);
      }
      expectPartition(First);
      expectPartition(Second);
    }

    // Verifies that parser-level nonsense remains a successful result when every byte is lexically valid.
    TEST(TokenStreamContractTest, SyntacticallyInvalidButLexicallyValidSourcesStillSucceed)
    {
      const std::vector<std::string> Sources = {
          ")(",
          "unknown_name",
          "let",
          "{[(",
          "import \"definitely-missing.ink\"",
          "a < /* gap */ = b",
          "\"value\"name",
          "999999999999999999999999999999999999999999i8",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_TRUE(File.succeeded());
        EXPECT_TRUE(File.diagnostics().empty());
        EXPECT_TRUE(std::none_of(File.tokens().begin(), File.tokens().end(), [](const Token &CurrentToken)
                                 {
                                   return CurrentToken.isError();
                                 }));
        expectPartition(File);
      }
    }

    // Verifies that exceeding the configured block-comment depth reports failure without losing the partition.
    TEST(TokenStreamContractTest, BlockCommentDepthLimitFailsWithoutBreakingPartition)
    {
      const std::string Source = "/* outer /* inner */ outer */ after";
      TokenizerOptions Options;
      Options.MaxBlockCommentDepth = 1;
      const TokenizedBuffer File = tokenize(Source, Options);

      ASSERT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::BlockCommentNestingLimit));
      EXPECT_TRUE(std::any_of(File.tokens().begin(), File.tokens().end(), [](const Token &CurrentToken)
                              {
                                return CurrentToken.isError();
                              }));
      expectPartition(File);
    }
  } // namespace
} // namespace ink::tokenizer
