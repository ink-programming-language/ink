#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/frontend/parser/parser.h"
#include "../core/environment_test_support.h"

#include <gtest/gtest.h>

#include <string>

namespace ink::semantic::test
{
  // The public entry reaches nested block dispatch and keeps successive calls independent.
  TEST(SemanticAnalyzerEntryTest, TraversesEmptyModulesAndNestedBlocks)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine);
    Analyzer Analysis;
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    auto Blocks = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{ {} } {}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Blocks.succeeded());
    EXPECT_TRUE(Analysis.analyze(Context, Empty));
    EXPECT_TRUE(Analysis.analyze(Context, Blocks));
    EXPECT_TRUE(Analysis.analyze(Context, Empty));
  }

  // Missing, erroneous, cancelled and limited parser results never enter semantic dispatch.
  TEST(SemanticAnalyzerEntryTest, RejectsUnusableParseResults)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine);
    Analyzer Analysis;
    frontend::parser::ParseResult Missing;
    auto Broken = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "var = ;"));
    auto Cancelled = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    auto Limited = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    ASSERT_FALSE(Broken.succeeded());
    ASSERT_TRUE(Cancelled.succeeded());
    ASSERT_TRUE(Limited.succeeded());
    Cancelled.Status = frontend::parser::ParseStatus::Cancelled;
    Limited.Status = frontend::parser::ParseStatus::LimitExceeded;
    EXPECT_FALSE(Analysis.analyze(Context, Missing));
    EXPECT_FALSE(Analysis.analyze(Context, Broken));
    EXPECT_FALSE(Analysis.analyze(Context, Cancelled));
    EXPECT_FALSE(Analysis.analyze(Context, Limited));
  }

  // Input from another compilation is rejected while input owned by the same context remains usable.
  TEST(SemanticAnalyzerEntryTest, RejectsForeignSourceContext)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    core::SourceManager OtherSources;
    core::DiagnosticEngine OtherDiagnosticEngine;
    frontend::FrontendContext OtherFrontend(OtherSources, OtherDiagnosticEngine);
    auto OtherParsed = frontend::parser::parse(OtherFrontend, frontend::tokenizer::tokenize(OtherFrontend, ""));
    ASSERT_TRUE(Parsed.succeeded());
    ASSERT_TRUE(OtherParsed.succeeded());
    SemanticContext Context(OtherSources, OtherDiagnosticEngine);
    Analyzer Analysis;
    EXPECT_FALSE(Analysis.analyze(Context, Parsed));
    EXPECT_TRUE(Analysis.analyze(Context, OtherParsed));
  }

  // Statement and declaration dispatch report the concrete unsupported node reached through the public entry.
  TEST(SemanticAnalyzerEntryDeathTest, ReachesConcreteStatementAndDeclarationHandlers)
  {
    struct Case
    {
        const char *Source;
        const char *Kind;
    };
    constexpr Case Cases[] = {
        {"value;", "SimpleStmt"},
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
        {"import library;", "DirectImportStmt"},
        {"from library import value;", "FromImportStmt"},
        {"{ var Value = 1; }", "VarDecl"},
        {"property Value: i32;", "FieldDecl"},
        {"{{ func F(): void {} }}", "FunctionDecl"},
        {"class Value {};", "ClassDecl"},
        {"enum E {};", "EnumDecl"},
        {"interface I {};", "InterfaceDecl"},
    };
    Analyzer Analysis;
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      core::SourceManager Sources;
      core::DiagnosticEngine DiagnosticEngine;
      frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
      auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, Entry.Source));
      ASSERT_TRUE(Parsed.succeeded());
      SemanticContext Context(Sources, DiagnosticEngine);
      EXPECT_DEATH(Analysis.analyze(Context, Parsed), std::string("initial semantic analyzer does not support ") + Entry.Kind);
    }
  }

  // Nested traversal enforces its budget and restores depth before siblings and subsequent calls.
  TEST(SemanticAnalyzerEntryDeathTest, BoundsBlockTraversalAndRestoresDepth)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_BLOCK_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("2"));
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine);
    Analyzer Analysis;
    auto Siblings = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{{}} {{}}"));
    auto TooDeep = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{{{}}}"));
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Siblings.succeeded());
    ASSERT_TRUE(TooDeep.succeeded());
    ASSERT_TRUE(Empty.succeeded());
    EXPECT_TRUE(Analysis.analyze(Context, Siblings));
    EXPECT_DEATH(Analysis.analyze(Context, TooDeep), "INK-S0014");
    EXPECT_TRUE(Analysis.analyze(Context, Siblings));
    ASSERT_TRUE(Limit.set("0"));
    EXPECT_TRUE(Analysis.analyze(Context, Empty));
    EXPECT_DEATH(Analysis.analyze(Context, Siblings), "INK-S0014");
  }

  // A comptime block cannot be accepted as an ordinary empty block while evaluation remains absent.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsComptimeAsUnimplemented)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "comptime {}"));
    ASSERT_TRUE(Parsed.succeeded());
    SemanticContext Context(Sources, DiagnosticEngine);
    EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed), "initial semantic analyzer does not support BlockStmt");
  }
} // namespace ink::semantic::test
