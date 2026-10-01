#include "ink/semantic/analyzer/analyzer.h"

#include "../core/environment_test_support.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"
#include "ink/ir/decl/module_decl.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>

namespace ink::semantic::test
{
  using namespace ink::ir;

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
    EXPECT_EQ(&First->context(), &Context.irContext());
    EXPECT_EQ(Context.namePool().text(First->name()), "main");
    EXPECT_EQ(Context.namePool().text(Second->name()), "second");
    EXPECT_EQ(First->entryBlock().outer(), First);
    EXPECT_TRUE(First->entryBlock().values().empty());
    EXPECT_TRUE(Second->entryBlock().values().empty());
    ASSERT_NE(First->declarationRoot(), nullptr);
    ASSERT_NE(Second->declarationRoot(), nullptr);
    EXPECT_EQ(&First->declarationRoot()->ast(), Empty.Unit->root());
    EXPECT_EQ(&Second->declarationRoot()->ast(), Blocks.Unit->root());
    EXPECT_EQ(&First->declarationRoot()->module(), First);
    EXPECT_EQ(&Second->declarationRoot()->module(), Second);
    EXPECT_NE(First->declarationRoot(), Second->declarationRoot());
    const ScopeStore &Scopes = Context.scopeStore();
    const Scope *FirstScope = Scopes.memberScope(*First);
    const Scope *SecondScope = Scopes.memberScope(*Second);
    ASSERT_NE(FirstScope, nullptr);
    ASSERT_NE(SecondScope, nullptr);
    EXPECT_NE(FirstScope, SecondScope);
    EXPECT_EQ(FirstScope->parent(), &Scopes.rootScope());
    EXPECT_EQ(SecondScope->parent(), &Scopes.rootScope());
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
        {"x = 1;", "AssignmentItem"},
        {"if (true) {}", "IfStmt"},
        {"while (true) {}", "WhileStmt"},
        {"for (;;) {}", "ClassicForStmt"},
        {"for (Item in Items) {}", "ForInStmt"},
        {"switch (X) {}", "SwitchStmt"},
        {"break;", "BreakStmt"},
        {"continue;", "ContinueStmt"},
        {"yield 1;", "YieldStmt"},
        {"defer {}", "DeferStmt"},
        {"import Foo;", "DirectImportStmt"},
        {"from Foo import Bar;", "FromImportStmt"},
        {"var X = 1;", "VarDecl"},
        {"field X: i32;", "FieldDecl"},
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
      EXPECT_DEATH(Analysis.analyze(Context, Parsed), std::string("initial semantic analyzer does not support ") + Entry.Kind);
    }
  }

  // Comptime flags reject otherwise supported syntax before blocks, expressions, types or call fast paths lower runtime IR.
  TEST(SemanticAnalyzerTest, RejectsComptimeBeforeRuntimeLowering)
  {
    struct Case
    {
        const char *Source;
        const char *Kind;
    };
    constexpr Case Cases[] = {
        {"comptime {}", "BlockStmt"},
        {"comptime func F(): void;", "DeclStmt"},
        {"comptime 1;", "LiteralExpr"},
        {"func F(): i32 { return comptime 1; }", "LiteralExpr"},
        {"func F(X: comptime i32): void;", "NameExpr"},
        {"func F(): comptime i32;", "NameExpr"},
        {"func F(X: i32): void; (comptime (F))(1);", "ParenExpr"},
        {"func F(X: i32): void; func F(X: i64): void; (comptime F)(1);", "NameExpr"},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      core::CompilationContext Compilation;
      core::FrontendContext Frontend(Compilation);
      auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, Entry.Source));
      ASSERT_TRUE(Parsed.succeeded());
      SemanticContext Context(Compilation);
      EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed), std::string("initial semantic analyzer does not support ") + Entry.Kind);
    }
  }

  // Recoverable type errors in nested blocks do not stop sibling diagnostics or contaminate the next analysis call.
  TEST(SemanticAnalyzerTest, ContinuesAfterErrorsAndRestoresBlockState)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Broken = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{ func first(x: Missing): void; { func second(x: Missing): void; } } func third(x: Missing): void;"));
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

  // A deeply nested error restores depth and scope before another deep sibling and a module-level function.
  TEST(SemanticAnalyzerTest, RestoresGuardsAfterNestedError)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    std::string Source(200, '{');
    Source.append("func invalid(x: Missing): void;");
    Source.append(200, '}');
    Source.append(200, '{');
    Source.append(200, '}');
    Source.append("func after(): void;");
    parser::ParseLimits Limits;
    Limits.MaxNestingDepth = 1024;
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, std::move(Source)), Limits);
    ASSERT_TRUE(Parsed.succeeded());
    SemanticContext Context(Compilation);
    core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    EXPECT_EQ(Analyzer{}.analyze(Context, Parsed), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownType);
    EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostics.diagnostics()[0]).Message, "unknown type 'Missing'");
    ASSERT_EQ(Context.modules().size(), 1U);
    Module &Result = *Context.modules()[0];
    ASSERT_EQ(Result.entryBlock().values().size(), 1U);
    NameResolver Resolver(Context);
    const auto *Binding = Resolver.lookupMember(Result, Context.namePool().find("after"));
    ASSERT_NE(Binding, nullptr);
    ASSERT_EQ(Binding->targets().size(), 1U);
    EXPECT_EQ(Binding->targets()[0], Result.entryBlock().values()[0].get());
  }

  // The environment limit accepts its boundary, rejects deeper blocks, and can change between analyses.
  TEST(SemanticAnalyzerTest, UsesConfiguredBlockDepthLimit)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_BLOCK_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set("2"));
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto AtLimit = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{{}} {{}}"));
    auto OverLimit = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{{{}}} {{}}"));
    ASSERT_TRUE(AtLimit.succeeded());
    ASSERT_TRUE(OverLimit.succeeded());
    SemanticContext Context(Compilation);
    core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    Analyzer Analysis;
    EXPECT_NE(Analysis.analyze(Context, AtLimit), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_DEATH(Analysis.analyze(Context, OverLimit), "internal compiler error\\[INK-S0014\\]");
    ASSERT_TRUE(Environment.set("3"));
    EXPECT_NE(Analysis.analyze(Context, OverLimit), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    ASSERT_TRUE(Environment.set("invalid"));
    EXPECT_NE(Analysis.analyze(Context, OverLimit), nullptr);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A zero block limit rejects the first block while still allowing an empty module.
  TEST(SemanticAnalyzerTest, ZeroBlockDepthLimitRejectsAnyBlock)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_BLOCK_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set("0"));
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Empty = parser::parse(Frontend, tokenizer::tokenize(Frontend, ""));
    auto Block = parser::parse(Frontend, tokenizer::tokenize(Frontend, "{}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Block.succeeded());
    SemanticContext Context(Compilation);
    core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    Analyzer Analysis;
    EXPECT_NE(Analysis.analyze(Context, Empty), nullptr);
    EXPECT_DEATH(Analysis.analyze(Context, Block), "internal compiler error\\[INK-S0014\\]");
  }

  // An invalid empty module name prints a construction ICE and terminates immediately.
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
    EXPECT_DEATH(Analysis.analyze(Context, Parsed, ""), "internal compiler error\\[INK-S0013\\]");
  }
} // namespace ink::semantic::test
