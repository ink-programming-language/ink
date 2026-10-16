#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
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
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().irContext()), ir::IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().irContext()), const ir::IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &&>().sourceManager()), core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &&>().sourceManager()), const core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &&>().irContext()), ir::IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &&>().irContext()), const ir::IRContext &>);

  // Declaration collection skips expression statements, and full analysis accepts the implemented literal/name forms.
  TEST(SemanticAnalyzerEntryTest, AnalyzesLiteralAndBuiltinBooleanStatements)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "0; 1; 14; \"Hello, world!\\n\"; true; false;"));
    ASSERT_TRUE(Input.succeeded());
    EXPECT_TRUE(Analyzer{}.analyze(Context, Input, Output));
    EXPECT_EQ(Output.globalFunctionCount(), 0U);
    EXPECT_EQ(Output.globalVariableCount(), 0U);
  }

  // Unknown expression names are diagnosed once each during full analysis, including following sibling statements.
  TEST(SemanticAnalyzerEntryTest, ReportsUnknownExpressionNamesDuringFullAnalysis)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "Missing; Other;"));
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(Analyzer{}.analyze(Context, Input, Output));
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    EXPECT_EQ(Consumer.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(Consumer.diagnostics()[1].Kind, core::DiagnosticKind::SemanticUnknownName);
  }

  // The public entry reaches nested block dispatch and keeps successive calls independent.
  TEST(SemanticAnalyzerEntryTest, TraversesEmptyModulesAndNestedBlocks)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    Analyzer Analysis;
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    auto Blocks = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{ {} } {}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Blocks.succeeded());
    EXPECT_TRUE(Analysis.analyze(Context, Empty, Output));
    EXPECT_TRUE(Analysis.analyze(Context, Blocks, Output));
    EXPECT_TRUE(Analysis.analyze(Context, Empty, Output));
  }

  // Concurrent states keep separate insertion points while their generated instructions outlive both states.
  TEST(SemanticAnalyzerEntryTest, KeepsStateBuildersIndependentAndRetainsGeneratedIr)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Input.succeeded());
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto *FirstBlock = Function->createBlock();
    auto *SecondBlock = Function->createBlock();
    ASSERT_NE(FirstBlock, nullptr);
    ASSERT_NE(SecondBlock, nullptr);
    ir::ReturnInstruction *FirstReturn = nullptr;
    ir::ReturnInstruction *SecondReturn = nullptr;
    {
      AnalyzerState First(Context, Input, Output);
      AnalyzerState Second(Context, Input, Output);
      ASSERT_TRUE(First.irBuilder().setInsertPoint(*FirstBlock));
      EXPECT_EQ(Second.irBuilder().insertBlock(), nullptr);
      ASSERT_TRUE(Second.irBuilder().setInsertPoint(*SecondBlock));
      FirstReturn = First.irBuilder().createReturn();
      SecondReturn = Second.irBuilder().createReturn();
      ASSERT_NE(FirstReturn, nullptr);
      ASSERT_NE(SecondReturn, nullptr);
      EXPECT_EQ(FirstBlock->instruction(0), FirstReturn);
      EXPECT_EQ(SecondBlock->instruction(0), SecondReturn);
      First.irBuilder().clearInsertPoint();
      EXPECT_EQ(Second.irBuilder().insertBlock(), SecondBlock);
    }
    ASSERT_EQ(Output.globalFunctionCount(), 1U);
    EXPECT_EQ(Output.globalFunction(0), Function);
    ASSERT_EQ(FirstBlock->instructionCount(), 1U);
    ASSERT_EQ(SecondBlock->instructionCount(), 1U);
    EXPECT_EQ(FirstBlock->instruction(0), FirstReturn);
    EXPECT_EQ(SecondBlock->instruction(0), SecondReturn);
    EXPECT_EQ(FirstReturn->type(), Context.typePool().getVoidType());
    EXPECT_EQ(SecondReturn->type(), Context.typePool().getVoidType());
  }

  // Caller-owned output retains its target layout, types and constants after the analyzer and parsed input expire.
  TEST(SemanticAnalyzerEntryTest, PreservesCallerOwnedIrBeyondAnalysisLifetime)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    constexpr target::TargetContext Target(target::PointerWidth::Bits32, target::ByteOrder::BigEndian);
    SemanticContext Context(Sources, DiagnosticEngine, Target);
    ir::Module Output(Context.irContext());
    ir::IRBuilder Builder(Context.irContext());
    ir::ReturnInstruction *Return = nullptr;
    {
      frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
      Analyzer Analysis;
      auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{}"));
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Analysis.analyze(Context, Parsed, Output));
      auto *Pointer = Context.typePool().getPointerType(Context.typePool().getInt32Type());
      ASSERT_NE(Pointer, nullptr);
      auto *Global = Output.createGlobalVariable(*Pointer);
      ASSERT_NE(Global, nullptr);
      const auto *Null = Output.constantPool().getNullPointerConstant();
      ASSERT_TRUE(Global->setInitializer(Null));
      auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Pointer));
      ASSERT_NE(Function, nullptr);
      auto *Block = Function->createBlock();
      ASSERT_TRUE(Builder.setInsertPoint(*Block));
      Return = Builder.createReturn(*Null);
      ASSERT_NE(Return, nullptr);
    }
    const SemanticContext &ReadOnly = Context;
    const ir::Module &ReadOnlyOutput = Output;
    const ir::IRBuilder &ReadOnlyBuilder = Builder;
    EXPECT_EQ(ReadOnlyOutput.targetContext(), Target);
    EXPECT_EQ(&ReadOnly.typePool(), &ReadOnlyOutput.typePool());
    EXPECT_EQ(&ReadOnly.typePool().irContext(), &ReadOnly.irContext());
    EXPECT_EQ(&ReadOnlyBuilder.irContext(), &ReadOnly.irContext());
    EXPECT_EQ(&ReadOnlyBuilder.typePool(), &ReadOnly.typePool());
    EXPECT_EQ(&ReadOnlyBuilder.constantPool(), &ReadOnlyOutput.constantPool());
    ASSERT_EQ(ReadOnlyOutput.globalVariableCount(), 1U);
    ASSERT_EQ(ReadOnlyOutput.globalFunctionCount(), 1U);
    const auto *Global = ReadOnlyOutput.globalVariable(0);
    const auto *Function = ReadOnlyOutput.globalFunction(0);
    EXPECT_EQ(Global->type()->size(), 4U);
    EXPECT_EQ(Global->type()->alignment(), 4U);
    EXPECT_EQ(Function->functionType()->returnType(), Global->type());
    EXPECT_TRUE(ReadOnlyOutput.ownsType(Global->type()));
    EXPECT_TRUE(ReadOnlyOutput.constantPool().owns(Global->initializer()));
    EXPECT_EQ(Function->entryBlock()->instruction(0), Return);
    EXPECT_EQ(Return->returnedValue(), Global->initializer());
    EXPECT_EQ(Return->type(), ReadOnly.typePool().getVoidType());
  }

  // Accepted analyses preserve caller-owned declarations, pooled constants and independent builder state.
  TEST(SemanticAnalyzerEntryTest, PreservesCallerOwnedIrAcrossAcceptedInputs)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    ir::IRBuilder Builder(Context.irContext());
    Analyzer Analysis;
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    auto Blocks = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{{}} {}"));
    ASSERT_TRUE(Empty.succeeded());
    ASSERT_TRUE(Blocks.succeeded());
    auto &Types = Context.typePool();
    const auto *Integer = Output.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 42);
    ASSERT_NE(Integer, nullptr);
    auto *Global = Output.createGlobalVariable(*Types.getInt32Type());
    ASSERT_NE(Global, nullptr);
    ASSERT_TRUE(Global->setInitializer(Integer));
    auto *Function = Output.createGlobalFunction(*Types.getFunctionType(Types.getInt32Type()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    const frontend::parser::ParseResult *Inputs[] = {&Empty, &Blocks, &Empty};
    for (const auto *Input : Inputs)
    {
      ASSERT_TRUE(Builder.setInsertPoint(*Block));
      ASSERT_TRUE(Analysis.analyze(Context, *Input, Output));
      EXPECT_EQ(Builder.insertBlock(), Block);
      EXPECT_EQ(Builder.insertFunction(), Function);
      EXPECT_EQ(Output.globalVariableCount(), 1U);
      EXPECT_EQ(Output.globalFunctionCount(), 1U);
      EXPECT_EQ(Output.globalVariable(0), Global);
      EXPECT_EQ(Output.globalFunction(0), Function);
      EXPECT_EQ(Global->initializer(), Integer);
      EXPECT_EQ(Output.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 42), Integer);
      EXPECT_EQ(Block->instructionCount(), 0U);
    }
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    auto *Return = Builder.createReturn(*Integer);
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
    ir::Module Output(Context.irContext());
    ir::IRBuilder Builder(Context.irContext());
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
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    const frontend::parser::ParseResult *Inputs[] = {&Missing, &Broken, &Cancelled, &Limited};
    for (const auto *Input : Inputs)
    {
      EXPECT_FALSE(Analysis.analyze(Context, *Input, Output));
      EXPECT_EQ(Builder.insertBlock(), Block);
      EXPECT_EQ(Builder.insertFunction(), Function);
      EXPECT_EQ(Output.globalFunctionCount(), 1U);
      EXPECT_EQ(Output.globalFunction(0), Function);
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
    ir::Module Output(Context.irContext());
    ir::IRBuilder Builder(Context.irContext());
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    Analyzer Analysis;
    EXPECT_FALSE(Analysis.analyze(Context, Parsed, Output));
    EXPECT_EQ(Builder.insertBlock(), Block);
    EXPECT_EQ(Builder.insertFunction(), Function);
    EXPECT_EQ(Output.globalFunctionCount(), 1U);
    EXPECT_EQ(Output.globalFunction(0), Function);
    EXPECT_TRUE(Analysis.analyze(Context, OtherParsed, Output));
    EXPECT_EQ(Builder.insertBlock(), Block);
    EXPECT_EQ(Output.globalFunction(0), Function);
  }

  // Explicit output selection isolates module contents while both outputs borrow the semantic context's pools.
  TEST(SemanticAnalyzerEntryTest, AcceptsIndependentCallerOwnedOutputs)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module First(Context.irContext());
    ir::Module Second(Context.irContext());
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{}"));
    ASSERT_TRUE(Parsed.succeeded());
    auto *Variable = First.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    Analyzer Analysis;
    EXPECT_TRUE(Analysis.analyze(Context, Parsed, First));
    EXPECT_TRUE(Analysis.analyze(Context, Parsed, Second));
    EXPECT_EQ(First.globalVariable(0), Variable);
    EXPECT_EQ(Second.globalVariableCount(), 0U);
    EXPECT_EQ(&First.constantPool(), &Second.constantPool());
  }

  // A foreign output context is rejected before analysis and leaves both output modules unchanged.
  TEST(SemanticAnalyzerEntryTest, RejectsOutputFromAnotherIrContext)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    SemanticContext Foreign(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    ir::Module ForeignOutput(Foreign.irContext());
    auto *Variable = ForeignOutput.createGlobalVariable(*Foreign.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Parsed.succeeded());
    Analyzer Analysis;
    EXPECT_FALSE(Analysis.analyze(Context, Parsed, ForeignOutput));
    EXPECT_EQ(ForeignOutput.globalVariable(0), Variable);
    EXPECT_EQ(ForeignOutput.globalVariableCount(), 1U);
    EXPECT_EQ(Output.globalVariableCount(), 0U);
    EXPECT_TRUE(Analysis.analyze(Context, Parsed, Output));
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
        {"value = 1;", "SimpleStmt"},
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
      ir::Module Output(Context.irContext());
      EXPECT_DEATH(Analysis.analyze(Context, Parsed, Output), std::string("initial semantic analyzer does not support ") + Entry.Kind);
    }
  }

  // Nested traversal enforces its budget and restores depth before siblings and subsequent calls.
  TEST(SemanticAnalyzerEntryDeathTest, BoundsBlockTraversalAndRestoresDepth)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("2"));
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    Analyzer Analysis;
    auto Siblings = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{{}} {{}}"));
    auto TooDeep = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{{{}}}"));
    auto Empty = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, ""));
    ASSERT_TRUE(Siblings.succeeded());
    ASSERT_TRUE(TooDeep.succeeded());
    ASSERT_TRUE(Empty.succeeded());
    EXPECT_TRUE(Analysis.analyze(Context, Siblings, Output));
    EXPECT_DEATH(Analysis.analyze(Context, TooDeep, Output), "INK-S0014");
    EXPECT_TRUE(Analysis.analyze(Context, Siblings, Output));
    ASSERT_TRUE(Limit.set("0"));
    EXPECT_TRUE(Analysis.analyze(Context, Empty, Output));
    EXPECT_DEATH(Analysis.analyze(Context, Siblings, Output), "INK-S0014");
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
    ir::Module Output(Context.irContext());
    EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed, Output), "initial semantic analyzer does not support BlockStmt");
  }
} // namespace ink::semantic::test
