#include "ink/semantic/analyze/analyzer.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <string>

namespace ink::semantic::test
{
  // Empty modules and nested empty blocks produce distinct context-owned modules across repeated calls.
  TEST(SemanticAnalyzerTest, CreatesModulesAndKeepsCallsIndependent)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Empty = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    auto Blocks = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{ {} } {}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Blocks.succeeded());
    SemanticContext Context(Compilation);
    Analyzer Analysis;
    Module *First = Analysis.analyze(Context, Empty);
    Module *Second = Analysis.analyze(Context, Blocks, "second");
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(&First->context(), &Context);
    EXPECT_EQ(Context.namePool().text(First->name()), "main");
    EXPECT_EQ(Context.namePool().text(Second->name()), "second");
    EXPECT_EQ(First->entryBlock().outer(), First);
    EXPECT_TRUE(First->entryBlock().values().empty());
    EXPECT_TRUE(Second->entryBlock().values().empty());
  }

  // Missing, erroneous, cancelled and incomplete parse inputs fail before allocating a module name.
  TEST(SemanticAnalyzerTest, RejectsUnusableParseResults)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Broken = parser::parse(Frontend, tokenizer::tokenize(Frontend, "var = ;"));
    auto Cancelled = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    auto Limited = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    ASSERT_FALSE(Broken.succeeded());
    Cancelled.Status = parser::ParseStatus::Cancelled;
    Limited.Status = parser::ParseStatus::LimitExceeded;
    SemanticContext Context(Compilation);
    Analyzer Analysis;
    parser::ParseResult Missing;
    EXPECT_EQ(Analysis.analyze(Context, Missing), nullptr);
    EXPECT_EQ(Analysis.analyze(Context, Broken), nullptr);
    EXPECT_EQ(Analysis.analyze(Context, Cancelled), nullptr);
    EXPECT_EQ(Analysis.analyze(Context, Limited), nullptr);
    EXPECT_EQ(Context.namePool().size(), 0U);
  }

  // Source ownership is checked even when two compilation contexts assign equal numeric source IDs.
  TEST(SemanticAnalyzerTest, RejectsForeignSourceContext)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Parsed.succeeded());
    core::CompilationContext OtherCompilation;
    core::FrontendContext OtherFrontend(OtherCompilation);
    auto OtherParsed = parser::parse(OtherFrontend, tokenizer::tokenize(OtherFrontend, ""));
    ASSERT_TRUE(OtherParsed.succeeded());
    SemanticContext Context(OtherCompilation);
    Analyzer Analysis;
    EXPECT_EQ(Analysis.analyze(Context, Parsed), nullptr);
    EXPECT_EQ(Context.namePool().size(), 0U);
    EXPECT_NE(Analysis.analyze(Context, OtherParsed), nullptr);
  }

  // Each currently unsupported concrete statement/declaration reports its own kind rather than silently accepting it.
  TEST(SemanticAnalyzerTest, DispatchesUnsupportedSyntaxToConcreteHandlers)
  {
    struct Case
    {
        const char *Source;
        const char *Kind;
    };
    constexpr Case Cases[] = {
        {"1;", "SimpleStmt"},
        {"if (true) {}", "IfStmt"},
        {"while (true) {}", "WhileStmt"},
        {"for (;;) {}", "ClassicForStmt"},
        {"for (Item in Items) {}", "ForInStmt"},
        {"switch (X) {}", "SwitchStmt"},
        {"return;", "ReturnStmt"},
        {"break;", "BreakStmt"},
        {"continue;", "ContinueStmt"},
        {"yield 1;", "YieldStmt"},
        {"defer {}", "DeferStmt"},
        {"comptime {}", "ComptimeStmt"},
        {"import Foo;", "DirectImportStmt"},
        {"from Foo import Bar;", "FromImportStmt"},
        {"var X = 1;", "VarDecl"},
        {"field X: i32;", "FieldDecl"},
        {"func F(): void {}", "FunctionDecl"},
        {"func F[T: type](X: T): T { return X; }", "FunctionDecl"},
        {"class C {};", "ClassDecl"},
        {"class C[T: type] {};", "ClassDecl"},
        {"enum E {};", "EnumDecl"},
        {"interface I {};", "InterfaceDecl"},
    };
    Analyzer Analysis;
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      core::CompilationContext Compilation;
      core::FrontendContext Frontend(Compilation);
      auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, Entry.Source));
      ASSERT_TRUE(Parsed.succeeded());
      SemanticContext Context(Compilation);
      core::CollectingDiagnosticConsumer Diagnostics;
      Compilation.diagnosticEngine().addConsumer(Diagnostics);
      EXPECT_EQ(Analysis.analyze(Context, Parsed), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      const auto &Diagnostic = Diagnostics.diagnostics()[0];
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticUnsupported);
      EXPECT_EQ(Diagnostic.Source, Parsed.Unit->input().lexedFile().sourceId());
      const auto *Stmt = Parsed.Unit->root()->statements()[0];
      const parser::ASTNodeBase *ExpectedNode = parser::DeclStmt::classof(Stmt) ? static_cast<const parser::DeclStmt *>(Stmt)->declaration() : static_cast<const parser::ASTNodeBase *>(Stmt);
      EXPECT_EQ(Diagnostic.Span, ExpectedNode->getSourceRange());
      EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostic).Message, std::string("initial semantic analyzer does not support ") + Entry.Kind);
    }
  }

  // Failure in a nested block does not stop sibling diagnostics or contaminate the next analysis call.
  TEST(SemanticAnalyzerTest, ContinuesAfterErrorsAndRestoresBlockState)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Broken = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{ 1; { return; } } var X = 2;"));
    auto Empty = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{}"));
    ASSERT_TRUE(Broken.succeeded());
    ASSERT_TRUE(Empty.succeeded());
    SemanticContext Context(Compilation);
    core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    Analyzer Analysis;
    EXPECT_EQ(Analysis.analyze(Context, Broken), nullptr);
    EXPECT_EQ(Diagnostics.diagnostics().size(), 3U);
    Diagnostics.clear();
    EXPECT_NE(Analysis.analyze(Context, Empty), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // An invalid empty module name reports a construction failure with the input source identity.
  TEST(SemanticAnalyzerTest, ReportsInvalidModuleName)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Parsed.succeeded());
    SemanticContext Context(Compilation);
    core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    Analyzer Analysis;
    EXPECT_EQ(Analysis.analyze(Context, Parsed, ""), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticConstructionFailed);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Source, Parsed.Unit->input().lexedFile().sourceId());
  }
} // namespace ink::semantic::test
