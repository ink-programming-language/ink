#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/context.h"
#include "../core/support/environment_test_support.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace ink::semantic::test
{
  namespace
  {
    class SemanticAnalyzerTypeTest : public testing::Test
    {
      protected:
        SemanticAnalyzerTypeTest()
            : Frontend(Sources, DiagnosticEngine),
              Context(Sources, DiagnosticEngine, target::TargetContext::native()),
              Output(Context.irContext())
        {
          DiagnosticEngine.addConsumer(Diagnostics);
        }

        frontend::parser::ParseResult read(std::string_view Type)
        {
          Diagnostics.clear();
          return frontend::parser::parse(Frontend, frontend::tokenizer::tokenize(Frontend, "var Value: " + std::string(Type) + ";"));
        }

        const frontend::parser::TypeSyntax &syntax(const frontend::parser::ParseResult &Input)
        {
          const auto *Statement = frontend::parser::cast<frontend::parser::DeclStmt>(Input.Unit->root()->statements()[0]);
          return *frontend::parser::cast<frontend::parser::VarDecl>(Statement->declaration())->type();
        }

        const ir::Type *analyze(const frontend::parser::ParseResult &Input)
        {
          AnalyzerState State(Context, Input, Output);
          return analyze(State, Input);
        }

        const ir::Type *analyze(AnalyzerState &State, const frontend::parser::ParseResult &Input)
        {
          Context.setSource(Input.Unit->input().lexedFile().sourceId());
          return Analysis.analyzeType(State, syntax(Input));
        }

        static core::Name name(std::string_view Text)
        {
          return core::NamePool::instance().intern(Text);
        }

        core::SourceManager Sources;
        core::CollectingDiagnosticConsumer Diagnostics;
        core::DiagnosticEngine DiagnosticEngine;
        frontend::FrontendContext Frontend;
        SemanticContext Context;
        ir::Module Output;
        Analyzer Analysis;
    };
  } // namespace

  // Every supported builtin resolves to its context's canonical type, including parentheses around the name.
  TEST_F(SemanticAnalyzerTypeTest, ResolvesBuiltinNamesAndParentheses)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"void", Types.getVoidType()},
        {"bool", Types.getBoolType()},
        {"u8", Types.getUInt8Type()},
        {"i32", Types.getInt32Type()},
        {"u32", Types.getUInt32Type()},
        {"i64", Types.getInt64Type()},
        {"u64", Types.getUInt64Type()},
        {"f32", Types.getFloat32Type()},
        {"f64", Types.getFloat64Type()},
        {"(((i32)))", Types.getInt32Type()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Reusing builtin lookup across live contexts returns each context's own canonical type.
  TEST_F(SemanticAnalyzerTypeTest, ResolvesBuiltinsInEachIrContext)
  {
    auto Input = read("i32");
    ASSERT_TRUE(Input.succeeded());
    SemanticContext OtherContext(Sources, DiagnosticEngine, target::TargetContext::native());
    ir::Module OtherOutput(OtherContext.irContext());
    OtherContext.setSource(Input.Unit->input().lexedFile().sourceId());
    AnalyzerState OtherState(OtherContext, Input, OtherOutput);
    const auto *First = Context.typePool().getInt32Type();
    const auto *Second = OtherContext.typePool().getInt32Type();
    ASSERT_NE(First, Second);
    EXPECT_EQ(analyze(Input), First);
    EXPECT_EQ(Analysis.analyzeType(OtherState, syntax(Input)), Second);
    EXPECT_EQ(analyze(Input), First);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Recoverable type lookup failures and later successful resolution preserve the state's selected block and existing IR.
  TEST_F(SemanticAnalyzerTypeTest, PreservesBuilderStateAcrossTypeFailureAndSuccess)
  {
    auto Input = read("*Missing");
    ASSERT_TRUE(Input.succeeded());
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    auto *Return = State.irBuilder().createReturn();
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(analyze(State, Input), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    ASSERT_EQ(Block->instructionCount(), 1U);
    EXPECT_EQ(Block->instruction(0), Return);
    Diagnostics.clear();
    auto *Type = Output.createClassType(4, 4);
    ASSERT_NE(Type, nullptr);
    ASSERT_EQ(State.nameResolver().bind(name("Missing"), *Type), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Context.typePool().getPointerType(Type));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    ASSERT_EQ(Block->instructionCount(), 1U);
    EXPECT_EQ(Block->instruction(0), Return);
  }

  // Pointer and reference operators compose recursively and preserve void as a valid referenced type.
  TEST_F(SemanticAnalyzerTypeTest, CanonicalizesNestedPointersAndReferences)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"*i32", Types.getPointerType(Types.getInt32Type())},
        {"&i32", Types.getReferenceType(Types.getInt32Type())},
        {"*(&(*i32))", Types.getPointerType(Types.getReferenceType(Types.getPointerType(Types.getInt32Type())))},
        {"*void", Types.getPointerType(Types.getVoidType())},
        {"&void", Types.getReferenceType(Types.getVoidType())},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), Entry.Expected);
      EXPECT_EQ(analyze(Input), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // The type wrapper and direct expression analysis share canonical constructors, including array counts beyond i32.
  TEST_F(SemanticAnalyzerTypeTest, SharesTypeExpressionsWithGeneralAnalysis)
  {
    auto &Types = Context.typePool();
    struct Case
    {
        const char *Text;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"((i32))", Types.getInt32Type()},
        {"*(&(*i32))", Types.getPointerType(Types.getReferenceType(Types.getPointerType(Types.getInt32Type())))},
        {"[*u8; 14]", Types.getArrayType(Types.getPointerType(Types.getUInt8Type()), 14)},
        {"[u8; 2147483648]", Types.getArrayType(Types.getUInt8Type(), 2147483648ULL)},
        {"[u8; 18446744073709551615]", Types.getArrayType(Types.getUInt8Type(), std::numeric_limits<std::uint64_t>::max())},
        {"func(i32, *u8...): &void", Types.getFunctionType(Types.getReferenceType(Types.getVoidType()), {ir::FunctionTypeParameter(Types.getInt32Type()), ir::FunctionTypeParameter(Types.getPointerType(Types.getUInt8Type()), true)})},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      Context.setSource(Input.Unit->input().lexedFile().sourceId());
      const auto *Result = Analysis.analyzeExpr(State, *syntax(Input).expression());
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result, Entry.Expected);
      EXPECT_EQ(Result->type(), nullptr);
      EXPECT_EQ(analyze(State, Input), Entry.Expected);
      EXPECT_EQ(State.recursionDepth(), 0U);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      EXPECT_FALSE(Analysis.analyzeExpr(State, *syntax(Input).expression(), {.ExpectedType = Types.getInt32Type()}));
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // Successfully analyzed literal values are rejected once by the enclosing type requirement, retaining its full source span.
  TEST_F(SemanticAnalyzerTypeTest, RejectsLiteralValuesAfterExpressionAnalysis)
  {
    constexpr const char *Cases[] = {
        "14",
        "1.5",
        "true",
        "'a'",
        "\"hello\"",
        "((14))",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      Context.setSource(Input.Unit->input().lexedFile().sourceId());
      ASSERT_TRUE(Analysis.analyzeExpr(State, *syntax(Input).expression()));
      EXPECT_EQ(analyze(State, Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(Input).expression()->getSourceRange());
      EXPECT_EQ(State.recursionDepth(), 0U);
    }
  }

  // Marked type expressions and array count expressions use the same unsupported-comptime policy.
  TEST_F(SemanticAnalyzerTypeTest, ReportsUnsupportedComptimeTypeExpressions)
  {
    constexpr const char *Cases[] = {
        "comptime *i32",
        "[u8; comptime 4]",
    };
    for (const char *Text : Cases)
    {
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "INK-S0012");
    }
  }

  // Named nominal types retain declaration identity and nearer bindings override both outer types and builtins.
  TEST_F(SemanticAnalyzerTypeTest, PreservesNominalIdentityAndLexicalShadowing)
  {
    auto Input = read("Foo");
    auto BuiltinInput = read("i32");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(BuiltinInput.succeeded());
    auto *Outer = Output.createClassType(4, 4, name("Foo"));
    auto *Inner = Output.createClassType(4, 4, name("Foo"));
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    AnalyzerState State(Context, Input, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Foo"), *Outer), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Outer);
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(name("Foo"), *Inner), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Inner);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(analyze(State, Input), Outer);
    AnalyzerState BuiltinState(Context, BuiltinInput, Output);
    ASSERT_EQ(BuiltinState.nameResolver().bind(name("i32"), *Inner), BindResult::Inserted);
    EXPECT_EQ(analyze(BuiltinState, BuiltinInput), Inner);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Visible values and overloads block type fallback, with singleton functions rejected by the consumer and multiple targets ambiguous.
  TEST_F(SemanticAnalyzerTypeTest, RejectsValuesAndOverloadsInTypePosition)
  {
    auto Input = read("Foo");
    auto BuiltinInput = read("i32");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(BuiltinInput.succeeded());
    auto *Type = Output.createClassType(4, 4, name("Foo"));
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    auto *Function = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Type, nullptr);
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Function, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Foo"), *Type), BindResult::Inserted);
    State.nameResolver().enterScope();
    ASSERT_EQ(State.nameResolver().bind(name("Foo"), *Variable), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    ASSERT_TRUE(State.nameResolver().exitScope());
    EXPECT_EQ(analyze(State, Input), Type);
    Diagnostics.clear();
    AnalyzerState BuiltinState(Context, BuiltinInput, Output);
    ASSERT_EQ(BuiltinState.nameResolver().bindOverload(name("i32"), *Function), BindResult::Inserted);
    EXPECT_EQ(Analysis.analyzeExpr(BuiltinState, *syntax(BuiltinInput).expression()), Function);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_EQ(analyze(BuiltinState, BuiltinInput), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    auto *OtherFunction = Output.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getUInt32Type()));
    ASSERT_NE(OtherFunction, nullptr);
    ASSERT_EQ(BuiltinState.nameResolver().bindOverload(name("i32"), *OtherFunction), BindResult::Inserted);
    Diagnostics.clear();
    EXPECT_EQ(analyze(BuiltinState, BuiltinInput), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticAmbiguousName);
  }

  // Unknown names and unsupported integer widths emit one diagnostic against the currently selected input.
  TEST_F(SemanticAnalyzerTypeTest, AttributesUnknownNameDiagnosticsToEachInput)
  {
    auto First = read("MissingType");
    auto Second = read("i16");
    ASSERT_TRUE(First.succeeded());
    ASSERT_TRUE(Second.succeeded());
    const frontend::parser::ParseResult *Inputs[] = {&First, &Second, &First};
    for (const auto *Input : Inputs)
    {
      Diagnostics.clear();
      EXPECT_EQ(analyze(*Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      const auto &Diagnostic = Diagnostics.diagnostics()[0];
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticUnknownName);
      EXPECT_EQ(Diagnostic.Source, Input->Unit->input().lexedFile().sourceId());
      EXPECT_EQ(Diagnostic.Span, syntax(*Input).expression()->getSourceRange());
    }
  }

  // A nominal type from another module in the same context is reusable, but a foreign context's type is rejected.
  TEST_F(SemanticAnalyzerTypeTest, EnforcesIrContextCompatibilityForNamedTypes)
  {
    auto Input = read("OtherType");
    ASSERT_TRUE(Input.succeeded());
    ir::Module SameContextModule(Context.irContext());
    ir::IRContext ForeignContext(target::TargetContext::native());
    ir::Module ForeignModule(ForeignContext);
    auto *Shared = SameContextModule.createClassType(8, 8);
    auto *Foreign = ForeignModule.createClassType(8, 8);
    ASSERT_NE(Shared, nullptr);
    ASSERT_NE(Foreign, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("OtherType"), *Shared), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), Shared);
    State.nameResolver().enterScope();
    ASSERT_EQ(State.nameResolver().bind(name("OtherType"), *Foreign), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Parenthesized module and nominal member owners retain their identity while resolving chained type names.
  TEST_F(SemanticAnalyzerTypeTest, ResolvesNestedModuleAndTypeMembers)
  {
    auto Input = read("Pkg.Outer.Inner");
    ASSERT_TRUE(Input.succeeded());
    ir::Module Package(Context.irContext());
    auto *Outer = Package.createClassType(8, 8);
    auto *Inner = Package.createClassType(4, 4);
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    AnalyzerState State(Context, Input, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_EQ(Resolver.bind(name("Outer"), *Outer), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(*Outer), nullptr);
    ASSERT_EQ(Resolver.bind(name("Inner"), *Inner), BindResult::Inserted);
    ASSERT_TRUE(Resolver.exitScope());
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(analyze(State, Input), Inner);
    struct Case
    {
        const char *Text;
        const ir::Type *Expected;
    };
    const Case Cases[] = {
        {"(Pkg).Outer", Outer},
        {"((Pkg)).Outer", Outer},
        {"(Pkg.Outer).Inner", Inner},
        {"((Pkg).Outer).Inner", Inner},
        {"(Pkg.Outer.Inner)", Inner},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto ParenthesizedInput = read(Entry.Text);
      ASSERT_TRUE(ParenthesizedInput.succeeded());
      EXPECT_EQ(analyze(ParenthesizedInput), Entry.Expected);
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Modules and ordinary values are rejected once at every position that requires an actual type.
  TEST_F(SemanticAnalyzerTypeTest, RejectsNonTypesAtEachTypeRequirement)
  {
    ir::Module Package(Context.irContext());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Value"), *Variable), BindResult::Inserted);
    constexpr const char *Cases[] = {
        "Pkg",
        "(Pkg)",
        "*Pkg",
        "*(Pkg)",
        "&Pkg",
        "[Pkg; 3]",
        "func(Pkg): void",
        "func(): Pkg",
        "Value",
        "(Value)",
        "func(Value): void",
        "func(): Value",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
      if (std::string_view(Text) == "(Pkg)" || std::string_view(Text) == "(Value)")
      {
        EXPECT_EQ(Diagnostics.diagnostics()[0].Span, syntax(Input).expression()->getSourceRange());
      }
    }
  }

  // Missing members retain unknown-name diagnostics without lexical fallback, while resolved non-type members fail the consumer requirement.
  TEST_F(SemanticAnalyzerTypeTest, RejectsInvalidMemberAccessWithoutLexicalFallback)
  {
    ir::Module Package(Context.irContext());
    auto *Outer = Package.createClassType(8, 8);
    auto *RootType = Output.createClassType(4, 4);
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(RootType, nullptr);
    ASSERT_NE(Variable, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Missing"), *RootType), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Value"), *Variable), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_EQ(Resolver.bind(name("Outer"), *Outer), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Value"), *Variable), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(*Outer), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    ASSERT_TRUE(Resolver.exitScope());
    struct Case
    {
        const char *Text;
        core::DiagnosticKind Expected;
    };
    constexpr Case Cases[] = {
        {"Pkg.Missing", core::DiagnosticKind::SemanticUnknownName},
        {"Pkg.Outer.Missing", core::DiagnosticKind::SemanticUnknownName},
        {"Pkg.Value", core::DiagnosticKind::SemanticTypeMismatch},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Entry.Expected);
    }
  }

  // Type annotations retain the common unsupported-feature ICE for runtime operations, optional access and calls.
  TEST_F(SemanticAnalyzerTypeTest, ReportsUnsupportedOperationsInTypePosition)
  {
    ir::Module Package(Context.irContext());
    auto *Variable = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Variable, nullptr);
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Value"), *Variable), BindResult::Inserted);
    constexpr const char *Cases[] = {
        "*Value",
        "&Value",
        "&(Value)",
        "[Value; 3]",
        "Value.Missing",
        "Pkg?.Outer",
        "MakeType()",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "INK-S0012");
    }
  }

  // Integer literals retain the full uint64 range through parentheses, and character integer constants are valid lengths.
  TEST_F(SemanticAnalyzerTypeTest, AcceptsLiteralArrayLengths)
  {
    struct Case
    {
        const char *Text;
        std::uint64_t Count;
    };
    constexpr Case Cases[] = {
        {"[u8; 0]", 0},
        {"[u8; 7]", 7},
        {"[u8; 0xff]", 255},
        {"[u8; 0b1010]", 10},
        {"[u8; 0o17]", 15},
        {"[u8; 00012]", 12},
        {"[u8; (4)]", 4},
        {"[u8; ((3))]", 3},
        {"[u8; 'a']", 97},
        {"[u8; '\\0']", 0},
        {"[u8; 18446744073709551615]", std::numeric_limits<std::uint64_t>::max()},
        {"[u8; ((18446744073709551615))]", std::numeric_limits<std::uint64_t>::max()},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), Context.typePool().getArrayType(Context.typePool().getUInt8Type(), Entry.Count));
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Arrays compose with other type constructors and permit zero-sized void elements even at the maximum count.
  TEST_F(SemanticAnalyzerTypeTest, HandlesNestedArraysAndVoidElements)
  {
    auto Nested = read("[*i32; 4]");
    auto Unit = read("[void; 18446744073709551615]");
    ASSERT_TRUE(Nested.succeeded());
    ASSERT_TRUE(Unit.succeeded());
    auto &Types = Context.typePool();
    EXPECT_EQ(analyze(Nested), Types.getArrayType(Types.getPointerType(Types.getInt32Type()), 4));
    EXPECT_EQ(analyze(Unit), Types.getArrayType(Types.getVoidType(), std::numeric_limits<std::uint64_t>::max()));
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Noninteger results fail the length requirement while unresolved names and overflowing literals preserve their expression diagnostics.
  TEST_F(SemanticAnalyzerTypeTest, RejectsInvalidArrayLengths)
  {
    struct Case
    {
        const char *Text;
        core::DiagnosticKind Expected;
    };
    constexpr Case Cases[] = {
        {"[i32; 1.5]", core::DiagnosticKind::SemanticInvalidArrayLength},
        {"[i32; true]", core::DiagnosticKind::SemanticInvalidArrayLength},
        {"[i32; \"4\"]", core::DiagnosticKind::SemanticInvalidArrayLength},
        {"[i32; u32]", core::DiagnosticKind::SemanticInvalidArrayLength},
        {"[i32; *u32]", core::DiagnosticKind::SemanticInvalidArrayLength},
        {"[i32; Count]", core::DiagnosticKind::SemanticUnknownName},
        {"[i32; Pkg.Count]", core::DiagnosticKind::SemanticUnknownName},
        {"[i32; 18446744073709551616]", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"[i32; 0x10000000000000000]", core::DiagnosticKind::SemanticIntegerOutOfRange},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Text);
      auto Input = read(Entry.Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Entry.Expected);
    }
  }

  // Length operands retain the common unsupported-feature ICE for calls and operations, including constant-only arithmetic.
  TEST_F(SemanticAnalyzerTypeTest, ReportsUnsupportedArrayLengthOperations)
  {
    constexpr const char *Cases[] = {
        "[u8; +3]",
        "[u8; -1]",
        "[u8; ~0]",
        "[u8; 1 + 2]",
        "[u8; 5 - 2]",
        "[u8; 2 * 3]",
        "[u8; 8 / 2]",
        "[u8; 14 % 5]",
        "[u8; 12 & 10]",
        "[u8; 8 | 4]",
        "[u8; 7 ^ 3]",
        "[u8; 1 << 4]",
        "[u8; 24 >> 2]",
        "[u8; (1 + 2)]",
        "[i32; Count()]",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_DEATH(analyze(Input), "INK-S0012");
    }
  }

  // Bound integer constants keep their declared types and supply lengths through names, members and parentheses.
  TEST_F(SemanticAnalyzerTypeTest, AcceptsBoundIntegerConstantArrayLengths)
  {
    auto &Types = Context.typePool();
    auto &Constants = Output.constantPool();
    struct Case
    {
        const char *Name;
        const ir::IntegerConstant *Constant;
        std::uint64_t Count;
    };
    const Case Cases[] = {
        {"SignedZero", Constants.getSignedIntegerConstant(Types.getInt32Type(), 0), 0},
        {"Signed32", Constants.getSignedIntegerConstant(Types.getInt32Type(), std::numeric_limits<std::int32_t>::max()), static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())},
        {"Unsigned32", Constants.getUnsignedIntegerConstant(Types.getUInt32Type(), std::numeric_limits<std::uint32_t>::max()), std::numeric_limits<std::uint32_t>::max()},
        {"Signed64", Constants.getSignedIntegerConstant(Types.getInt64Type(), std::numeric_limits<std::int64_t>::max()), static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())},
        {"Unsigned64", Constants.getUnsignedIntegerConstant(Types.getUInt64Type(), std::numeric_limits<std::uint64_t>::max()), std::numeric_limits<std::uint64_t>::max()},
        {"Unsigned8", Constants.getUnsignedIntegerConstant(Types.getUInt8Type(), 255), 255},
    };
    ir::Module Package(Context.irContext());
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    auto *PackageScope = Resolver.enterScope(Package);
    ASSERT_NE(PackageScope, nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      ASSERT_NE(Entry.Constant, nullptr);
      ASSERT_EQ(Resolver.bind(name(Entry.Name), const_cast<ir::IntegerConstant &>(*Entry.Constant)), BindResult::Inserted);
      ASSERT_TRUE(Resolver.enterScope(*PackageScope));
      ASSERT_EQ(Resolver.bind(name(Entry.Name), const_cast<ir::IntegerConstant &>(*Entry.Constant)), BindResult::Inserted);
      ASSERT_TRUE(Resolver.exitScope());
      const std::string Expressions[] = {
          Entry.Name,
          "((" + std::string(Entry.Name) + "))",
          "Pkg." + std::string(Entry.Name),
          "((Pkg." + std::string(Entry.Name) + "))",
      };
      for (const auto &Expression : Expressions)
      {
        SCOPED_TRACE(Expression);
        auto Input = read("[u8; " + Expression + "]");
        ASSERT_TRUE(Input.succeeded());
        AnalyzerState State(Context, Input, Output);
        EXPECT_EQ(analyze(State, Input), Types.getArrayType(Types.getUInt8Type(), Entry.Count));
        const auto *Array = frontend::parser::cast<frontend::parser::ArrayRepeatExpr>(syntax(Input).expression());
        ASSERT_NE(Array, nullptr);
        EXPECT_EQ(Analysis.analyzeExpr(State, *Array->count(), {.DefaultIntegerLiteralType = Types.getUInt64Type()}), Entry.Constant);
        EXPECT_TRUE(Diagnostics.diagnostics().empty());
        EXPECT_EQ(State.recursionDepth(), 0U);
      }
    }
  }

  // Negative pooled signed constants are rejected using their signed type, including the minimum i32 and i64 values.
  TEST_F(SemanticAnalyzerTypeTest, RejectsNegativeIntegerConstantArrayLengths)
  {
    auto &Types = Context.typePool();
    auto &Constants = Output.constantPool();
    const ir::IntegerConstant *Cases[] = {
        Constants.getSignedIntegerConstant(Types.getInt32Type(), -1),
        Constants.getSignedIntegerConstant(Types.getInt32Type(), std::numeric_limits<std::int32_t>::min()),
        Constants.getSignedIntegerConstant(Types.getInt64Type(), -1),
        Constants.getSignedIntegerConstant(Types.getInt64Type(), std::numeric_limits<std::int64_t>::min()),
    };
    for (const auto *Constant : Cases)
    {
      ASSERT_NE(Constant, nullptr);
      auto Input = read("[u8; ((Count))]");
      ASSERT_TRUE(Input.succeeded());
      AnalyzerState State(Context, Input, Output);
      State.nameResolver().enterScope();
      ASSERT_EQ(State.nameResolver().bind(name("Count"), const_cast<ir::IntegerConstant &>(*Constant)), BindResult::Inserted);
      EXPECT_EQ(analyze(State, Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
      EXPECT_EQ(State.recursionDepth(), 0U);
      ASSERT_TRUE(State.nameResolver().exitScope());
    }
  }

  // Module values and missing module members respectively fail the length requirement and ordinary name lookup.
  TEST_F(SemanticAnalyzerTypeTest, RejectsModuleValuesAndUnknownMembersAsArrayLengths)
  {
    ir::Module Package(Context.irContext());
    NameResolver Resolver(Context);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    auto ModuleInput = read("[u8; Pkg]");
    ASSERT_TRUE(ModuleInput.succeeded());
    EXPECT_EQ(analyze(ModuleInput), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
    auto MissingInput = read("[u8; Pkg.Missing]");
    ASSERT_TRUE(MissingInput.succeeded());
    EXPECT_EQ(analyze(MissingInput), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
  }

  // A length that fits uint64 still fails when multiplying it by the element size overflows the array layout.
  TEST_F(SemanticAnalyzerTypeTest, RejectsArrayLayoutOverflow)
  {
    auto Input = read("[i32; 18446744073709551615]");
    ASSERT_TRUE(Input.succeeded());
    EXPECT_EQ(analyze(Input), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
  }

  // A bound variable is not an integer constant even when initialized with one.
  TEST_F(SemanticAnalyzerTypeTest, RejectsMutableVariableAsArrayLength)
  {
    auto Input = read("[u8; Count]");
    ASSERT_TRUE(Input.succeeded());
    auto *Count = Output.createGlobalVariable(*Context.typePool().getInt32Type());
    ASSERT_NE(Count, nullptr);
    ASSERT_TRUE(Count->setInitializer(Output.constantPool().getSignedIntegerConstant(Context.typePool().getInt32Type(), 4)));
    AnalyzerState State(Context, Input, Output);
    ASSERT_EQ(State.nameResolver().bind(name("Count"), *Count), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
  }

  // A visible current-function parameter has an integer type but cannot provide a compile-time array length.
  TEST_F(SemanticAnalyzerTypeTest, RejectsParameterAsArrayLength)
  {
    auto Input = read("[u8; Count]");
    ASSERT_TRUE(Input.succeeded());
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType(), {ir::FunctionTypeParameter(Context.typePool().getUInt32Type())});
    auto *Function = Output.createGlobalFunction(*Signature);
    ASSERT_NE(Function, nullptr);
    auto *Block = Function->createBlock();
    ASSERT_NE(Block, nullptr);
    AnalyzerState State(Context, Input, Output);
    ASSERT_TRUE(State.irBuilder().setInsertPoint(*Block));
    ASSERT_EQ(State.nameResolver().bind(name("Count"), *Function->parameter(0)), BindResult::Inserted);
    EXPECT_EQ(analyze(State, Input), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
    EXPECT_EQ(State.irBuilder().insertBlock(), Block);
    EXPECT_EQ(Block->instructionCount(), 0U);
  }

  // Function signatures preserve ordered parameter types and variadic flags while discarding parameter names.
  TEST_F(SemanticAnalyzerTypeTest, CanonicalizesFunctionSignaturesWithVariadicParameters)
  {
    auto Fixed = read("func(i32, *u8): &void");
    auto Variadic = read("func(i32, *u8...): &void");
    auto Renamed = read("func(First: i32, Remaining: *u8...): &void");
    auto NonFinalVariadic = read("func(i32..., *u8): &void");
    auto Unit = read("func(void): void");
    auto Empty = read("func(): void");
    ASSERT_TRUE(Fixed.succeeded());
    ASSERT_TRUE(Variadic.succeeded());
    ASSERT_TRUE(Renamed.succeeded());
    ASSERT_TRUE(NonFinalVariadic.succeeded());
    ASSERT_TRUE(Unit.succeeded());
    ASSERT_TRUE(Empty.succeeded());
    auto &Types = Context.typePool();
    const auto *Result = Types.getReferenceType(Types.getVoidType());
    const auto *Pointer = Types.getPointerType(Types.getUInt8Type());
    const auto *FixedType = Types.getFunctionType(Result, {ir::FunctionTypeParameter(Types.getInt32Type()), ir::FunctionTypeParameter(Pointer)});
    const auto *VariadicType = Types.getFunctionType(Result, {ir::FunctionTypeParameter(Types.getInt32Type()), ir::FunctionTypeParameter(Pointer, true)});
    ASSERT_NE(FixedType, nullptr);
    ASSERT_NE(VariadicType, nullptr);
    EXPECT_NE(FixedType, VariadicType);
    EXPECT_EQ(analyze(Fixed), FixedType);
    EXPECT_EQ(analyze(Variadic), VariadicType);
    EXPECT_EQ(analyze(Renamed), VariadicType);
    EXPECT_EQ(analyze(NonFinalVariadic), Types.getFunctionType(Result, {ir::FunctionTypeParameter(Types.getInt32Type(), true), ir::FunctionTypeParameter(Pointer)}));
    EXPECT_EQ(analyze(Unit), Types.getFunctionType(Types.getVoidType(), {ir::FunctionTypeParameter(Types.getVoidType())}));
    EXPECT_EQ(analyze(Empty), Types.getFunctionType(Types.getVoidType()));
    ASSERT_EQ(VariadicType->parameters().size(), 2U);
    EXPECT_EQ(VariadicType->parameters()[0].ParameterType, Types.getInt32Type());
    EXPECT_EQ(VariadicType->parameters()[1].ParameterType, Pointer);
    EXPECT_FALSE(VariadicType->parameters()[0].Variadic);
    EXPECT_TRUE(VariadicType->parameters()[1].Variadic);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Failures in nested type constructors propagate the original unknown-name diagnostic exactly once.
  TEST_F(SemanticAnalyzerTypeTest, PropagatesNestedTypeFailureWithoutDuplicateDiagnostics)
  {
    constexpr const char *Cases[] = {
        "*Missing",
        "&Missing",
        "[Missing; 3]",
        "func(Missing): void",
        "func(i32): Missing",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    }
  }

  // Recursion depth is restored after sibling parameters, successful repeated calls and recoverable nested errors.
  TEST_F(SemanticAnalyzerTypeTest, RestoresDepthAcrossSiblingsAndFailures)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("3"));
    auto Input = read("func(*i32, *i32): *i32");
    auto Failure = read("*Missing");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(Failure.succeeded());
    AnalyzerState State(Context, Input, Output);
    const auto *First = analyze(State, Input);
    ASSERT_NE(First, nullptr);
    EXPECT_EQ(analyze(State, Input), First);
    AnalyzerState FailureState(Context, Failure, Output);
    EXPECT_EQ(analyze(FailureState, Failure), nullptr);
    auto *Type = Output.createClassType(4, 4);
    ASSERT_NE(Type, nullptr);
    ASSERT_EQ(FailureState.nameResolver().bind(name("Missing"), *Type), BindResult::Inserted);
    EXPECT_EQ(analyze(FailureState, Failure), Context.typePool().getPointerType(Type));
  }

  // Recursive type syntax terminates with the configured semantic limit diagnostic instead of exhausting the stack.
  TEST_F(SemanticAnalyzerTypeTest, EnforcesRecursionDepthLimit)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("2"));
    auto Valid = read("*i32");
    auto Input = read("*(*i32)");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Input.succeeded());
    EXPECT_EQ(analyze(Valid), Context.typePool().getPointerType(Context.typePool().getInt32Type()));
    EXPECT_DEATH(analyze(Input), "INK-S0014");
    ASSERT_TRUE(Limit.set("0"));
    auto Builtin = read("i32");
    ASSERT_TRUE(Builtin.succeeded());
    EXPECT_DEATH(analyze(Builtin), "INK-S0014");
  }

  // A member, a parenthesis and its module name consume exactly three levels, restored across repeated calls.
  TEST_F(SemanticAnalyzerTypeTest, CountsParenthesizedMemberOwnersOncePerNode)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("3"));
    auto Valid = read("(Pkg).T");
    auto Nested = read("((Pkg)).T");
    auto Missing = read("(Pkg).Missing");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Nested.succeeded());
    ASSERT_TRUE(Missing.succeeded());
    ir::Module Package(Context.irContext());
    auto *Type = Package.createClassType(4, 4);
    ASSERT_NE(Type, nullptr);
    AnalyzerState State(Context, Valid, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    auto *PackageScope = Resolver.enterScope(Package);
    ASSERT_NE(PackageScope, nullptr);
    ASSERT_EQ(Resolver.bind(name("T"), *Type), BindResult::Inserted);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(analyze(State, Valid), Type);
    EXPECT_EQ(analyze(State, Valid), Type);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    AnalyzerState FailureState(Context, Missing, Output);
    EXPECT_EQ(analyze(FailureState, Missing), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownName);
    Diagnostics.clear();
    ASSERT_TRUE(Resolver.enterScope(*PackageScope));
    ASSERT_EQ(Resolver.bind(name("Missing"), *Type), BindResult::Inserted);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(analyze(FailureState, Missing), Type);
    EXPECT_EQ(analyze(State, Valid), Type);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_DEATH(analyze(Nested), "INK-S0014");
  }

  // Array element and count expressions share the recursion budget, including parentheses around the count.
  TEST_F(SemanticAnalyzerTypeTest, EnforcesArrayTypeDepthAndRestoresItAcrossCalls)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("2"));
    auto Valid = read("[u8; 3]");
    auto Nested = read("[[u8; 3]; 2]");
    auto Parenthesized = read("[u8; (3)]");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Nested.succeeded());
    ASSERT_TRUE(Parenthesized.succeeded());
    AnalyzerState State(Context, Valid, Output);
    const auto *Expected = Context.typePool().getArrayType(Context.typePool().getUInt8Type(), 3);
    EXPECT_EQ(analyze(State, Valid), Expected);
    EXPECT_EQ(analyze(State, Valid), Expected);
    EXPECT_DEATH(analyze(Nested), "INK-S0014");
    EXPECT_DEATH(analyze(Parenthesized), "INK-S0014");
    EXPECT_EQ(analyze(State, Valid), Expected);
    EXPECT_EQ(State.recursionDepth(), 0U);
    ASSERT_TRUE(Limit.set("3"));
    EXPECT_EQ(analyze(Parenthesized), Expected);
  }
} // namespace ink::semantic::test
