#include "ink/tokenizer/tokenizer.h"
#include "tokenizer_test_support.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticArgument;
    using core::DiagnosticArgumentName;
    using core::DiagnosticKind;
    using core::DiagnosticRelatedKind;

    template <typename ValueType>
    const ValueType *findArgumentValue(const std::vector<DiagnosticArgument> &Arguments, DiagnosticArgumentName Name)
    {
      for (const DiagnosticArgument &Argument : Arguments)
      {
        if (Argument.Name == Name)
        {
          return std::get_if<ValueType>(&Argument.Value);
        }
      }
      return nullptr;
    }

    bool hasTokenKind(const TokenizedBuffer &Buffer, TokenKind Kind)
    {
      for (const Token &TokenEntry : Buffer.tokens())
      {
        if (TokenEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasDiagnosticKind(const TokenizedBuffer &Buffer, DiagnosticKind Kind)
    {
      for (const Diagnostic &DiagnosticEntry : Buffer.diagnostics())
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasErrorToken(const TokenizedBuffer &Buffer)
    {
      for (const Token &TokenEntry : Buffer.tokens())
      {
        if (TokenEntry.isError())
        {
          return true;
        }
      }
      return false;
    }

    void expectFullFidelity(const TokenizedBuffer &Buffer)
    {
      const std::vector<Token> &Tokens = Buffer.tokens();
      ASSERT_FALSE(Tokens.empty());
      EXPECT_EQ(Tokens.back().Kind, TokenKind::EndOfFile);
      EXPECT_EQ(Tokens.back().Span.Start, Buffer.source().size());
      EXPECT_EQ(Tokens.back().Span.End, Buffer.source().size());
      EXPECT_TRUE(Buffer.raw(Tokens.back()).empty());

      std::size_t NextByte = 0;
      std::size_t EofCount = 0;
      std::string Reconstructed;
      for (const Token &TokenEntry : Tokens)
      {
        EXPECT_LE(TokenEntry.Span.Start, TokenEntry.Span.End);
        EXPECT_LE(TokenEntry.Span.End, Buffer.source().size());
        EXPECT_EQ(TokenEntry.isTrivia(), isTrivia(TokenEntry.Kind));
        EXPECT_EQ(TokenEntry.isError(), isError(TokenEntry.Kind));
        if (TokenEntry.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(&TokenEntry, &Tokens.back());
          continue;
        }
        EXPECT_EQ(TokenEntry.Span.Start, NextByte);
        EXPECT_EQ(Buffer.raw(TokenEntry).size(), TokenEntry.Span.size());
        Reconstructed.append(Buffer.raw(TokenEntry));
        NextByte = TokenEntry.Span.End;
      }
      EXPECT_EQ(EofCount, 1U);
      EXPECT_EQ(NextByte, Buffer.source().size());
      EXPECT_EQ(Reconstructed, Buffer.source());
    }

    // Tests that trivia and significant tokens share one source-ordered token stream.
    TEST(TriviaCommentsTest, TriviaTokensShareTheSingleOrderedTokenList)
    {
      const std::string Source = utf8(u8"\uFEFF \tlet\r\n//comment\n/**/");
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 8U);
      const std::vector<TokenKind> Expected = {
          TokenKind::Utf8Bom,
          TokenKind::SpacesAndTabs,
          TokenKind::Keyword,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::LineBreak,
          TokenKind::BlockComment,
          TokenKind::EndOfFile,
      };
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].Kind, Expected[Index]);
      }
      for (std::size_t Index = 0; Index + 1 < Buffer.tokens().size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].isTrivia(), Index != 2U);
      }
      expectFullFidelity(Buffer);
    }

    // Tests maximal grouping of adjacent ASCII spaces and tabs.
    TEST(TriviaCommentsTest, AdjacentSpacesAndTabsFormOneMaximalTriviaToken)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, " \t  \t");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), " \t  \t");
      EXPECT_TRUE(Buffer.tokens()[0].isTrivia());
      expectFullFidelity(Buffer);
    }

    // Tests that line-comment tokens exclude both LF and CRLF terminators.
    TEST(TriviaCommentsTest, LineCommentExcludesLfOrCrLfTerminator)
    {
      struct LineEndingCase
      {
        const char *Source;
        const char *LineBreak;
      };
      const std::vector<LineEndingCase> Cases = {
          {"// comment\n", "\n"},
          {"// comment\r\n", "\r\n"},
      };

      for (const LineEndingCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.LineBreak);
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, TestCase.Source);
        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 3U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "// comment");
        EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), TestCase.LineBreak);
        expectFullFidelity(Buffer);
      }
    }

    // Tests that a line comment can terminate directly at end of file.
    TEST(TriviaCommentsTest, LineCommentMayEndAtEof)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "// no final line break");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "// no final line break");
      expectFullFidelity(Buffer);
    }

    // Tests that Unicode newline lookalikes remain text inside a line comment.
    TEST(TriviaCommentsTest, UnicodeNewlineLookalikesDoNotEndLineComments)
    {
      const std::string Source = utf8(u8"//a\u0085b\u2028c\u2029d\nx");
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), utf8(u8"//a\u0085b\u2028c\u2029d"));
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "\n");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      expectFullFidelity(Buffer);
    }

    // Tests that a BOM inside a line comment is preserved as comment text.
    TEST(TriviaCommentsTest, NonInitialByteOrderMarkIsPreservedAsCommentText)
    {
      const std::string Source = utf8(u8"//a\uFEFFb");
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      expectFullFidelity(Buffer);
    }

    // Tests that all specified Unicode whitespace and line-separator lookalikes remain comment text and do not create source lines.
    TEST(TriviaCommentsTest, UnicodeWhitespaceInsideCommentsIsPreservedWithoutChangingLineStarts)
    {
      const std::string UnicodeWhitespace = utf8(u8"\u0085\u00A0\u1680\u2000\u2003\u200A\u2028\u2029\u202F\u205F\u3000");
      const std::string LineComment = std::string("//line") + UnicodeWhitespace;
      const std::string BlockComment = std::string("/*block") + UnicodeWhitespace + "*/";
      const std::string Source = LineComment + "\r\n" + BlockComment;
      const std::size_t BlockStart = LineComment.size() + 2;
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_TRUE(Buffer.diagnostics().empty());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), LineComment);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "\r\n");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), BlockComment);
      EXPECT_EQ(Buffer.lineStarts(), (std::vector<std::size_t>{0, BlockStart}));
      EXPECT_EQ(Buffer.lineNumber(LineComment.find(utf8(u8"\u2028"))), 1U);
      EXPECT_EQ(Buffer.lineNumber(BlockStart + BlockComment.find(utf8(u8"\u2029"))), 2U);
      expectFullFidelity(Buffer);
    }

    // Tests that a lone carriage return remains inside a line-comment token but receives an exact diagnostic before LF recovery.
    TEST(TriviaCommentsTest, LoneCarriageReturnInsideLineCommentIsDiagnosedPrecisely)
    {
      const std::string Source = "//a\rb\nx";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, 5U);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "//a\rb");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.tokens()[1].Span.Start, 5U);
      EXPECT_EQ(Buffer.tokens()[1].Span.End, 6U);
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "x");
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::LoneCarriageReturn);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, 3U);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.End, 4U);
      EXPECT_EQ(Buffer.lineStarts(), (std::vector<std::size_t>{0, 6}));
      expectFullFidelity(Buffer);
    }

    // Tests nested block-comment scanning as one token across internal line breaks.
    TEST(TriviaCommentsTest, NestedBlockCommentIsOneTokenIncludingInternalLineBreaks)
    {
      const std::string Source = "/* outer\n/* inner */ \" // still outer */tail";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/* outer\n/* inner */ \" // still outer */");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      EXPECT_FALSE(hasTokenKind(Buffer, TokenKind::LineBreak));
      expectFullFidelity(Buffer);
    }

    // Tests line mapping for LF and CRLF bytes hidden inside one opaque block-comment token.
    TEST(TriviaCommentsTest, BlockCommentInternalLineBreaksPopulateExactLineStarts)
    {
      const std::string BlockComment = "/*first\r\nsecond\nthird*/";
      const std::string Source = BlockComment + "tail";
      const std::size_t SecondLineStart = BlockComment.find("\r\n") + 2;
      const std::size_t ThirdLineStart = BlockComment.find('\n', SecondLineStart) + 1;
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, BlockComment.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), BlockComment);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      EXPECT_FALSE(hasTokenKind(Buffer, TokenKind::LineBreak));
      EXPECT_EQ(Buffer.lineStarts(), (std::vector<std::size_t>{0, SecondLineStart, ThirdLineStart}));
      EXPECT_EQ(Buffer.lineNumber(SecondLineStart - 1), 1U);
      EXPECT_EQ(Buffer.lineNumber(SecondLineStart), 2U);
      EXPECT_EQ(Buffer.lineNumber(ThirdLineStart - 1), 2U);
      EXPECT_EQ(Buffer.lineNumber(ThirdLineStart), 3U);
      EXPECT_EQ(Buffer.lineNumber(Source.size()), 3U);
      expectFullFidelity(Buffer);
    }

    // Tests exact invalid UTF-8 recovery inside closed line and block comments without consuming following valid tokens.
    TEST(TriviaCommentsTest, ClosedCommentsReportInvalidUtf8WithExactSpansAndRecovery)
    {
      std::string InvalidSequence;
      InvalidSequence.push_back(static_cast<char>(0xE2));
      InvalidSequence.push_back(static_cast<char>(0x82));

      const std::string LinePrefix = "//before";
      const std::string LineComment = LinePrefix + InvalidSequence + "after";
      const std::string LineSource = LineComment + "\nnext";
      const TokenizedBuffer LineBuffer = tokenize(TestSourceFileId, LineSource);

      ASSERT_FALSE(LineBuffer.succeeded());
      ASSERT_EQ(LineBuffer.tokens().size(), 4U);
      EXPECT_EQ(LineBuffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
      EXPECT_EQ(LineBuffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(LineBuffer.tokens()[0].Span.End, LineComment.size());
      EXPECT_EQ(LineBuffer.raw(LineBuffer.tokens()[0]), LineComment);
      EXPECT_EQ(LineBuffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(LineBuffer.tokens()[1].Span.Start, LineComment.size());
      EXPECT_EQ(LineBuffer.tokens()[1].Span.End, LineComment.size() + 1);
      EXPECT_EQ(LineBuffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(LineBuffer.raw(LineBuffer.tokens()[2]), "next");
      ASSERT_EQ(LineBuffer.diagnostics().size(), 1U);
      EXPECT_EQ(LineBuffer.diagnostics()[0].Kind, DiagnosticKind::InvalidUtf8);
      EXPECT_EQ(LineBuffer.diagnostics()[0].Span.Start, LinePrefix.size());
      EXPECT_EQ(LineBuffer.diagnostics()[0].Span.End, LinePrefix.size() + InvalidSequence.size());
      expectFullFidelity(LineBuffer);

      const std::string BlockPrefix = "/*before";
      const std::string BlockComment = BlockPrefix + InvalidSequence + "after*/";
      const std::string BlockSource = BlockComment + "next";
      const TokenizedBuffer BlockBuffer = tokenize(TestSourceFileId, BlockSource);

      ASSERT_FALSE(BlockBuffer.succeeded());
      ASSERT_EQ(BlockBuffer.tokens().size(), 3U);
      EXPECT_EQ(BlockBuffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
      EXPECT_EQ(BlockBuffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(BlockBuffer.tokens()[0].Span.End, BlockComment.size());
      EXPECT_EQ(BlockBuffer.raw(BlockBuffer.tokens()[0]), BlockComment);
      EXPECT_EQ(BlockBuffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(BlockBuffer.raw(BlockBuffer.tokens()[1]), "next");
      ASSERT_EQ(BlockBuffer.diagnostics().size(), 1U);
      EXPECT_EQ(BlockBuffer.diagnostics()[0].Kind, DiagnosticKind::InvalidUtf8);
      EXPECT_EQ(BlockBuffer.diagnostics()[0].Span.Start, BlockPrefix.size());
      EXPECT_EQ(BlockBuffer.diagnostics()[0].Span.End, BlockPrefix.size() + InvalidSequence.size());
      expectFullFidelity(BlockBuffer);
    }

    // Tests acceptance of an empty block comment.
    TEST(TriviaCommentsTest, EmptyBlockCommentIsLegal)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "/**/");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/**/");
      expectFullFidelity(Buffer);
    }

    // Tests that comment delimiters have meaning only in the active lexical state.
    TEST(TriviaCommentsTest, CommentDelimitersOnlyActInTheActiveLexicalState)
    {
      const std::string Source = "// /* not a block */ \"not a string\"\n\"https://example.com/*text*/\" /* // nested text */";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 6U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::StringLiteral);
      EXPECT_EQ(Buffer.tokens()[3].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Buffer.tokens()[4].Kind, TokenKind::BlockComment);
      expectFullFidelity(Buffer);
    }

    // Tests that trivia establishes boundaries between adjacent identifiers and symbols.
    TEST(TriviaCommentsTest, TriviaForcesIdentifierAndSymbolBoundaries)
    {
      const std::string Source = "first/* comment */second +/* comment */+";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      const std::vector<TokenKind> Expected = {
          TokenKind::Identifier,
          TokenKind::BlockComment,
          TokenKind::Identifier,
          TokenKind::SpacesAndTabs,
          TokenKind::Symbol,
          TokenKind::BlockComment,
          TokenKind::Symbol,
          TokenKind::EndOfFile,
      };
      ASSERT_EQ(Buffer.tokens().size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].Kind, Expected[Index]);
      }
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "first");
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "second");
      EXPECT_EQ(std::get<char>(Buffer.tokens()[4].Payload), '+');
      EXPECT_EQ(std::get<char>(Buffer.tokens()[6].Payload), '+');
      expectFullFidelity(Buffer);
    }

    // Tests that documentation-like spellings remain ordinary comment tokens.
    TEST(TriviaCommentsTest, DocumentationLikeCommentsRemainOrdinaryComments)
    {
      const std::string Source = "/// line\n//! inner\n/** block */ /*! inner */";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_TRUE(Buffer.succeeded());
      const std::vector<TokenKind> Expected = {
          TokenKind::LineComment,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::LineBreak,
          TokenKind::BlockComment,
          TokenKind::SpacesAndTabs,
          TokenKind::BlockComment,
          TokenKind::EndOfFile,
      };
      ASSERT_EQ(Buffer.tokens().size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].Kind, Expected[Index]);
      }
      expectFullFidelity(Buffer);
    }

    // Tests that a closing block-comment delimiter outside a comment becomes two symbols.
    TEST(TriviaCommentsTest, StarSlashOutsideBlockCommentIsTwoSymbols)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "*/");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Buffer.tokens()[0].Payload), '*');
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Buffer.tokens()[1].Payload), '/');
      expectFullFidelity(Buffer);
    }

    // Tests that a single unterminated block comment reports its remaining depth without a redundant related range at the primary opening.
    TEST(TriviaCommentsTest, SingleUnterminatedBlockCommentHasNoRedundantRelatedOpening)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "/* text");

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      const Diagnostic &DiagnosticEntry = Buffer.diagnostics().front();
      EXPECT_EQ(DiagnosticEntry.Kind, DiagnosticKind::UnterminatedBlockComment);
      EXPECT_EQ(DiagnosticEntry.Span, (core::SourceRange{0, 2}));
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1U);
      const std::uint64_t *RemainingNestingDepth = findArgumentValue<std::uint64_t>(DiagnosticEntry.Arguments, DiagnosticArgumentName::RemainingNestingDepth);
      ASSERT_NE(RemainingNestingDepth, nullptr);
      EXPECT_EQ(*RemainingNestingDepth, 1U);
      EXPECT_TRUE(DiagnosticEntry.Related.empty());
      expectFullFidelity(Buffer);
    }

    // Tests unterminated nested-comment coverage and detailed nesting diagnostics.
    TEST(TriviaCommentsTest, UnterminatedNestedBlockCommentCoversFromOutermostStartToEof)
    {
      const std::string Source = "/* outer /* inner";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::UnterminatedBlockComment);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      const Diagnostic &DiagnosticEntry = Buffer.diagnostics().front();
      EXPECT_EQ(DiagnosticEntry.Kind, DiagnosticKind::UnterminatedBlockComment);
      EXPECT_EQ(DiagnosticEntry.Span, (core::SourceRange{0, 2}));
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1U);
      const std::uint64_t *RemainingNestingDepth = findArgumentValue<std::uint64_t>(DiagnosticEntry.Arguments, DiagnosticArgumentName::RemainingNestingDepth);
      ASSERT_NE(RemainingNestingDepth, nullptr);
      EXPECT_EQ(*RemainingNestingDepth, 2U);
      EXPECT_EQ(findArgumentValue<bool>(DiagnosticEntry.Arguments, DiagnosticArgumentName::MostRecentOpeningUnavailable), nullptr);
      ASSERT_EQ(DiagnosticEntry.Related.size(), 1U);
      EXPECT_EQ(DiagnosticEntry.Related[0].Kind, DiagnosticRelatedKind::MostRecentUnclosedBlockComment);
      EXPECT_EQ(DiagnosticEntry.Related[0].Span, (core::SourceRange{9, 11}));
      EXPECT_TRUE(DiagnosticEntry.Related[0].Arguments.empty());
      expectFullFidelity(Buffer);
    }

    // Tests the combined diagnostics when an over-limit nested block comment also reaches end of file before closing.
    TEST(TriviaCommentsTest, OverLimitUnterminatedBlockCommentReportsBothExactDiagnostics)
    {
      const std::string Source = "/* outer /* inner /* too deep";
      const std::size_t OverLimitOpening = Source.rfind("/*");
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source, TokenizerOptions{2});

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::UnterminatedBlockComment);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::EndOfFile);

      ASSERT_EQ(Buffer.diagnostics().size(), 2U);
      EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::BlockCommentNestingLimit);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, OverLimitOpening);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.End, OverLimitOpening + 2);
      EXPECT_TRUE(Buffer.diagnostics()[0].Arguments.empty());
      EXPECT_TRUE(Buffer.diagnostics()[0].Related.empty());
      EXPECT_EQ(Buffer.diagnostics()[1].Kind, DiagnosticKind::UnterminatedBlockComment);
      EXPECT_EQ(Buffer.diagnostics()[1].Span.Start, 0U);
      EXPECT_EQ(Buffer.diagnostics()[1].Span.End, 2U);
      ASSERT_EQ(Buffer.diagnostics()[1].Arguments.size(), 2U);
      const std::uint64_t *RemainingNestingDepth = findArgumentValue<std::uint64_t>(Buffer.diagnostics()[1].Arguments, DiagnosticArgumentName::RemainingNestingDepth);
      ASSERT_NE(RemainingNestingDepth, nullptr);
      EXPECT_EQ(*RemainingNestingDepth, 3U);
      const bool *MostRecentOpeningUnavailable = findArgumentValue<bool>(Buffer.diagnostics()[1].Arguments, DiagnosticArgumentName::MostRecentOpeningUnavailable);
      ASSERT_NE(MostRecentOpeningUnavailable, nullptr);
      EXPECT_TRUE(*MostRecentOpeningUnavailable);
      EXPECT_TRUE(Buffer.diagnostics()[1].Related.empty());
      expectFullFidelity(Buffer);
    }

    // Tests that a zero block-comment depth limit rejects even a closed empty comment and preserves following source text.
    TEST(TriviaCommentsTest, ZeroBlockCommentDepthLimitRejectsTheOutermostComment)
    {
      const std::string Source = "/**/tail";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source, TokenizerOptions{0});

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/**/");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      EXPECT_EQ(Buffer.diagnostics().front().Kind, DiagnosticKind::BlockCommentNestingLimit);
      expectFullFidelity(Buffer);
    }

    // Tests that a block comment nested exactly to the configured depth remains valid trivia.
    TEST(TriviaCommentsTest, ExactBlockCommentDepthLimitIsAccepted)
    {
      const std::string Source = "/* outer /* inner */ outer */tail";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source, TokenizerOptions{2});

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/* outer /* inner */ outer */");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      EXPECT_TRUE(Buffer.diagnostics().empty());
      expectFullFidelity(Buffer);
    }

    // Tests that exceeding the configured depth still consumes the closed comment and resumes at following source text.
    TEST(TriviaCommentsTest, ConfiguredBlockCommentNestingLimitProducesAnErrorWithoutLosingBytes)
    {
      const std::string Source = "/* one /* two /* three */ two */ one */tail";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source, TokenizerOptions{2});

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/* one /* two /* three */ two */ one */");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      EXPECT_EQ(Buffer.diagnostics().front().Kind, DiagnosticKind::BlockCommentNestingLimit);
      EXPECT_TRUE(hasErrorToken(Buffer));
      expectFullFidelity(Buffer);
    }

    // Tests that a backslash cannot continue a physical source line.
    TEST(TriviaCommentsTest, BackslashBeforeLineBreakDoesNotContinueTheLine)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "a\\\nb");

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 5U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "\\");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.tokens()[3].Kind, TokenKind::Identifier);
      expectFullFidelity(Buffer);
    }
  } // namespace
} // namespace ink::tokenizer
