#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"
#include "ink/semantic/context.h"
#include "ink/frontend/parser/parser.h"
#include "../core/support/environment_test_support.h"

#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  namespace
  {
    class InvalidStatement final : public frontend::parser::Stmt
    {
      public:
        explicit InvalidStatement(core::SourceRange Range)
            : Stmt(frontend::parser::ASTKind::LiteralExpr, Range)
        {
        }
    };

    class InvalidDeclaration final : public frontend::parser::Decl
    {
      public:
        explicit InvalidDeclaration(core::SourceRange Range)
            : Decl(frontend::parser::ASTKind::SimpleStmt, Range)
        {
        }
    };
  } // namespace

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
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &>().moduleManager()), ir::ModuleManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &>().moduleManager()), const ir::ModuleManager &>);
  static_assert(std::is_same_v<decltype(std::declval<SemanticContext &&>().moduleManager()), ir::ModuleManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const SemanticContext &&>().moduleManager()), const ir::ModuleManager &>);
  static_assert(std::is_same_v<decltype(std::declval<AnalyzerState &>().input()), const frontend::parser::TokenBuffer &>);
  static_assert(std::is_same_v<decltype(std::declval<const AnalyzerState &>().input()), const frontend::parser::TokenBuffer &>);

  // A single analysis pass accepts the implemented literal and builtin boolean statement forms.
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

  // Unknown expression names are diagnosed once each, including following sibling statements in the single pass.
  TEST(SemanticAnalyzerEntryTest, ReportsUnknownExpressionNamesOnce)
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

  // Runtime consumers preserve full descriptor-expression ranges, inner lookup ranges and diagnostic order without reporting null results twice.
  TEST(SemanticAnalyzerEntryTest, ReportsRuntimeDescriptorErrorsInSourceOrder)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    constexpr std::string_view Source = "((i32)); ((Missing)); ((type));";
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, std::string(Source)));
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(Analyzer{}.analyze(Context, Input, Output));
    ASSERT_EQ(Consumer.diagnostics().size(), 3U);
    EXPECT_EQ(Consumer.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    EXPECT_EQ(Consumer.diagnostics()[1].Kind, core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(Consumer.diagnostics()[2].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    const std::string_view Highlights[] = {"((i32))", "Missing", "((type))"};
    for (std::size_t Index = 0; Index < std::size(Highlights); ++Index)
    {
      const auto Offset = Source.find(Highlights[Index]);
      ASSERT_NE(Offset, std::string_view::npos);
      EXPECT_EQ(Consumer.diagnostics()[Index].Source, Input.Unit->input().lexedFile().sourceId());
      EXPECT_EQ(Consumer.diagnostics()[Index].Span, core::SourceRange::fromByteOffsets(Offset, Offset + Highlights[Index].size()));
    }
  }

  // Returns in ordinary module blocks are source errors and each statement is visited once.
  TEST(SemanticAnalyzerEntryTest, ReportsReturnOutsideFunctionOnce)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "{ return; } return 1;"));
    ASSERT_TRUE(Input.succeeded());
    EXPECT_FALSE(Analyzer{}.analyze(Context, Input, Output));
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    EXPECT_EQ(Consumer.diagnostics()[0].Kind, core::DiagnosticKind::SemanticReturnOutsideFunction);
    EXPECT_EQ(Consumer.diagnostics()[1].Kind, core::DiagnosticKind::SemanticReturnOutsideFunction);
    EXPECT_EQ(Output.globalFunctionCount(), 0U);
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

  // Concurrent states share the parsed token view, keep separate insertion points and retain generated instructions beyond both states.
  TEST(SemanticAnalyzerEntryTest, KeepsStateBuildersIndependentAndRetainsGeneratedIr)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "14;"));
    ASSERT_TRUE(Input.succeeded());
    auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
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
      const AnalyzerState &ReadOnlyFirst = First;
      EXPECT_EQ(&First.input(), &Input.Unit->input());
      EXPECT_EQ(&ReadOnlyFirst.input(), &Input.Unit->input());
      EXPECT_EQ(&Second.input(), &First.input());
      const auto &Token = First.input().requireToken(0, Diagnostics, core::SourceRange::fromByteOffsets(0, 2));
      EXPECT_EQ(&Token, &Input.Unit->input().lexedFile().tokens().front());
      EXPECT_EQ(First.input().spelling(Token), "14");
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
    EXPECT_EQ(FirstReturn->type(), Context.typePool().get<ink::ir::VoidType>());
    EXPECT_EQ(SecondReturn->type(), Context.typePool().get<ink::ir::VoidType>());
  }

  // Node, explicit-range and derived sites retain their input source across context changes without modifying the parent site.
  TEST(SemanticAnalyzerEntryTest, KeepsAnalysisSitesAnchoredAcrossSourceSelection)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    const auto FirstSource = Sources.addSource("first.ink", "First;\n  Second;");
    const auto SecondSource = Sources.addSource("second.ink", "Other;");
    const auto SelectedSource = Sources.addSource("selected.ink", "Selected;");
    auto FirstInput = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, FirstSource));
    auto SecondInput = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, SecondSource));
    ASSERT_TRUE(FirstInput.succeeded());
    ASSERT_TRUE(SecondInput.succeeded());
    ASSERT_TRUE(Consumer.diagnostics().empty());
    AnalyzerState First(Context, FirstInput, Output);
    const auto &FirstNode = *FirstInput.Unit->root()->statements()[0];
    const auto ExplicitRange = core::SourceRange::fromByteOffsets(9, 15);
    const auto SelectedRange = core::SourceRange::fromByteOffsets(0, 8);
    Context.setSource(SelectedSource);
    const auto NodeSite = First.at(FirstNode);
    const auto RangeSite = First.at(ExplicitRange);
    Context.report<core::DiagnosticKind::SemanticUnknownName>(SelectedRange, "Selected");
    Context.setSource(SecondSource);
    const auto DerivedSite = NodeSite.at(ExplicitRange);
    EXPECT_EQ(&DerivedSite.semanticContext(), &NodeSite.semanticContext());
    EXPECT_EQ(&DerivedSite.semanticContext(), &Context);
    AnalyzerState Second(Context, SecondInput, Output);
    const auto &SecondNode = *SecondInput.Unit->root()->statements()[0];
    const auto OtherSite = Second.at(SecondNode);
    OtherSite.reportError<core::DiagnosticKind::SemanticUnknownName>("Other");
    NodeSite.reportError<core::DiagnosticKind::SemanticUnknownName>("First");
    RangeSite.reportError<core::DiagnosticKind::SemanticUnknownName>("Second");
    DerivedSite.reportError<core::DiagnosticKind::SemanticUnknownName>("Second");
    Context.report<core::DiagnosticKind::SemanticUnknownName>(SecondNode.getSourceRange(), "Other");
    ASSERT_EQ(Consumer.diagnostics().size(), 6U);
    EXPECT_EQ(Consumer.diagnostics()[0].Source, SelectedSource);
    EXPECT_EQ(Consumer.diagnostics()[0].Span, SelectedRange);
    EXPECT_EQ(Consumer.diagnostics()[1].Source, SecondSource);
    EXPECT_EQ(Consumer.diagnostics()[1].Span, SecondNode.getSourceRange());
    EXPECT_EQ(Consumer.diagnostics()[2].Source, FirstSource);
    EXPECT_EQ(Consumer.diagnostics()[2].Span, FirstNode.getSourceRange());
    EXPECT_EQ(Consumer.diagnostics()[3].Source, FirstSource);
    EXPECT_EQ(Consumer.diagnostics()[3].Span, ExplicitRange);
    EXPECT_EQ(Consumer.diagnostics()[4].Source, FirstSource);
    EXPECT_EQ(Consumer.diagnostics()[4].Span, ExplicitRange);
    EXPECT_EQ(Consumer.diagnostics()[5].Source, SecondSource);
    EXPECT_EQ(Consumer.diagnostics()[5].Span, SecondNode.getSourceRange());
  }

  // Runtime validation rejects descriptors at the site's captured location while preserving null propagation and valid value identity.
  TEST(SemanticAnalyzerEntryTest, RuntimeValueValidationPreservesAnalysisSiteLocation)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    const auto Source = Sources.addSource("value.ink", "Value;");
    const auto OtherSource = Sources.addSource("other.ink", "Other;");
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, Source));
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    const auto Range = core::SourceRange::fromByteOffsets(0, 5);
    const auto Site = State.at(Range);
    Context.setSource(OtherSource);
    EXPECT_EQ(requireRuntimeValue(Site, &Output), nullptr);
    EXPECT_EQ(requireRuntimeValue(Site, Context.typePool().get<ink::ir::IntegerType>(32, true)), nullptr);
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    for (const auto &Diagnostic : Consumer.diagnostics())
    {
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticTypeMismatch);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_EQ(Diagnostic.Source, Source);
      EXPECT_EQ(Diagnostic.Span, Range);
    }
    EXPECT_EQ(requireRuntimeValue(Site, nullptr), nullptr);
    const auto *Boolean = Context.irContext().constantPool().get<ink::ir::BoolConstant>(true);
    ASSERT_NE(Boolean, nullptr);
    EXPECT_EQ(requireRuntimeValue(Site, Boolean), Boolean);
    EXPECT_EQ(Consumer.diagnostics().size(), 2U);
  }

  // Type requirements retain their site when the context changes and distinguish local types from values and foreign types.
  TEST(SemanticAnalyzerEntryTest, TypeValidationPreservesAnalysisSiteLocation)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    ir::IRContext Foreign(target::TargetContext::native());
    const auto Source = Sources.addSource("type.ink", "Type;");
    const auto OtherSource = Sources.addSource("other.ink", "Other;");
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, Source));
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    const auto Range = core::SourceRange::fromByteOffsets(0, 4);
    const auto Site = State.at(Range);
    Context.setSource(OtherSource);
    const auto *Boolean = Context.irContext().constantPool().get<ink::ir::BoolConstant>(true);
    ASSERT_NE(Boolean, nullptr);
    EXPECT_EQ(requireType(Site, Boolean), nullptr);
    EXPECT_EQ(requireType(Site, Foreign.typePool().get<ink::ir::IntegerType>(32, true)), nullptr);
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    for (const auto &Diagnostic : Consumer.diagnostics())
    {
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticTypeMismatch);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_EQ(Diagnostic.Source, Source);
      EXPECT_EQ(Diagnostic.Span, Range);
    }
    EXPECT_EQ(requireType(Site, nullptr), nullptr);
    const auto *Type = Context.typePool().get<ink::ir::IntegerType>(32, true);
    EXPECT_EQ(requireType(Site, Type), Type);
    EXPECT_EQ(Consumer.diagnostics().size(), 2U);
  }

  // Recoverable guards observe candidates once and format diagnostics only on failure without losing their captured location.
  TEST(SemanticAnalyzerEntryTest, RecoverableGuardsPreserveLazyFailurePropagation)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    const auto Source = Sources.addSource("guard.ink", "Missing;");
    const auto OtherSource = Sources.addSource("other.ink", "Other;");
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, Source));
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    const auto Range = core::SourceRange::fromByteOffsets(0, 7);
    const auto Site = State.at(Range);
    Context.setSource(OtherSource);
    unsigned Conditions = 0;
    unsigned Sites = 0;
    unsigned Descriptions = 0;
    unsigned FailureValues = 0;
    unsigned Continuations = 0;
    const auto Observe = [&](bool Condition)
    {
      ++Conditions;
      return Condition;
    };
    const auto Describe = [&]()
    {
      ++Descriptions;
      Context.setSource(OtherSource);
      return "Missing";
    };
    const auto SelectSite = [&]() -> const AnalysisSite &
    {
      ++Sites;
      return Site;
    };
    const auto Failure = [&]() -> const ir::Value *
    {
      ++FailureValues;
      EXPECT_EQ(Consumer.diagnostics().size(), FailureValues);
      return nullptr;
    };
    const auto Require = [&](const ir::Value *Candidate) -> const ir::Value *
    {
      RETURN_ERROR_IF_NOT(SelectSite(), Observe(Candidate != nullptr), core::DiagnosticKind::SemanticUnknownName, Failure(), Describe());
      ++Continuations;
      return Candidate;
    };
    const auto Reject = [&](const ir::Value *Candidate) -> const ir::Value *
    {
      RETURN_ERROR_IF(SelectSite(), Observe(Candidate == nullptr), core::DiagnosticKind::SemanticUnknownName, Failure(), Describe());
      ++Continuations;
      return Candidate;
    };
    const auto *Value = Context.irContext().constantPool().get<ink::ir::BoolConstant>(true);
    ASSERT_NE(Value, nullptr);
    EXPECT_EQ(Require(Value), Value);
    EXPECT_EQ(Reject(Value), Value);
    EXPECT_EQ(Conditions, 2U);
    EXPECT_EQ(Sites, 0U);
    EXPECT_EQ(Descriptions, 0U);
    EXPECT_EQ(FailureValues, 0U);
    EXPECT_EQ(Continuations, 2U);
    EXPECT_TRUE(Consumer.diagnostics().empty());
    EXPECT_EQ(Require(nullptr), nullptr);
    EXPECT_EQ(Reject(nullptr), nullptr);
    EXPECT_EQ(Conditions, 4U);
    EXPECT_EQ(Sites, 2U);
    EXPECT_EQ(Descriptions, 2U);
    EXPECT_EQ(FailureValues, 2U);
    EXPECT_EQ(Continuations, 2U);
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    for (const auto &Diagnostic : Consumer.diagnostics())
    {
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticUnknownName);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostic.Kind), core::DiagnosticSeverity::Error);
      EXPECT_EQ(Diagnostic.Source, Source);
      EXPECT_EQ(Diagnostic.Span, Range);
    }
  }

  // Warning guards evaluate their condition once, retain the captured location and continue after either warning direction.
  TEST(SemanticAnalyzerEntryTest, WarningGuardsPreserveLazyReportingAndContinuation)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    const auto Source = Sources.addSource("warning.ink", "  Value;");
    const auto OtherSource = Sources.addSource("other.ink", "Other;");
    const auto Range = core::SourceRange::fromByteOffsets(2, 7);
    const AnalysisSite Site(Context, Source, Range);
    Context.setSource(OtherSource);
    unsigned Conditions = 0;
    unsigned Sites = 0;
    unsigned Descriptions = 0;
    unsigned Continuations = 0;
    const auto Observe = [&](bool Condition)
    {
      ++Conditions;
      return Condition;
    };
    const auto SelectSite = [&]() -> const AnalysisSite &
    {
      ++Sites;
      return Site;
    };
    const auto Describe = [&]()
    {
      ++Descriptions;
      Context.setSource(OtherSource);
      return "Value";
    };
    const auto WarnRead = [&](bool Uninitialized)
    {
      WARNING_IF(SelectSite(), Observe(Uninitialized), core::DiagnosticKind::SemanticUninitializedRead, Describe());
      ++Continuations;
    };
    const auto WarnAddress = [&](bool Initialized)
    {
      WARNING_IF_NOT(SelectSite(), Observe(Initialized), core::DiagnosticKind::SemanticUninitializedAddress, Describe());
      ++Continuations;
    };
    WarnRead(false);
    WarnAddress(true);
    EXPECT_EQ(Conditions, 2U);
    EXPECT_EQ(Sites, 0U);
    EXPECT_EQ(Descriptions, 0U);
    EXPECT_EQ(Continuations, 2U);
    EXPECT_TRUE(Consumer.diagnostics().empty());
    WarnRead(true);
    WarnAddress(false);
    EXPECT_EQ(Conditions, 4U);
    EXPECT_EQ(Sites, 2U);
    EXPECT_EQ(Descriptions, 2U);
    EXPECT_EQ(Continuations, 4U);
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    EXPECT_EQ(Consumer.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUninitializedRead);
    EXPECT_EQ(Consumer.diagnostics()[1].Kind, core::DiagnosticKind::SemanticUninitializedAddress);
    for (const auto &Diagnostic : Consumer.diagnostics())
    {
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostic.Kind), core::DiagnosticSeverity::Warning);
      EXPECT_EQ(Diagnostic.Source, Source);
      EXPECT_EQ(Diagnostic.Span, Range);
    }
  }

  // Plain returns propagate null once without diagnostics, and successful ICE checks leave their reporting arguments unevaluated.
  TEST(SemanticAnalyzerEntryTest, KeepsPlainPropagationAndSuccessfulICEChecksSilent)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    unsigned Conditions = 0;
    unsigned ReturnValues = 0;
    unsigned Continuations = 0;
    unsigned Sites = 0;
    unsigned Descriptions = 0;
    const auto Observe = [&](const int *Candidate)
    {
      ++Conditions;
      return Candidate;
    };
    const auto Failure = [&]() -> const int *
    {
      ++ReturnValues;
      return nullptr;
    };
    const auto Require = [&](const int *Candidate) -> const int *
    {
      RETURN_IF_NOT(Observe(Candidate), Failure());
      ++Continuations;
      return Candidate;
    };
    const auto Reject = [&](const int *Candidate) -> const int *
    {
      RETURN_IF(Observe(Candidate) == nullptr, Failure());
      ++Continuations;
      return Candidate;
    };
    const int Value = 42;
    EXPECT_EQ(Require(&Value), &Value);
    EXPECT_EQ(Reject(&Value), &Value);
    EXPECT_EQ(Conditions, 2U);
    EXPECT_EQ(ReturnValues, 0U);
    EXPECT_EQ(Continuations, 2U);
    EXPECT_EQ(Require(nullptr), nullptr);
    EXPECT_EQ(Reject(nullptr), nullptr);
    EXPECT_EQ(Conditions, 4U);
    EXPECT_EQ(ReturnValues, 2U);
    EXPECT_EQ(Continuations, 2U);
    const auto SelectSite = [&]()
    {
      ++Sites;
      return AnalysisSite(Context, {}, {});
    };
    const auto Describe = [&]()
    {
      ++Descriptions;
      return "required value";
    };
    CHECK_ICE(SelectSite(), Observe(&Value), core::DiagnosticKind::SemanticInvalidAST, Describe());
    EXPECT_EQ(Conditions, 5U);
    EXPECT_EQ(Sites, 0U);
    EXPECT_EQ(Descriptions, 0U);
    EXPECT_TRUE(Consumer.diagnostics().empty());
  }

  // Site ICE formatting uses its captured source and exact range after the context selects a different source.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsAnalysisSiteICEAtCapturedLocation)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics(Sources);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    const auto Source = Sources.addSource("first.ink", "First;\n  Second;");
    const auto OtherSource = Sources.addSource("other.ink", "Unrelated;");
    auto Input = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, Source));
    ASSERT_TRUE(Input.succeeded());
    AnalyzerState State(Context, Input, Output);
    const auto Site = State.at(core::SourceRange::fromByteOffsets(9, 15));
    Context.setSource(OtherSource);
    EXPECT_DEATH(Site.reportICE<core::DiagnosticKind::SemanticInvalidAST>("missing required child"), ::testing::AllOf(::testing::HasSubstr("first.ink:2:3: internal compiler error[INK-S0061]: invalid AST: missing required child"), ::testing::HasSubstr("[bytes 9, 15)"), ::testing::HasSubstr("2 |   Second;")));
  }

  // A state without a parsed unit reports an explicit ICE without attributing the missing input to a stale semantic source.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsInvalidASTForStateWithoutParsedUnit)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics(Sources);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    Context.setSource(Sources.addSource("unrelated.ink", "old;"));
    ir::Module Output(Context.irContext());
    frontend::parser::ParseResult Input;
    AnalyzerState State(Context, Input, Output);
    EXPECT_DEATH(State.input(), "<unknown>: internal compiler error\\[INK-S0061\\]: invalid AST: analysis input requires a parsed unit");
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
      auto *Pointer = Context.typePool().get<ink::ir::PointerType>(Context.typePool().get<ink::ir::IntegerType>(32, true));
      ASSERT_NE(Pointer, nullptr);
      auto *Global = Output.createGlobalVariable(*Pointer);
      ASSERT_NE(Global, nullptr);
      const auto *Null = Output.constantPool().get<ink::ir::NullPointerConstant>();
      ASSERT_TRUE(Global->setInitializer(Null));
      auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Pointer));
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
    EXPECT_EQ(Return->type(), ReadOnly.typePool().get<ink::ir::VoidType>());
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
    const auto *Integer = Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42));
    ASSERT_NE(Integer, nullptr);
    auto *Global = Output.createGlobalVariable(*Types.get<ink::ir::IntegerType>(32, true));
    ASSERT_NE(Global, nullptr);
    ASSERT_TRUE(Global->setInitializer(Integer));
    auto *Function = Output.createGlobalFunction(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::IntegerType>(32, true)));
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
      EXPECT_EQ(Output.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42)), Integer);
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
    auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
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

  // A successful parse result must carry a module root even when its source and token buffer are valid.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsInvalidASTForMissingModuleRoot)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    frontend::parser::ParseResult Input;
    Input.Unit = std::make_unique<frontend::parser::ParsedUnit>(std::make_shared<frontend::parser::TokenBuffer>(frontend::tokenizer::tokenize(Frontend, "")));
    ASSERT_TRUE(Input.succeeded());
    EXPECT_DEATH(Analyzer{}.analyze(Context, Input, Output), "internal compiler error\\[INK-S0061\\]: invalid AST: successful parse result requires a module AST root");
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
    auto *Function = Output.createGlobalFunction(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
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
    auto *Variable = First.createGlobalVariable(*Context.typePool().get<ink::ir::IntegerType>(32, true));
    ASSERT_NE(Variable, nullptr);
    Analyzer Analysis;
    EXPECT_TRUE(Analysis.analyze(Context, Parsed, First));
    EXPECT_TRUE(Analysis.analyze(Context, Parsed, Second));
    EXPECT_EQ(First.globalVariable(0), Variable);
    EXPECT_EQ(Second.globalVariableCount(), 0U);
    EXPECT_EQ(&First.constantPool(), &Second.constantPool());
  }

  // Context-owned modules share its pools while the manager keeps separate named outputs and const access preserves their identity.
  TEST(SemanticAnalyzerEntryTest, OwnsNamedModulesInItsIrContext)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    auto &Names = core::NamePool::instance();
    const ir::ModuleID FirstID{{Names.intern("first")}};
    const ir::ModuleID SecondID{{Names.intern("second")}};
    const auto First = Context.moduleManager().createModule(FirstID);
    const auto Second = Context.moduleManager().createModule(SecondID);
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    EXPECT_NE(*First, *Second);
    EXPECT_EQ(&(*First)->irContext(), &Context.irContext());
    EXPECT_EQ(&(*Second)->irContext(), &Context.irContext());
    EXPECT_EQ(&(*First)->typePool(), &(*Second)->typePool());
    EXPECT_EQ(&(*First)->constantPool(), &(*Second)->constantPool());

    const SemanticContext &ReadOnly = Context;
    EXPECT_EQ(&ReadOnly.moduleManager().irContext(), &ReadOnly.irContext());
    EXPECT_EQ(ReadOnly.moduleManager().moduleCount(), 2U);
    EXPECT_EQ(ReadOnly.moduleManager().findModule(FirstID), *First);
    EXPECT_EQ(ReadOnly.moduleManager().findModule(SecondID), *Second);
    EXPECT_EQ(&std::move(Context).moduleManager(), &Context.moduleManager());
    EXPECT_EQ(&std::move(ReadOnly).moduleManager(), &ReadOnly.moduleManager());
  }

  // Independently parsed modules keep same-named function scopes and exports isolated after their parsed units are destroyed.
  TEST(SemanticAnalyzerEntryTest, AnalyzesManagedModulesWithIsolatedScopesAndExports)
  {
    core::SourceManager Sources;
    core::CollectingDiagnosticConsumer Consumer;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Consumer);
    frontend::FrontendContext Frontend(Sources, Diagnostics);
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    auto &Names = core::NamePool::instance();
    const ir::ModuleID FirstID{{Names.intern("first")}};
    const ir::ModuleID SecondID{{Names.intern("second")}};
    const auto First = Context.moduleManager().createModule(FirstID);
    const auto Second = Context.moduleManager().createModule(SecondID);
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    {
      auto FirstInput = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "public func Shared(): i32 { return 1; } public func OnlyFirst(): void {}"));
      auto SecondInput = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "public func Shared(): i32 { return 2; }"));
      ASSERT_TRUE(FirstInput.succeeded());
      ASSERT_TRUE(SecondInput.succeeded());
      ASSERT_TRUE(Analyzer{}.analyze(Context, FirstInput, **First));
      ASSERT_TRUE(Analyzer{}.analyze(Context, SecondInput, **Second));
    }
    EXPECT_TRUE(Consumer.diagnostics().empty());
    ASSERT_EQ((*First)->globalFunctionCount(), 2U);
    ASSERT_EQ((*Second)->globalFunctionCount(), 1U);
    const auto *FirstFunction = (*First)->globalFunction(0);
    const auto *SecondFunction = (*Second)->globalFunction(0);
    EXPECT_NE(FirstFunction, SecondFunction);
    EXPECT_EQ(FirstFunction->functionType(), SecondFunction->functionType());
    EXPECT_FALSE(FirstFunction->isDeclaration());
    EXPECT_FALSE(SecondFunction->isDeclaration());

    const auto *FirstScope = Context.scopeStore().memberScope(**First);
    const auto *SecondScope = Context.scopeStore().memberScope(**Second);
    ASSERT_NE(FirstScope, nullptr);
    ASSERT_NE(SecondScope, nullptr);
    EXPECT_NE(FirstScope, SecondScope);
    const core::Name Shared = Names.intern("Shared");
    const core::Name OnlyFirst = Names.intern("OnlyFirst");
    const auto *FirstBinding = FirstScope->lookupLocal(Shared);
    const auto *SecondBinding = SecondScope->lookupLocal(Shared);
    ASSERT_NE(FirstBinding, nullptr);
    ASSERT_NE(SecondBinding, nullptr);
    ASSERT_EQ(FirstBinding->targets().size(), 1U);
    ASSERT_EQ(SecondBinding->targets().size(), 1U);
    EXPECT_EQ(FirstBinding->targets()[0], FirstFunction);
    EXPECT_EQ(SecondBinding->targets()[0], SecondFunction);
    EXPECT_NE(FirstScope->lookupLocal(OnlyFirst), nullptr);
    EXPECT_EQ(SecondScope->lookupLocal(OnlyFirst), nullptr);
    EXPECT_EQ(Context.scopeStore().rootScope().lookupLocal(Shared), nullptr);

    const auto FirstExport = Context.moduleManager().resolveSymbol({FirstID, {Shared}});
    const auto SecondExport = Context.moduleManager().resolveSymbol({SecondID, {Shared}});
    ASSERT_TRUE(FirstExport);
    ASSERT_TRUE(SecondExport);
    ASSERT_EQ((*FirstExport)->Entries.size(), 1U);
    ASSERT_EQ((*SecondExport)->Entries.size(), 1U);
    EXPECT_EQ((*FirstExport)->Entries[0].Declaration, FirstFunction);
    EXPECT_EQ((*SecondExport)->Entries[0].Declaration, SecondFunction);
    EXPECT_EQ((*FirstExport)->Entries[0].SymbolType, FirstFunction->functionType());
    EXPECT_EQ((*SecondExport)->Entries[0].SymbolType, SecondFunction->functionType());
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
    auto *Variable = ForeignOutput.createGlobalVariable(*Foreign.typePool().get<ink::ir::IntegerType>(32, true));
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

  // A comptime block reports the unsupported ICE at its exact source range while evaluation remains absent.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsComptimeAsUnimplemented)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine(Sources);
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    const auto Source = Sources.addSource("comptime.ink", "\n  comptime {}");
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenizeSource(Frontend, Source));
    ASSERT_TRUE(Parsed.succeeded());
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed, Output), ::testing::AllOf(::testing::HasSubstr("comptime.ink:2:3: internal compiler error[INK-S0012]: initial semantic analyzer does not support BlockStmt"), ::testing::HasSubstr("[bytes 3, 14)"), ::testing::HasSubstr("2 |   comptime {}")));
  }

  // A real statement object carrying an expression kind must fail statement dispatch with the AST contract ICE.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsInvalidASTForWrongStatementCategory)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "0;"));
    ASSERT_TRUE(Parsed.succeeded());
    auto *Invalid = Parsed.Unit->context().make<InvalidStatement>(Parsed.Unit->root()->getSourceRange());
    // ASTContext owns mutable pointer storage; the node-array API exposes its entries through a const view.
    const_cast<frontend::parser::Stmt *&>(Parsed.Unit->root()->statements()[0]) = Invalid;
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed, Output), "INK-S0061.*invalid AST: statement node kind must belong to the statement AST category");
  }

  // A real declaration object carrying a statement kind must fail declaration dispatch with the AST contract ICE.
  TEST(SemanticAnalyzerEntryDeathTest, ReportsInvalidASTForWrongDeclarationCategory)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine DiagnosticEngine;
    frontend::FrontendContext Frontend(Sources, DiagnosticEngine);
    auto Parsed = frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "0;"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto Range = Parsed.Unit->root()->getSourceRange();
    auto &AST = Parsed.Unit->context();
    auto *Invalid = AST.make<InvalidDeclaration>(Range);
    auto *Statement = AST.make<frontend::parser::DeclStmt>(Range, Invalid);
    // ASTContext owns mutable pointer storage; the node-array API exposes its entries through a const view.
    const_cast<frontend::parser::Stmt *&>(Parsed.Unit->root()->statements()[0]) = Statement;
    SemanticContext Context(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module Output(Context.irContext());
    EXPECT_DEATH(Analyzer{}.analyze(Context, Parsed, Output), "INK-S0061.*invalid AST: declaration node kind must belong to the declaration AST category");
  }
} // namespace ink::semantic::test
