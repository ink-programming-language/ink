#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/module/module.h"
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

  // A visible value or overload set blocks outer type and builtin fallback instead of supplying its value type.
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
    EXPECT_EQ(analyze(BuiltinState, BuiltinInput), nullptr);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Unknown names and unsupported integer widths emit one diagnostic against the currently selected input.
  TEST_F(SemanticAnalyzerTypeTest, AttributesUnknownTypeDiagnosticsToEachInput)
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
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticUnknownType);
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
        "*Value",
        "&Value",
        "&(Value)",
        "[Value; 3]",
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

  // Missing members never fall back to a lexical parent, and values or optional member access cannot denote types.
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
        {"Pkg.Missing", core::DiagnosticKind::SemanticUnknownType},
        {"Pkg.Outer.Missing", core::DiagnosticKind::SemanticUnknownType},
        {"Pkg.Value", core::DiagnosticKind::SemanticTypeMismatch},
        {"Value.Missing", core::DiagnosticKind::SemanticTypeMismatch},
        {"Pkg?.Outer", core::DiagnosticKind::SemanticTypeMismatch},
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

  // Integer literals in every supported radix determine canonical array lengths up to the uint64 maximum.
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
        {"[u8; 18446744073709551615]", std::numeric_limits<std::uint64_t>::max()},
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

  // Noninteger literals, parentheses, names, members, calls and literals above uint64 cannot supply array lengths.
  TEST_F(SemanticAnalyzerTypeTest, RejectsInvalidArrayLengths)
  {
    constexpr const char *Cases[] = {
        "[i32; 1.5]",
        "[i32; true]",
        "[i32; (4)]",
        "[i32; ((3))]",
        "[i32; Count]",
        "[i32; Pkg.Count]",
        "[i32; 18446744073709551616]",
        "[i32; 0x10000000000000000]",
        "[i32; Count()]",
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
    }
  }

  // Unary and binary operations are rejected even when their operands and result would be valid integer constants.
  TEST_F(SemanticAnalyzerTypeTest, RejectsArrayLengthOperations)
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
    };
    for (const char *Text : Cases)
    {
      SCOPED_TRACE(Text);
      auto Input = read(Text);
      ASSERT_TRUE(Input.succeeded());
      EXPECT_EQ(analyze(Input), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
    }
  }

  // Names and members are rejected even when their bindings refer to valid pooled integer constants.
  TEST_F(SemanticAnalyzerTypeTest, RejectsBoundIntegerConstantArrayLengths)
  {
    auto Input = read("[u8; Count]");
    auto MemberInput = read("[u8; Pkg.Count]");
    ASSERT_TRUE(Input.succeeded());
    ASSERT_TRUE(MemberInput.succeeded());
    ir::Module Package(Context.irContext());
    const auto *Count = Output.constantPool().getUnsignedIntegerConstant(Context.typePool().getUInt32Type(), 6);
    ASSERT_NE(Count, nullptr);
    AnalyzerState State(Context, Input, Output);
    auto &Resolver = State.nameResolver();
    ASSERT_EQ(Resolver.bind(name("Count"), const_cast<ir::IntegerConstant &>(*Count)), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(name("Pkg"), Package), BindResult::Inserted);
    ASSERT_NE(Resolver.enterScope(Package), nullptr);
    ASSERT_EQ(Resolver.bind(name("Count"), const_cast<ir::IntegerConstant &>(*Count)), BindResult::Inserted);
    ASSERT_TRUE(Resolver.exitScope());
    const frontend::parser::ParseResult *Inputs[] = {&Input, &MemberInput};
    for (const auto *Current : Inputs)
    {
      Diagnostics.clear();
      EXPECT_EQ(analyze(*Current), nullptr);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticInvalidArrayLength);
    }
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

  // A bound variable is rejected as length syntax even when initialized with an integer constant.
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
    const auto *FixedType = Types.getFunctionType(Result, {ir::Parameter(Types.getInt32Type()), ir::Parameter(Pointer)});
    const auto *VariadicType = Types.getFunctionType(Result, {ir::Parameter(Types.getInt32Type()), ir::Parameter(Pointer, true)});
    ASSERT_NE(FixedType, nullptr);
    ASSERT_NE(VariadicType, nullptr);
    EXPECT_NE(FixedType, VariadicType);
    EXPECT_EQ(analyze(Fixed), FixedType);
    EXPECT_EQ(analyze(Variadic), VariadicType);
    EXPECT_EQ(analyze(Renamed), VariadicType);
    EXPECT_EQ(analyze(NonFinalVariadic), Types.getFunctionType(Result, {ir::Parameter(Types.getInt32Type(), true), ir::Parameter(Pointer)}));
    EXPECT_EQ(analyze(Unit), Types.getFunctionType(Types.getVoidType(), {ir::Parameter(Types.getVoidType())}));
    EXPECT_EQ(analyze(Empty), Types.getFunctionType(Types.getVoidType()));
    ASSERT_EQ(VariadicType->parameters().size(), 2U);
    EXPECT_FALSE(VariadicType->parameters()[0]->isVariadic());
    EXPECT_TRUE(VariadicType->parameters()[1]->isVariadic());
    EXPECT_FALSE(VariadicType->parameters()[0]->hasName());
    EXPECT_FALSE(VariadicType->parameters()[1]->hasName());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Failures in nested type constructors propagate the original unknown-type diagnostic exactly once.
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
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownType);
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
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownType);
    Diagnostics.clear();
    ASSERT_TRUE(Resolver.enterScope(*PackageScope));
    ASSERT_EQ(Resolver.bind(name("Missing"), *Type), BindResult::Inserted);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(analyze(FailureState, Missing), Type);
    EXPECT_EQ(analyze(State, Valid), Type);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_DEATH(analyze(Nested), "INK-S0014");
  }

  // Nested array element types consume recursion depth while literal lengths allow depth restoration across calls.
  TEST_F(SemanticAnalyzerTypeTest, EnforcesArrayTypeDepthAndRestoresItAcrossCalls)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_RECURSION_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("2"));
    auto Valid = read("[u8; 3]");
    auto Nested = read("[[u8; 3]; 2]");
    ASSERT_TRUE(Valid.succeeded());
    ASSERT_TRUE(Nested.succeeded());
    AnalyzerState State(Context, Valid, Output);
    const auto *Expected = Context.typePool().getArrayType(Context.typePool().getUInt8Type(), 3);
    EXPECT_EQ(analyze(State, Valid), Expected);
    EXPECT_EQ(analyze(State, Valid), Expected);
    EXPECT_DEATH(analyze(Nested), "INK-S0014");
    EXPECT_EQ(analyze(State, Valid), Expected);
  }
} // namespace ink::semantic::test
