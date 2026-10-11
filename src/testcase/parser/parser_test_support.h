#pragma once
#include "ink/frontend/context.h"
#include "ink/frontend/parser/parser.h"
#include "ink/frontend/parser/ast_walker.h"
#include "ink/frontend/parser/ast_visitor.h"
#include <gtest/gtest.h>
namespace ink::frontend::parser::test
{
  class ParserTest : public testing::Test
  {
    protected:
      ParserTest()
          : Frontend(Sources, DiagnosticEngine)
      {
        DiagnosticEngine.addConsumer(Diagnostics);
      }
      ParseResult read(std::string Source, ParseLimits Limits = {})
      {
        Diagnostics.clear();
        auto Result = parse(Frontend, tokenizer::tokenize(Frontend, std::move(Source)), std::move(Limits));
        std::string Reason;
        EXPECT_TRUE(verifyAST(Result.Unit->root(), Result.Unit->input().lexedFile().source().size(), &Reason)) << Reason << '\n'
                                                                                                               << dumpAST(Result.Unit->root());
        expectValidMetadata(Result);
        return Result;
      }
      void expectValidMetadata(const ParseResult &Result)
      {
        const auto &Input = Result.Unit->input();
        const auto &Source = Input.lexedFile();
        const auto CheckRange = [&](SourceRange Range)
        {
          EXPECT_TRUE(Range.isValid());
          EXPECT_LE(Range.getBegin().getByteOffset(), Range.getEnd().getByteOffset());
          EXPECT_LE(Range.getEnd().getByteOffset(), Source.source().size());
        };
        for (const auto &Diagnostic : Diagnostics.diagnostics())
        {
          EXPECT_EQ(Diagnostic.Source, Source.sourceId());
          EXPECT_EQ(Diagnostic.classification(), Diagnostic.Kind == core::DiagnosticKind::ParserLimitExceeded ? core::DiagnosticClass::InternalCompilerError : core::DiagnosticClass::User);
          CheckRange(Diagnostic.Span);
          EXPECT_FALSE(core::DiagnosticFormatter{}.format(Diagnostic).Message.empty());
        }
        for (const auto &Entry : Result.Unit->recoveryInfo().Entries)
        {
          EXPECT_EQ(Entry.Node, Result.Unit->root());
          CheckRange(Entry.Token.Range);
          EXPECT_NE(Entry.Token.Status, ExpectStatus::Matched);
          if (Entry.Token.Status == ExpectStatus::Inserted)
          {
            EXPECT_FALSE(Entry.Token.Actual.has_value());
            EXPECT_TRUE(Entry.Token.Range.empty());
          }
          else
          {
            ASSERT_TRUE(Entry.Token.Actual.has_value());
            ASSERT_LT(*Entry.Token.Actual, Input.size());
            EXPECT_EQ(Input.token(*Entry.Token.Actual).Kind, Entry.Token.Expected);
            EXPECT_EQ(Input.token(*Entry.Token.Actual).Span, Entry.Token.Range);
          }
          if (Entry.Skipped)
          {
            CheckRange(*Entry.Skipped);
            // Interrupted recovery can stop before consuming any skipped token.
            if (Entry.Skipped->empty())
            {
              EXPECT_NE(Result.Status, ParseStatus::Completed);
              EXPECT_EQ(Entry.Token.Status, ExpectStatus::Inserted);
            }
            if (Entry.Token.Actual)
            {
              EXPECT_LE(Entry.Skipped->getEnd().getByteOffset(), Entry.Token.Range.getBegin().getByteOffset());
            }
          }
        }
      }
      const Expr *expression(const ParseResult &Result, std::size_t Index = 0)
      {
        return cast<ExprItem>(cast<SimpleStmt>(Result.Unit->root()->statements()[Index])->items()[0])->expression();
      }
      const Decl *declaration(const ParseResult &Result, std::size_t Index = 0)
      {
        return cast<DeclStmt>(Result.Unit->root()->statements()[Index])->declaration();
      }
      core::SourceManager Sources;
      core::DiagnosticEngine DiagnosticEngine;
      frontend::FrontendContext Frontend;
      core::CollectingDiagnosticConsumer Diagnostics;
  };
} // namespace ink::frontend::parser::test
