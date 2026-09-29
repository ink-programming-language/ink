#include "ink/semantic/analyzer/analyzer.h"
#include "../core/environment_test_support.h"

#include "ink/parser/parser.h"
#include "ink/semantic/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>

namespace ink::semantic::test
{
  namespace
  {
    class FunctionAnalysis
    {
      public:
        explicit FunctionAnalysis(std::string_view Source, parser::ParseLimits Limits = {})
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)), Limits)),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        Module *analyze()
        {
          return Analyzer{}.analyze(Context, Parsed);
        }

        const Binding<Value *> *lookup(Module &Owner, std::string_view Text)
        {
          return NameResolver(Context).lookupMember(Owner, Context.namePool().find(Text));
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };
  } // namespace

  // C declarations preserve decoded linkage, pointer types, parameter names and declaration-only ownership.
  TEST(SemanticFunctionAnalyzerTest, AnalyzesExternPrintfDeclaration)
  {
    FunctionAnalysis Input("extern \"\\x43\" func printf(msg: *u8): i32;");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    ASSERT_EQ(Result->entryBlock().values().size(), 1U);
    const auto *Binding = Input.lookup(*Result, "printf");
    ASSERT_NE(Binding, nullptr);
    ASSERT_EQ(Binding->targets().size(), 1U);
    ASSERT_TRUE(Function::classof(Binding->targets()[0]));
    auto &FunctionValue = static_cast<Function &>(*Binding->targets()[0]);
    EXPECT_EQ(FunctionValue.outer(), &Result->entryBlock());
    EXPECT_FALSE(FunctionValue.hasBody());
    EXPECT_EQ(FunctionValue.languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(FunctionValue.callingConvention(), CallingConvention::C);
    EXPECT_EQ(&FunctionValue.functionType().returnType(), Input.Context.typePool().getType<TypeKind::Integer>(32, true));
    ASSERT_EQ(FunctionValue.parameters().size(), 1U);
    const auto &Parameter = *FunctionValue.parameters()[0];
    EXPECT_EQ(Input.Context.namePool().text(Parameter.name()), "msg");
    EXPECT_EQ(Parameter.parameterKind(), ParameterKind::Positional);
    EXPECT_EQ(Parameter.outer(), &FunctionValue);
    const Type *U8 = Input.Context.typePool().getType<TypeKind::Integer>(8, false);
    EXPECT_EQ(&Parameter.type(), Input.Context.typePool().getType<TypeKind::Pointer>(*U8, AccessKind::ReadWrite));
    const auto *ParameterBinding = NameResolver(Input.Context).lookupMember(FunctionValue, Parameter.name());
    ASSERT_NE(ParameterBinding, nullptr);
    EXPECT_EQ(ParameterBinding->targets()[0], &Parameter);
    EXPECT_EQ(Input.lookup(*Result, "msg"), nullptr);
    EXPECT_TRUE(Result->declarationRoot()->children().empty());
  }

  // Definitions create empty entry blocks without synthesizing returns or checking return paths; prototypes have no body.
  TEST(SemanticFunctionAnalyzerTest, BuildsEmptyDefinitionsAndPrototypes)
  {
    FunctionAnalysis Input("func declared(x: (i64), flag: bool, y: f32, p: **u8, r: &i16): void; func empty(): void { {} } extern \"C\" func c(): void {} func pending(): i32 {}");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->entryBlock().values().size(), 4U);
    auto &Declared = static_cast<Function &>(*Result->entryBlock().values()[0]);
    EXPECT_FALSE(Declared.hasBody());
    EXPECT_EQ(Declared.languageLinkage(), LanguageLinkage::Ink);
    ASSERT_EQ(Declared.parameters().size(), 5U);
    EXPECT_EQ(Declared.parameters()[4]->type().typeKind(), TypeKind::Reference);
    for (std::size_t Index = 1; Index < 4; ++Index)
    {
      auto &Definition = static_cast<Function &>(*Result->entryBlock().values()[Index]);
      ASSERT_TRUE(Definition.hasBody());
      EXPECT_TRUE(Definition.entryBlock()->values().empty());
    }
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
  }

  // Fixed builtin scalar spellings resolve to canonical types, including wide integers and all supported floats.
  TEST(SemanticFunctionAnalyzerTest, ResolvesBuiltinScalarSignatures)
  {
    FunctionAnalysis Input("func f(a: i8, b: i16, c: i32, d: i64, e: i128, f: u8, g: u16, h: u32, i: u64, j: u128, k: f16, l: f32, m: f64, n: bool): *void;");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto &FunctionValue = static_cast<const Function &>(*Result->entryBlock().values()[0]);
    ASSERT_EQ(FunctionValue.parameters().size(), 14U);
    EXPECT_EQ(&FunctionValue.parameters()[4]->type(), Input.Context.typePool().getType<TypeKind::Integer>(128, true));
    EXPECT_EQ(&FunctionValue.parameters()[9]->type(), Input.Context.typePool().getType<TypeKind::Integer>(128, false));
    EXPECT_EQ(&FunctionValue.parameters()[10]->type(), Input.Context.typePool().getType<TypeKind::Float>(16));
    EXPECT_EQ(&FunctionValue.parameters()[12]->type(), Input.Context.typePool().getType<TypeKind::Float>(64));
  }

  // Distinct parameter lists form an Ink overload set with signatures interned independently of parameter names.
  TEST(SemanticFunctionAnalyzerTest, BindsOrdinaryOverloads)
  {
    FunctionAnalysis Input("func f(x: i32): void; func f(x: u8): void; func g(renamed: i32): void;");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto *Overloads = Input.lookup(*Result, "f");
    ASSERT_NE(Overloads, nullptr);
    ASSERT_EQ(Overloads->targets().size(), 2U);
    EXPECT_TRUE(Overloads->isOverloadSet());
    EXPECT_EQ(&Overloads->targets()[0]->type(), &Input.lookup(*Result, "g")->targets()[0]->type());
    EXPECT_NE(&Overloads->targets()[0]->type(), &Overloads->targets()[1]->type());
  }

  // Nested functions remain owned by the enclosing function and cannot leak into the module's bindings.
  TEST(SemanticFunctionAnalyzerTest, PreservesNestedFunctionOwnershipAndScopes)
  {
    FunctionAnalysis Input("func outer(x: i32): void { func inner(y: u8): void {} } func after(x: f64): void;");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto &Outer = static_cast<const Function &>(*Result->entryBlock().values()[0]);
    ASSERT_EQ(Outer.entryBlock()->values().size(), 1U);
    auto &Inner = static_cast<Function &>(*Outer.entryBlock()->values()[0]);
    EXPECT_EQ(Inner.outer(), Outer.entryBlock());
    Scope *InnerScope = Input.Context.scopeStore().memberScope(Inner);
    ASSERT_NE(InnerScope, nullptr);
    NameResolver Resolver(*InnerScope);
    ASSERT_NE(Resolver.lookup(Input.Context.namePool().find("x")), nullptr);
    EXPECT_EQ(Resolver.lookup(Input.Context.namePool().find("x"))->targets()[0], Outer.parameters()[0].get());
    ASSERT_NE(Resolver.lookup(Input.Context.namePool().find("outer")), nullptr);
    EXPECT_EQ(Resolver.lookup(Input.Context.namePool().find("outer"))->targets()[0], &Outer);
    EXPECT_EQ(Input.lookup(*Result, "inner"), nullptr);
    EXPECT_EQ(Input.lookup(*Result, "x"), nullptr);
    EXPECT_NE(Input.lookup(*Result, "after"), nullptr);
  }

  // Invalid parameter/return types and duplicate parameter names report source diagnostics without publishing functions.
  TEST(SemanticFunctionAnalyzerTest, RejectsInvalidSignatures)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"func f(x: Missing): void;", core::DiagnosticKind::SemanticUnknownType},
        {"func f(): Missing;", core::DiagnosticKind::SemanticUnknownType},
        {"func f(x: void): void;", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: type): void;", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: &void): void;", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: *type): void;", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: i32, x: u8): void;", core::DiagnosticKind::SemanticDuplicateParameterName},
        {"func f(x: i032): void;", core::DiagnosticKind::SemanticUnknownType},
        {"func f(x: f128): void;", core::DiagnosticKind::SemanticUnknownType},
        {"func f(x: i999999999999999999999): void;", core::DiagnosticKind::SemanticUnknownType},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      FunctionAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      const auto &Diagnostic = Input.Diagnostics.diagnostics()[0];
      EXPECT_EQ(Diagnostic.Kind, Entry.Kind);
      EXPECT_EQ(Diagnostic.Source, Input.Parsed.Unit->input().lexedFile().sourceId());
      EXPECT_TRUE(Diagnostic.Span.isValid());
      EXPECT_TRUE(Input.Context.modules()[0]->entryBlock().values().empty());
      EXPECT_EQ(Input.lookup(*Input.Context.modules()[0], "f"), nullptr);
    }
  }

  // Each repeated parameter receives a dedicated uniqueness error on its own name token, regardless of its type.
  TEST(SemanticFunctionAnalyzerTest, ReportsDuplicateParameterNamesPrecisely)
  {
    FunctionAnalysis Input("func f(x: i32, x: u8, x: bool): void;");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    const auto &Diagnostics = Input.Diagnostics.diagnostics();
    ASSERT_EQ(Diagnostics.size(), 2U);
    const auto &Declaration = static_cast<const parser::FunctionDecl &>(*static_cast<const parser::DeclStmt *>(Input.Parsed.Unit->root()->statements()[0])->declaration());
    for (std::size_t Index = 0; Index < Diagnostics.size(); ++Index)
    {
      const auto &Diagnostic = Diagnostics[Index];
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::SemanticDuplicateParameterName);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_STREQ(Diagnostic.code(), "INK-S0015");
      EXPECT_EQ(Diagnostic.Span, Declaration.parameters()[Index + 1].name().Range);
      EXPECT_EQ(Diagnostic.Source, Input.Parsed.Unit->input().lexedFile().sourceId());
      EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostic).Message, "duplicate parameter name 'x'; parameter names must be unique within a function");
    }
    EXPECT_TRUE(Input.Context.modules()[0]->entryBlock().values().empty());
  }

  // Generic/default/variadic/attributed declarations, unsupported linkages and complex types are never silently accepted.
  TEST(SemanticFunctionAnalyzerTest, DiagnosesUnsupportedSignatureFeatures)
  {
    const char *Cases[] = {
        "func f[T: type](x: T): T;",
        "[tag] func f(): void;",
        "func f(x: i32 = 1): void;",
        "func f(x: i32...): void;",
        "extern \"C++\" func f(): void;",
        "extern \"C\\0other\" func f(): void;",
        "extern \"\" func f(): void;",
        "func f(): type;",
        "func f(x: i32[4]): void;",
        "func f(x: func(): void): void;",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      FunctionAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-S0012\\]");
    }
  }

  // Return-only overloads, duplicate definitions and C symbol overloads conflict with an existing function.
  TEST(SemanticFunctionAnalyzerTest, RejectsConflictingFunctions)
  {
    const char *Cases[] = {
        "func f(): void {} func f(): void {}",
        "func f(x: i32): i32; func f(x: i32): u8;",
        "extern \"C\" func f(x: i32): void; extern \"C\" func f(x: u8): void;",
        "func f(x: i32): void; extern \"C\" func f(x: u8): void;",
        "extern \"C\" func f(x: i32): void; func f(x: u8): void;",
        "func f(x: i32): void; extern \"C\" func f(x: i32): void;",
        "extern \"C\" func f(x: i32): void; func f(x: i32): void;",
        "extern \"C\" func f(x: i32): void; func f(x: i32): void {}",
        "func f(x: i32): void {} extern \"C\" func f(x: i32): void;",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      FunctionAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticDuplicateName);
      ASSERT_EQ(Input.Context.modules()[0]->entryBlock().values().size(), 1U);
      ASSERT_EQ(Input.lookup(*Input.Context.modules()[0], "f")->targets().size(), 1U);
    }
  }

  // Compatible redeclarations remain explicitly unsupported until declaration/definition merging is implemented.
  TEST(SemanticFunctionAnalyzerTest, RejectsUnimplementedRedeclarationMerging)
  {
    struct Case
    {
        const char *Source;
        bool FirstHasBody;
    };
    const Case Cases[] = {
        {"func f(x: i32): void; func f(y: i32): void {}", false},
        {"func f(x: i32): void; func f(y: i32): void;", false},
        {"func f(x: i32): void {} func f(y: i32): void;", true},
        {"extern \"C\" func f(x: i32): void; extern \"C\" func f(y: i32): void {}", false},
        {"extern \"C\" func f(x: i32): void; extern \"C\" func f(y: i32): void;", false},
        {"extern \"C\" func f(x: i32): void {} extern \"C\" func f(y: i32): void;", true},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      FunctionAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_DEATH(Input.analyze(), "initial semantic analyzer does not support function redeclarations");
    }
  }

  // A failed body discards its nested functions, while later declarations retain the enclosing insertion point and scope.
  TEST(SemanticFunctionAnalyzerTest, DiscardsFailedBodiesAndContinuesSiblings)
  {
    FunctionAnalysis Input("func bad(): i32 { func child(): void {} func invalid(x: Missing): void; } func good(): void {}");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticUnknownType);
    EXPECT_EQ(core::DiagnosticFormatter{}.format(Input.Diagnostics.diagnostics()[0]).Message, "unknown type 'Missing'");
    Module &Result = *Input.Context.modules()[0];
    ASSERT_EQ(Result.entryBlock().values().size(), 1U);
    EXPECT_NE(Input.lookup(Result, "good"), nullptr);
    EXPECT_EQ(Input.lookup(Result, "bad"), nullptr);
    EXPECT_EQ(Input.lookup(Result, "child"), nullptr);
  }

  // The hello-world example accepts printf's signature and panics at the first unsupported body statement.
  TEST(SemanticFunctionAnalyzerTest, AnalyzesHelloWorldThroughFunctionBodies)
  {
    FunctionAnalysis Input("extern \"C\" func printf(msg: *u8): i32; func main(): i32 { printf(\"hello, world\"); return 0; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_DEATH(Input.analyze(), "initial semantic analyzer does not support SimpleStmt");
  }

  // A visible function or parameter in a type position is rejected as a value, respecting lexical shadowing.
  TEST(SemanticFunctionAnalyzerTest, ResolvesNamesBeforeBuiltinFallback)
  {
    const char *Cases[] = {
        "func T(): void; func f(x: T): void;",
        "func outer(i32: bool): void { func inner(x: i32): void; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      FunctionAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticTypeMismatch);
    }
  }

  // A successful later analysis reuses the context without seeing another module's overloads or parameter bindings.
  TEST(SemanticFunctionAnalyzerTest, KeepsRepeatedAnalysisIndependent)
  {
    FunctionAnalysis Input("func f(x: i32): void {}");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *First = Input.analyze();
    Module *Second = Input.analyze();
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_NE(Input.lookup(*First, "f")->targets()[0], Input.lookup(*Second, "f")->targets()[0]);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
  }

  // Long pointer chains stop at the default semantic budget and succeed when its override is raised.
  TEST(SemanticFunctionAnalyzerTest, BoundsSignatureTypeNesting)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_TYPE_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set(nullptr));
    parser::ParseLimits Limits;
    Limits.MaxNestingDepth = 1024;
    FunctionAnalysis Input(std::string("func f(x: ") + std::string(260, '*') + "u8): void;", Limits);
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-S0014\\]");
    ASSERT_TRUE(Environment.set("300"));
    EXPECT_NE(Input.analyze(), nullptr);
  }

  // Type depth is bounded independently of block depth and refreshed for each analysis, including invalid overrides.
  TEST(SemanticFunctionAnalyzerTest, UsesConfiguredTypeDepthLimit)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_TYPE_DEPTH_LIMIT");
    core::test::ScopedEnvironmentVariable BlockEnvironment("INK_SEMANTIC_BLOCK_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set("2"));
    ASSERT_TRUE(BlockEnvironment.set("0"));
    FunctionAnalysis AtLimit("func f(x: *u8): void;");
    FunctionAnalysis OverLimit("func f(x: **u8): void;");
    ASSERT_TRUE(AtLimit.Parsed.succeeded());
    ASSERT_TRUE(OverLimit.Parsed.succeeded());
    EXPECT_NE(AtLimit.analyze(), nullptr);
    EXPECT_DEATH(OverLimit.analyze(), "internal compiler error\\[INK-S0014\\]");
    ASSERT_TRUE(Environment.set("3"));
    EXPECT_NE(OverLimit.analyze(), nullptr);
    ASSERT_TRUE(Environment.set("invalid"));
    EXPECT_NE(OverLimit.analyze(), nullptr);
  }

  // A zero type-depth limit rejects even a scalar return type while still allowing an empty module.
  TEST(SemanticFunctionAnalyzerTest, ZeroTypeDepthLimitRejectsAnyType)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_TYPE_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set("0"));
    FunctionAnalysis Empty("");
    FunctionAnalysis Function("func f(): void;");
    ASSERT_TRUE(Empty.Parsed.succeeded());
    ASSERT_TRUE(Function.Parsed.succeeded());
    EXPECT_NE(Empty.analyze(), nullptr);
    EXPECT_DEATH(Function.analyze(), "internal compiler error\\[INK-S0014\\]");
  }
} // namespace ink::semantic::test
