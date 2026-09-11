#ifndef INK_TESTCASE_TOKENIZER_GRAMMAR_TEST_SUPPORT_H
#define INK_TESTCASE_TOKENIZER_GRAMMAR_TEST_SUPPORT_H

#include "tokenizer_test_support.h"
#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace ink::tokenizer::grammar_test
{
  inline TokenizerOptions preservingTrivia(std::size_t MaxDepth = 0)
  {
    TokenizerOptions Options;
    Options.PreserveTrivia = true;
    Options.MaxBlockCommentDepth = MaxDepth;
    return Options;
  }

  inline std::string bytes(std::initializer_list<unsigned> Values)
  {
    std::string Result;
    for (unsigned Value : Values)
    {
      Result.push_back(static_cast<char>(Value));
    }
    return Result;
  }

  inline void appendScalar(std::string &Result, char32_t Value)
  {
    if (Value <= 0x7F)
    {
      Result.push_back(static_cast<char>(Value));
    }
    else if (Value <= 0x7FF)
    {
      Result.push_back(static_cast<char>(0xC0 | (Value >> 6)));
      Result.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
    }
    else if (Value <= 0xFFFF)
    {
      Result.push_back(static_cast<char>(0xE0 | (Value >> 12)));
      Result.push_back(static_cast<char>(0x80 | ((Value >> 6) & 0x3F)));
      Result.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
    }
    else
    {
      Result.push_back(static_cast<char>(0xF0 | (Value >> 18)));
      Result.push_back(static_cast<char>(0x80 | ((Value >> 12) & 0x3F)));
      Result.push_back(static_cast<char>(0x80 | ((Value >> 6) & 0x3F)));
      Result.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
    }
  }

  inline bool hasDiagnostic(const TokenizedBuffer &File, core::DiagnosticKind Kind)
  {
    return std::any_of(testDiagnostics(File).begin(), testDiagnostics(File).end(), [Kind](const core::Diagnostic &Entry)
    {
      return Entry.Kind == Kind;
    });
  }

  inline void expectStream(const TokenizedBuffer &File, bool FullFidelity = false)
  {
    ASSERT_FALSE(File.tokens().empty());
    std::size_t PreviousEnd = 0;
    std::size_t EofCount = 0;
    std::string Rebuilt;
    for (const Token &Entry : File.tokens())
    {
      EXPECT_LE(PreviousEnd, Entry.Span.Start);
      EXPECT_LE(Entry.Span.Start, Entry.Span.End);
      EXPECT_LE(Entry.Span.End, File.source().size());
      EXPECT_EQ(File.raw(Entry).size(), Entry.Span.size());
      if (Entry.Kind == TokenKind::EndOfFile)
      {
        ++EofCount;
        EXPECT_EQ(&Entry, &File.tokens().back());
        EXPECT_EQ(Entry.Span, (core::SourceRange{File.source().size(), File.source().size()}));
      }
      else
      {
        EXPECT_LT(Entry.Span.Start, Entry.Span.End);
        if (FullFidelity)
        {
          EXPECT_EQ(PreviousEnd, Entry.Span.Start);
          Rebuilt.append(File.raw(Entry));
        }
        else
        {
          EXPECT_FALSE(Entry.isTrivia());
        }
      }
      PreviousEnd = Entry.Span.End;
    }
    EXPECT_EQ(EofCount, 1U);
    if (FullFidelity)
    {
      EXPECT_EQ(Rebuilt, File.source());
    }
    for (const core::Diagnostic &Entry : testDiagnostics(File))
    {
      EXPECT_EQ(Entry.Source, File.sourceId());
      EXPECT_LE(Entry.Span.Start, Entry.Span.End);
      EXPECT_LE(Entry.Span.End, File.source().size());
      for (const core::DiagnosticRelatedInformation &Related : Entry.Related)
      {
        EXPECT_LE(Related.Span.Start, Related.Span.End);
        EXPECT_LE(Related.Span.End, File.source().size());
      }
    }
  }

  inline void expectRaws(const TokenizedBuffer &File, const std::vector<std::string> &Expected)
  {
    ASSERT_EQ(File.tokens().size(), Expected.size() + 1);
    for (std::size_t Index = 0; Index < Expected.size(); ++Index)
    {
      SCOPED_TRACE(Index);
      EXPECT_EQ(File.raw(File.tokens()[Index]), Expected[Index]);
    }
    expectStream(File);
  }

  inline void expectString(std::string Source, StringMode Mode, const std::string &Decoded)
  {
    const TokenizedBuffer File = tokenize(std::move(Source));
    ASSERT_TRUE(File.succeeded());
    ASSERT_TRUE(testDiagnostics(File).empty());
    ASSERT_EQ(File.tokens().size(), 2U);
    EXPECT_EQ(File.tokens()[0].Kind, TokenKind::StringLiteral);
    const StringInfo *Payload = std::get_if<StringInfo>(&File.tokens()[0].Payload);
    ASSERT_NE(Payload, nullptr);
    EXPECT_EQ(Payload->Mode, Mode);
    EXPECT_EQ(Payload->Decoded, Decoded);
    expectStream(File);
  }

  inline void expectStringError(std::string Source, core::DiagnosticKind Kind)
  {
    const TokenizedBuffer File = tokenize(std::move(Source));
    ASSERT_FALSE(File.succeeded());
    ASSERT_FALSE(File.tokens().empty());
    EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidStringLiteral);
    EXPECT_TRUE(hasDiagnostic(File, Kind));
    expectStream(File);
  }
} // namespace ink::tokenizer::grammar_test

#endif
