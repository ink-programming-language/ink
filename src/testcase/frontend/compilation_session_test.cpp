#include "ink/frontend/compilation_session.h"
#include "ink/ast/verifier.h"

#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>
#include <string>

namespace ink::frontend
{
  namespace
  {
    // Verifies that a session assigns stable file IDs and retains its source, string, canonical type, and target contexts.
    TEST(CompilationSessionTest, OwnsStableSourceAndStringContexts)
    {
      CompilationSession Session;
      const core::SourceFileId First = Session.addSource("first.ink", "const First = 1;");
      const core::SourceFile &FirstBeforeSecondInsert = Session.sourceManager().sourceFile(First);
      const core::SourceFileId Second = Session.addSource("second.ink", "const Second = 2;");
      const core::InternedStringId Name = Session.stringInterner().intern("First");

      EXPECT_EQ(First.value(), 0U);
      EXPECT_EQ(Second.value(), 1U);
      EXPECT_EQ(&Session.sourceManager().sourceFile(First), &FirstBeforeSecondInsert);
      EXPECT_EQ(Session.sourceManager().sourceFile(First).Contents, "const First = 1;");
      EXPECT_EQ(Session.sourceManager().sourceFile(Second).Path, "second.ink");
      EXPECT_EQ(Session.stringInterner().string(Name), "First");
      EXPECT_EQ(Session.typeContext().type(Session.typeContext().i32Type()).Kind, type::TypeKind::I32);
      EXPECT_TRUE(Session.targetContext().key().isValid());
    }

    // Verifies that token buffers and tokenizer diagnostics retain the source file selected by the session.
    TEST(CompilationSessionTest, TokenizesWithStableFileOwnership)
    {
      CompilationSession Session;
      const core::SourceFileId CleanFile = Session.addSource("clean.ink", "const Value = 1;");
      const core::SourceFileId BrokenFile = Session.addSource("broken.ink", "?");

      const tokenizer::TokenizedBuffer Clean = Session.tokenize(CleanFile);
      const tokenizer::TokenizedBuffer Broken = Session.tokenize(BrokenFile);

      EXPECT_EQ(Clean.sourceFileId(), CleanFile);
      EXPECT_TRUE(Clean.succeeded());
      EXPECT_EQ(Broken.sourceFileId(), BrokenFile);
      ASSERT_FALSE(Broken.diagnostics().empty());
      EXPECT_EQ(Broken.diagnostics().front().File, BrokenFile);
      ASSERT_EQ(Session.diagnostics().size(), Broken.diagnostics().size());
      EXPECT_EQ(Session.diagnostics().front().File, BrokenFile);
    }

    // Verifies that parser diagnostics and parsed CST ownership use the same file ID as their tokenized input.
    TEST(CompilationSessionTest, ParsesWithStableFileOwnership)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("parse-error.ink", "const Value = ;");

      const std::optional<parser::ParsedFile> Parsed = Session.parse(File);

      ASSERT_TRUE(Parsed.has_value());
      EXPECT_EQ(Parsed->sourceFileId(), File);
      ASSERT_FALSE(Parsed->diagnostics().empty());
      for (const core::Diagnostic &Diagnostic : Parsed->diagnostics())
      {
        EXPECT_EQ(Diagnostic.File, File);
      }
      EXPECT_EQ(Session.diagnostics(), Parsed->diagnostics());
    }

    // Verifies that lexical failure stops parsing while preserving the tokenizer diagnostic in the session.
    TEST(CompilationSessionTest, StopsBeforeParsingLexicallyInvalidSource)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("lex-error.ink", "?");

      const std::optional<parser::ParsedFile> Parsed = Session.parse(File);

      EXPECT_FALSE(Parsed.has_value());
      ASSERT_FALSE(Session.diagnostics().empty());
      EXPECT_EQ(Session.diagnostics().front().File, File);
      EXPECT_EQ(Session.diagnostics().front().Kind, core::DiagnosticKind::InvalidCharacter);
      Session.clearDiagnostics();
      EXPECT_TRUE(Session.diagnostics().empty());
    }

    // Verifies that neither the low-level tokenizer nor a session accepts an invalid source file ID.
    TEST(CompilationSessionTest, RejectsInvalidSourceFileIds)
    {
      CompilationSession Session;

      EXPECT_THROW(tokenizer::tokenize({}, ""), std::invalid_argument);
      EXPECT_THROW(Session.tokenize({}), std::out_of_range);
      EXPECT_THROW(Session.parse({}), std::out_of_range);
    }

    // Verifies the combined session entry point owns the AST arena and returns a file that passes structural verification.
    TEST(CompilationSessionTest, ParsesAndLowersIntoOwnedAstContext)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("ast.ink", "const Value = 1;");

      const std::optional<ast::AstFile> AstFile = Session.parseAndLower(File);

      ASSERT_TRUE(AstFile.has_value());
      EXPECT_EQ(AstFile->sourceFile(), File);
      EXPECT_TRUE(ast::verifyAst(Session.astContext(), *AstFile, Session.stringInterner()).succeeded());
    }

    // Verifies parseAndLower retains parser diagnostics while returning a structurally valid recovery AST for lexically valid source.
    TEST(CompilationSessionTest, ParsesAndLowersRecoverableParserErrors)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("recovery.ink", "const Value = ;");

      const std::optional<ast::AstFile> AstFile = Session.parseAndLower(File);

      ASSERT_TRUE(AstFile.has_value());
      EXPECT_EQ(AstFile->sourceFile(), File);
      EXPECT_FALSE(Session.diagnostics().empty());
      EXPECT_TRUE(ast::verifyAst(Session.astContext(), *AstFile, Session.stringInterner()).succeeded());
    }

    // Verifies semantic analysis uses the session-owned canonical TypeContext and seals a valid module for downstream IR lowering.
    TEST(CompilationSessionTest, AnalyzesIntoVerifiedSemanticModule)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("semantic.ink", "func identity(Value: i32) -> i32 { return Value; }");
      const std::optional<ast::AstFile> AstFile = Session.parseAndLower(File);
      ASSERT_TRUE(AstFile.has_value());

      sema::SemanticAnalysisResult Analysis = Session.analyze(*AstFile);

      EXPECT_TRUE(Analysis.succeeded());
      ASSERT_TRUE(Analysis.Module.has_value());
      EXPECT_EQ(Analysis.Module->sourceFile(), File);
      EXPECT_TRUE(Session.diagnostics().empty());
    }

    // Verifies semantic failures retain their stable file/range diagnostics in the owning compilation session and do not forge a verified module.
    TEST(CompilationSessionTest, RetainsSemanticDiagnosticsWithoutSealingInvalidInput)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("semantic-error.ink", "func broken() -> i32 { return Missing; }");
      const std::optional<ast::AstFile> AstFile = Session.parseAndLower(File);
      ASSERT_TRUE(AstFile.has_value());

      sema::SemanticAnalysisResult Analysis = Session.analyze(*AstFile);

      EXPECT_FALSE(Analysis.succeeded());
      EXPECT_FALSE(Analysis.Module.has_value());
      ASSERT_FALSE(Analysis.Diagnostics.empty());
      EXPECT_EQ(Analysis.Diagnostics.front().Kind, core::DiagnosticKind::UnresolvedName);
      EXPECT_EQ(Analysis.Diagnostics.front().File, File);
      EXPECT_EQ(Session.diagnostics(), Analysis.Diagnostics);
    }
  } // namespace
} // namespace ink::frontend
