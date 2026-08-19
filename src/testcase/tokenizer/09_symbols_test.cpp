#include "tokenizer_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;
    using core::SourceRange;

    void expectPartition(const TokenizedBuffer &File)
    {
      ASSERT_FALSE(File.tokens().empty());
      std::size_t Cursor = 0;
      std::size_t EofCount = 0;
      std::string Rebuilt;
      for (const Token &CurrentToken : File.tokens())
      {
        if (CurrentToken.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(CurrentToken.Span, (SourceRange{File.source().size(), File.source().size()}));
          EXPECT_TRUE(File.raw(CurrentToken).empty());
          continue;
        }
        EXPECT_EQ(CurrentToken.Span.Start, Cursor);
        EXPECT_EQ(CurrentToken.Span.size(), File.raw(CurrentToken).size());
        Rebuilt.append(File.raw(CurrentToken).data(), File.raw(CurrentToken).size());
        Cursor = CurrentToken.Span.End;
      }
      EXPECT_EQ(Cursor, File.source().size());
      EXPECT_EQ(Rebuilt, File.source());
      EXPECT_EQ(EofCount, 1u);
      EXPECT_EQ(File.tokens().back().Kind, TokenKind::EndOfFile);
    }

    void expectToken(const TokenizedBuffer &File, std::size_t Index, TokenKind Kind, std::string_view Raw)
    {
      ASSERT_LT(Index, File.tokens().size());
      EXPECT_EQ(File.tokens()[Index].Kind, Kind);
      EXPECT_EQ(std::string(File.raw(File.tokens()[Index])), std::string(Raw));
    }

    bool hasDiagnostic(const TokenizedBuffer &File, DiagnosticKind Kind)
    {
      return std::any_of(testDiagnostics(File).begin(), testDiagnostics(File).end(), [Kind](const Diagnostic &CurrentDiagnostic)
                         {
                           return CurrentDiagnostic.Kind == Kind;
                         });
    }

    // Verifies that every accepted punctuation byte produces one symbol token with the matching payload.
    TEST(SymbolTokenTest, LexesEveryAcceptedSymbolAsOneByteToken)
    {
      const std::string Symbols = "(){}[],;:.@+-*/%=!&|^~<>";
      const TokenizedBuffer File = tokenize(Symbols);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), Symbols.size() + 1);
      for (std::size_t Index = 0; Index < Symbols.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        EXPECT_EQ(File.tokens()[Index].Kind, TokenKind::Symbol);
        EXPECT_EQ(File.tokens()[Index].Span, (SourceRange{Index, Index + 1}));
        EXPECT_EQ(File.raw(File.tokens()[Index]), std::string_view(Symbols).substr(Index, 1));
        ASSERT_TRUE(std::holds_alternative<char>(File.tokens()[Index].Payload));
        EXPECT_EQ(std::get<char>(File.tokens()[Index].Payload), Symbols[Index]);
      }
      expectPartition(File);
    }

    // Verifies that multi-byte punctuation spellings always remain individual one-byte symbol tokens.
    TEST(SymbolTokenTest, NeverCombinesCompoundPunctuationOrOperators)
    {
      const std::vector<std::string> Spellings = {
          "::",
          "::<",
          "..",
          "...",
          "->",
          "=>",
          "==",
          "!=",
          "<=",
          ">=",
          "&&",
          "||",
          "<<",
          ">>",
          "+=",
          ">>=",
          "++",
          "--",
      };
      for (const std::string &Spelling : Spellings)
      {
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer File = tokenize(Spelling);
        ASSERT_TRUE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), Spelling.size() + 1);
        for (std::size_t Index = 0; Index < Spelling.size(); ++Index)
        {
          EXPECT_EQ(File.tokens()[Index].Kind, TokenKind::Symbol);
          EXPECT_EQ(std::get<char>(File.tokens()[Index].Payload), Spelling[Index]);
          EXPECT_EQ(File.tokens()[Index].Span, (SourceRange{Index, Index + 1}));
        }
        expectPartition(File);
      }
    }

    // Verifies that whitespace and comments preserve the boundaries between neighboring symbols.
    TEST(SymbolTokenTest, TriviaPreservesCompoundSymbolBoundaries)
    {
      const TokenizedBuffer Adjacent = tokenize("a<=b");
      const TokenizedBuffer Spaced = tokenize("a < = b");
      const TokenizedBuffer Commented = tokenize("a < /*c*/ = b");

      ASSERT_TRUE(Adjacent.succeeded());
      ASSERT_EQ(Adjacent.tokens().size(), 5u);
      expectToken(Adjacent, 1, TokenKind::Symbol, "<");
      expectToken(Adjacent, 2, TokenKind::Symbol, "=");
      EXPECT_EQ(Adjacent.tokens()[1].Span.End, Adjacent.tokens()[2].Span.Start);

      ASSERT_TRUE(Spaced.succeeded());
      ASSERT_EQ(Spaced.tokens().size(), 8u);
      expectToken(Spaced, 2, TokenKind::Symbol, "<");
      expectToken(Spaced, 3, TokenKind::SpacesAndTabs, " ");
      expectToken(Spaced, 4, TokenKind::Symbol, "=");
      EXPECT_NE(Spaced.tokens()[2].Span.End, Spaced.tokens()[4].Span.Start);

      ASSERT_TRUE(Commented.succeeded());
      ASSERT_EQ(Commented.tokens().size(), 10u);
      expectToken(Commented, 2, TokenKind::Symbol, "<");
      expectToken(Commented, 4, TokenKind::BlockComment, "/*c*/");
      expectToken(Commented, 6, TokenKind::Symbol, "=");
      EXPECT_NE(Commented.tokens()[2].Span.End, Commented.tokens()[6].Span.Start);
      expectPartition(Adjacent);
      expectPartition(Spaced);
      expectPartition(Commented);
    }

    // Verifies that adjacent generic closing brackets and assignment remain separate symbol tokens.
    TEST(SymbolTokenTest, NestedGenericClosersRemainSeparateSymbols)
    {
      const TokenizedBuffer Nested = tokenize("Vector::<Vector::<i32>>");
      const TokenizedBuffer Assignment = tokenize("Container::<Item::<T>>=value");

      ASSERT_TRUE(Nested.succeeded());
      ASSERT_EQ(Nested.tokens().size(), 12u);
      expectToken(Nested, 9, TokenKind::Symbol, ">");
      expectToken(Nested, 10, TokenKind::Symbol, ">");
      EXPECT_EQ(Nested.tokens()[9].Span.End, Nested.tokens()[10].Span.Start);

      ASSERT_TRUE(Assignment.succeeded());
      ASSERT_GE(Assignment.tokens().size(), 5u);
      const std::size_t ValueIndex = Assignment.tokens().size() - 2;
      expectToken(Assignment, ValueIndex - 3, TokenKind::Symbol, ">");
      expectToken(Assignment, ValueIndex - 2, TokenKind::Symbol, ">");
      expectToken(Assignment, ValueIndex - 1, TokenKind::Symbol, "=");
      expectToken(Assignment, ValueIndex, TokenKind::Identifier, "value");
      expectPartition(Nested);
      expectPartition(Assignment);
    }

    // Verifies that a numeric token absorbs a decimal point only when a digit follows it.
    TEST(SymbolTokenTest, NumericScannerOwnsOnlyDecimalPointFollowedByDigit)
    {
      const std::string Source = "1.member 1..10 0...value";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 15u);
      expectToken(File, 0, TokenKind::IntegerLiteral, "1");
      expectToken(File, 1, TokenKind::Symbol, ".");
      expectToken(File, 2, TokenKind::Identifier, "member");
      expectToken(File, 4, TokenKind::IntegerLiteral, "1");
      expectToken(File, 5, TokenKind::Symbol, ".");
      expectToken(File, 6, TokenKind::Symbol, ".");
      expectToken(File, 7, TokenKind::IntegerLiteral, "10");
      expectToken(File, 9, TokenKind::IntegerLiteral, "0");
      expectToken(File, 10, TokenKind::Symbol, ".");
      expectToken(File, 11, TokenKind::Symbol, ".");
      expectToken(File, 12, TokenKind::Symbol, ".");
      expectToken(File, 13, TokenKind::Identifier, "value");
      expectPartition(File);
    }

    // Verifies that comment openers take precedence over standalone slash and star symbols.
    TEST(SymbolTokenTest, CommentDelimitersTakePriorityOverSlashSymbols)
    {
      const std::string Source = "//line\n/* block */ / / / * */";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 15u);
      expectToken(File, 0, TokenKind::LineComment, "//line");
      expectToken(File, 1, TokenKind::LineBreak, "\n");
      expectToken(File, 2, TokenKind::BlockComment, "/* block */");
      expectToken(File, 4, TokenKind::Symbol, "/");
      expectToken(File, 6, TokenKind::Symbol, "/");
      expectToken(File, 8, TokenKind::Symbol, "/");
      expectToken(File, 10, TokenKind::Symbol, "*");
      expectToken(File, 12, TokenKind::Symbol, "*");
      expectToken(File, 13, TokenKind::Symbol, "/");
      expectPartition(File);
    }

    // Verifies that scalar and string literal delimiters take precedence over symbol scanning.
    TEST(SymbolTokenTest, LiteralDelimitersTakePriorityOverSymbolScanning)
    {
      const std::string Source = R"ink('/' "/" r"/*")ink";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 6u);
      expectToken(File, 0, TokenKind::ScalarLiteral, "'/'");
      expectToken(File, 2, TokenKind::StringLiteral, "\"/\"");
      expectToken(File, 4, TokenKind::StringLiteral, "r\"/*\"");
      expectPartition(File);
    }

    // Verifies that a symbol payload depends only on its byte and not on its apparent syntactic role.
    TEST(SymbolTokenTest, SameSymbolPayloadIsIndependentOfSyntacticRole)
    {
      const std::string Source = "a*b value:T* *pointer a&b value:T& &value a<b Vector::<i32>";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      std::vector<char> SymbolValues;
      for (const Token &CurrentToken : File.tokens())
      {
        if (CurrentToken.Kind == TokenKind::Symbol)
        {
          SymbolValues.push_back(std::get<char>(CurrentToken.Payload));
        }
      }
      EXPECT_EQ(SymbolValues, (std::vector<char>{'*', ':', '*', '*', '&', ':', '&', '&', '<', ':', ':', '<', '>'}));
      expectPartition(File);
    }

    // Verifies that unsupported ASCII punctuation becomes an error token with an invalid-character diagnostic.
    TEST(SymbolTokenTest, RejectsAsciiCharactersOutsideTheSymbolTable)
    {
      const std::vector<std::string> Sources = {
          "?",
          "$",
          "#",
          "`",
          "\\",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 2u);
        expectToken(File, 0, TokenKind::InvalidCharacter, Source);
        EXPECT_TRUE(File.tokens()[0].isError());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidCharacter));
        expectPartition(File);
      }
    }

    // Verifies that a non-identifier Unicode scalar is preserved as one invalid-character token.
    TEST(SymbolTokenTest, RejectsNonIdentifierUnicodeCharacterAsOneErrorToken)
    {
      const std::string Emoji = "\xF0\x9F\x98\x80";
      const TokenizedBuffer File = tokenize(Emoji);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      expectToken(File, 0, TokenKind::InvalidCharacter, Emoji);
      EXPECT_EQ(File.tokens()[0].Span, (SourceRange{0, 4}));
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidCharacter));
      expectPartition(File);
    }
  } // namespace
} // namespace ink::tokenizer
