#pragma once
#include "ink/parser/parser.h"
#include "ink/parser/ast_walker.h"
#include "ink/parser/ast_visitor.h"
#include <gtest/gtest.h>
namespace ink::parser::test
{
  class ParserTest : public testing::Test
  {
    protected:
      ParserTest()
          : Frontend(Compilation)
      {
        Compilation.diagnosticEngine().addConsumer(Diagnostics);
      }
      ParseResult read(std::string Source, ParseLimits Limits = {})
      {
        Diagnostics.clear();
        auto Result = parse(Frontend, tokenizer::tokenize(Frontend, std::move(Source)), std::move(Limits));
        std::string Reason;
        EXPECT_TRUE(verifyAST(Result.Unit->root(), Result.Unit->input().lexedFile().source().size(), &Reason)) << Reason << '\n'
                                                                                                               << dumpAST(Result.Unit->root());
        return Result;
      }
      const Expr *expression(const ParseResult &Result, std::size_t Index = 0)
      {
        return cast<ExprItem>(cast<SimpleStmt>(Result.Unit->root()->statements()[Index])->items()[0])->expression();
      }
      const Decl *declaration(const ParseResult &Result, std::size_t Index = 0)
      {
        return cast<DeclStmt>(Result.Unit->root()->statements()[Index])->declaration();
      }
      core::CompilationContext Compilation;
      core::FrontendContext Frontend;
      core::CollectingDiagnosticConsumer Diagnostics;
  };
} // namespace ink::parser::test
