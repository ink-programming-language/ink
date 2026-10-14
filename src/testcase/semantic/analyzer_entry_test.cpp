#include "ink/semantic/analyzer/analyzer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"
#include "ink/semantic/context.h"
#include "ink/frontend/parser/parser.h"
#include "../core/support/environment_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  // Explicit object accessors preserve mutable/const service and IR references, including calls on rvalues.
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().diagnosticEngine()), core::DiagnosticEngine &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().diagnosticEngine()), const core::DiagnosticEngine &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().sourceManager()), core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().sourceManager()), const core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().typePool()), ir::TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().typePool()), const ir::TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &&>().typePool()), ir::TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &&>().typePool()), const ir::TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().module()), ir::Module &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().module()), const ir::Module &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().irBuilder()), ir::IRBuilder &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().irBuilder()), const ir::IRBuilder &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &&>().sourceManager()), core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &&>().sourceManager()), const core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &&>().module()), ir::Module &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &&>().module()), const ir::Module &>);

  // The public entry reaches nested block dispatch and keeps successive calls independent.
  TEST(SemanticAnalyzerEntryTest, TraversesEmptyModulesAndNestedBlocks)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    Analyzer Analysis;
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    auto Blocks = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{ {} } {}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Blocks.succeeded());
    EXPECT_TRUE(Analysis.analyze(Context, Empty));
    EXPECT_TRUE(Analysis.analyze(Context, Blocks));
    EXPECT_TRUE(Analysis.analyze(Context, Empty));
  }

  // Context-owned IR retains its target layout, types and constants after the analyzer and parsed input expire.
  TEST(SemanticAnalyzerEntryTest, OwnsTargetSpecificIrBeyondAnalysisLifetime)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    constexpr target::TargetContext Target(target::PointerWidth::Bits32, target::ByteOrder::BigEndian);
    SemanticContext Context(Sources, DiagnosticEngine, Target);
    ir::ReturnInstruction *Return = nullptr;
    {
      frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
      Analyzer Analysis;
      auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{}"));
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Analysis.analyze(Context, Parsed));
      auto *Pointer = Context.typePool().getPointerType(Context.typePool().getInt32Type());
      ASSERT_NE(Pointer, nullptr);
      auto *Global = Context.module().createGlobalVariable(*Pointer);
      ASSERT_NE(Global, nullptr);
      const auto *Null = Context.module().constantPool().getNullPointerConstant();
      ASSERT_TRUE(Global->setInitializer(Null));
      auto *Function = Context.module().createGlobalFunction(*Context.typePool().getFunctionType(Pointer));
      ASSERT_NE(Function, nullptr);
      auto *Block = Function->createBlock();
      ASSERT_TRUE(Context.irBuilder().setInsertPoint(*Block));
      Return = Context.irBuilder().createReturn(*Null);
      ASSERT_NE(Return, nullptr);
    }
    const SemanticContext &ReadOnly = Context;
    EXPECT_EQ(ReadOnly.module().targetContext(), Target);
    EXPECT_EQ(&ReadOnly.typePool(), &ReadOnly.module().typePool());
    EXPECT_EQ(&ReadOnly.typePool().module(), &ReadOnly.module());
    EXPECT_EQ(&ReadOnly.irBuilder().module(), &ReadOnly.module());
    EXPECT_EQ(&ReadOnly.irBuilder().typePool(), &ReadOnly.typePool());
    EXPECT_EQ(&ReadOnly.irBuilder().constantPool(), &ReadOnly.module().constantPool());
    ASSERT_EQ(ReadOnly.module().globalVariableCount(), 1U);
    ASSERT_EQ(ReadOnly.module().globalFunctionCount(), 1U);
    const auto *Global = ReadOnly.module().globalVariable(0);
    const auto *Function = ReadOnly.module().globalFunction(0);
    EXPECT_EQ(Global->type()->size(), 4U);
    EXPECT_EQ(Global->type()->alignment(), 4U);
    EXPECT_EQ(Function->functionType()->returnType(), Global->type());
    EXPECT_TRUE(ReadOnly.module().ownsType(Global->type()));
    EXPECT_TRUE(ReadOnly.module().constantPool().owns(Global->initializer()));
    EXPECT_EQ(Function->entryBlock()->instruction(0), Return);
    EXPECT_EQ(Return->returnedValue(), Global->initializer());
    EXPECT_EQ(Return->type(), ReadOnly.typePool().getVoidType());
  }

  // Accepted analyses preserve existing module declarations and pooled constants while resetting the insertion point.
  TEST(SemanticAnalyzerEntryTest, PreservesOwnedIrAcrossAcceptedInputs)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    Analyzer Analysis;
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    auto Blocks = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{{}} {}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Blocks.succeeded());
    auto &Types = Context.typePool();
    const auto *Integer = Context.module().constantPool().getSignedIntegerConstant(Types.getInt32Type(), 42);
    ASSERT_NE(Integer, nullptr);
    auto *Global = Context.module().createGlobalVariable(*Types.getInt32Type());
    ASSERT_NE(Global, nullptr);
    ASSERT_TRUE(Global->setInitializer(Integer));
    auto *Function = Context.module().createGlobalFunction(*Types.getFunctionType(Types.getInt32Type()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    const frontend::parser::ParseResult *Inputs[] = {&Empty, &Blocks, &Empty};
    for (const auto *Input : Inputs)
    {
      ASSERT_TRUE(Context.irBuilder().setInsertPoint(*Block));
      ASSERT_TRUE(Analysis.analyze(Context, *Input));
      EXPECT_EQ(Context.irBuilder().insertBlock(), nullptr);
      EXPECT_EQ(Context.irBuilder().insertFunction(), nullptr);
      EXPECT_EQ(Context.module().globalVariableCount(), 1U);
      EXPECT_EQ(Context.module().globalFunctionCount(), 1U);
      EXPECT_EQ(Context.module().globalVariable(0), Global);
      EXPECT_EQ(Context.module().globalFunction(0), Function);
      EXPECT_EQ(Global->initializer(), Integer);
      EXPECT_EQ(Context.module().constantPool().getSignedIntegerConstant(Types.getInt32Type(), 42), Integer);
      EXPECT_EQ(Block->instructionCount(), 0U);
    }
    ASSERT_TRUE(Context.irBuilder().setInsertPoint(*Block));
    auto *Return = Context.irBuilder().createReturn(*Integer);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Integer);
  }

  // Missing, erroneous, cancelled and limited parser results never enter semantic dispatch.
  TEST(SemanticAnalyzerEntryTest, RejectsUnusableParseResults)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
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
    auto *Function = Context.module().createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_TRUE(Context.irBuilder().setInsertPoint(*Block));
    const frontend::parser::ParseResult *Inputs[] = {&Missing, &Broken, &Cancelled, &Limited};
    for (const auto *Input : Inputs)
    {
      EXPECT_FALSE(Analysis.analyze(Context, *Input));
      EXPECT_EQ(Context.irBuilder().insertBlock(), Block);
      EXPECT_EQ(Context.irBuilder().insertFunction(), Function);
      EXPECT_EQ(Context.module().globalFunctionCount(), 1U);
      EXPECT_EQ(Context.module().globalFunction(0), Function);
      EXPECT_EQ(Block->instructionCount(), 0U);
    }
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
    SemanticContext Context(OtherSources, OtherDiagnosticEngine, target::TargetContext::native());
    auto *Function = Context.module().createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_TRUE(Context.irBuilder().setInsertPoint(*Block));
    Analyzer Analysis;
    EXPECT_FALSE(Analysis.analyze(Context, Parsed));
    EXPECT_EQ(Context.irBuilder().insertBlock(), Block);
    EXPECT_EQ(Context.irBuilder().insertFunction(), Function);
    EXPECT_EQ(Context.module().globalFunctionCount(), 1U);
    EXPECT_EQ(Context.module().globalFunction(0), Function);
    EXPECT_TRUE(Analysis.analyze(Context, OtherParsed));
    EXPECT_EQ(Context.irBuilder().insertBlock(), nullptr);
    EXPECT_EQ(Context.module().globalFunction(0), Function);
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
      SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
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
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
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
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed), "initial semantic analyzer does not support BlockStmt");
  }
} // namespace ink::semantic::test
