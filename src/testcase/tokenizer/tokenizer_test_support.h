#ifndef INK_TESTCASE_TOKENIZER_TEST_SUPPORT_H
#define INK_TESTCASE_TOKENIZER_TEST_SUPPORT_H

#include "ink/tokenizer/tokenizer.h"
#include "ink/tokenizer/unicode.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <utility>

namespace ink::tokenizer::test
{
  using core::DiagnosticKind;

  inline std::string utf8(char32_t Value)
  {
    std::string Result;
    unicode::appendUtf8(Result, Value);
    return Result;
  }

  class TokenizerTest : public testing::Test
  {
    protected:
      TokenizerTest()
          : Context(Compilation)
      {
        Compilation.diagnosticEngine().addConsumer(Diagnostics);
      }

      TokenizedBuffer lex(std::string Source)
      {
        Diagnostics.clear();
        return tokenize(Context, std::move(Source));
      }

      void expectKinds(const TokenizedBuffer &Buffer, std::initializer_list<TokenKind> Expected)
      {
        ASSERT_TRUE(Buffer.succeeded());
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
        ASSERT_EQ(Buffer.tokens().size(), Expected.size() + 1);
        std::size_t Index = 0;
        std::size_t PreviousEnd = 0;
        for (const TokenKind Kind : Expected)
        {
          const Token &Entry = Buffer.tokens()[Index++];
          EXPECT_EQ(Entry.Kind, Kind) << Index;
          ASSERT_TRUE(Entry.Span.isValid());
          EXPECT_GE(Entry.Span.getBegin().getByteOffset(), PreviousEnd);
          EXPECT_GT(Entry.getLength(), 0U);
          PreviousEnd = Entry.Span.getEnd().getByteOffset();
          EXPECT_EQ(Buffer.raw(Entry), std::string_view(Buffer.source()).substr(Entry.getLocation().getByteOffset(), Entry.getLength()));
        }
        const Token &End = Buffer.tokens().back();
        EXPECT_EQ(End.Kind, TokenKind::EndOfFile);
        EXPECT_EQ(End.Span, core::SourceRange::fromByteOffsets(Buffer.source().size(), Buffer.source().size()));
        EXPECT_TRUE(End.Span.empty());
        EXPECT_TRUE(Buffer.raw(End).empty());
      }

      void expectError(std::string Source, DiagnosticKind Kind)
      {
        SCOPED_TRACE(Source);
        const auto Buffer = lex(std::move(Source));
        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
        const core::Diagnostic &Entry = Diagnostics.diagnostics().front();
        EXPECT_EQ(Entry.Kind, Kind);
        EXPECT_EQ(Entry.Source, Buffer.sourceId());
        EXPECT_TRUE(Entry.Span.isValid());
        EXPECT_LE(Entry.Span.getEnd().getByteOffset(), Buffer.source().size());
        EXPECT_EQ(core::diagnosticDomain(Kind), core::DiagnosticDomain::Tokenizer);
        EXPECT_EQ(core::diagnosticDefaultSeverity(Kind), core::DiagnosticSeverity::Error);
        EXPECT_FALSE(core::DiagnosticFormatter{}.format(Entry).Message.empty());
        for (const Token &Token : Buffer.tokens())
        {
          EXPECT_NE(Token.Kind, TokenKind::EndOfFile);
        }
      }

      core::CompilationContext Compilation;
      core::FrontendContext Context;
      core::CollectingDiagnosticConsumer Diagnostics;
  };
} // namespace ink::tokenizer::test

#endif
