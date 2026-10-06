#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "../core/environment_test_support.h"

#include "ink/parser/parser.h"
#include "ink/ir/ir_builder.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>

namespace ink::semantic::test
{
  using namespace ink::ir;

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

  // Native import, local C ABI and native export retain independent ownership and source visibility while executing local bodies.
  TEST(SemanticFunctionAnalyzerTest, SeparatesNativeBindingFromAbiAndVisibility)
  {
    FunctionAnalysis Input("private import \"C\" func abs(Value: i32): i32; [abi(\"\\x43\")] private func callback(Value: i32): i32 { return Value + 1; } private export \"C\" func hidden(Value: i32): i32 { return callback(Value); } public export \"C\" func exposed(): i32 { return hidden(41); } func main(): i32 { return exposed(); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    const auto FunctionNamed = [&](std::string_view Name) -> const Function &
    {
      return static_cast<const Function &>(*Input.lookup(*Result, Name)->targets().front());
    };
    EXPECT_TRUE(FunctionNamed("abs").isNativeImport());
    EXPECT_FALSE(FunctionNamed("abs").hasBody());
    EXPECT_EQ(FunctionNamed("abs").visibility(), core::VisibilityKind::Private);
    EXPECT_EQ(FunctionNamed("callback").languageLinkage(), LanguageLinkage::C);
    EXPECT_EQ(FunctionNamed("callback").binding(), core::FunctionBinding::Local);
    EXPECT_TRUE(FunctionNamed("callback").hasBody());
    EXPECT_TRUE(FunctionNamed("hidden").isNativeExport());
    EXPECT_EQ(FunctionNamed("hidden").visibility(), core::VisibilityKind::Private);
    EXPECT_TRUE(FunctionNamed("exposed").isNativeExport());
    EXPECT_EQ(FunctionNamed("exposed").visibility(), core::VisibilityKind::Public);
    execution::ExecutionEngine Engine(Input.Context.irContext());
    const auto Executed = Engine.execute(FunctionNamed("main"));
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.integer().bits().words().front(), 42U);
  }

  // ABI-only local functions retain ordinary overload resolution because they expose no native symbol names.
  TEST(SemanticFunctionAnalyzerTest, AllowsLocalCAbiOverloads)
  {
    FunctionAnalysis Input("[abi(\"C\")] func choose(Value: i8): i32 { return 0; } [abi(\"C\")] func choose(Value: i32): i32 { return Value + 1; } func main(): i32 { return choose(41); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Input.lookup(*Result, "choose")->targets().size(), 2U);
    const auto &Main = static_cast<const Function &>(*Input.lookup(*Result, "main")->targets().front());
    execution::ExecutionEngine Engine(Input.Context.irContext());
    const auto Executed = Engine.execute(Main);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.integer().bits().words().front(), 42U);
  }

  // Malformed ABI attributes, unsupported ABI names and invalid native declaration bodies produce source errors instead of ICEs.
  TEST(SemanticFunctionAnalyzerTest, RejectsInvalidNativeDeclarationsAndAbiAttributes)
  {
    const std::pair<std::string_view, core::DiagnosticKind> Cases[] = {
        {"import \"C\" func f(): void {}", core::DiagnosticKind::SemanticNativeImportHasBody},
        {"export \"C\" func f(): void;", core::DiagnosticKind::SemanticNativeExportRequiresBody},
        {"func outer(): void { export \"C\" func f(): void {} }", core::DiagnosticKind::SemanticNativeExportRequiresTopLevel},
        {"[abi(\"C\")] func f(): void;", core::DiagnosticKind::SemanticFunctionRequiresBody},
        {"[abi] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi = \"C\"] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi()] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi(1)] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi(Name)] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi(Name = \"C\")] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi(\"C\", \"C\")] func f(): void {}", core::DiagnosticKind::SemanticInvalidAbiAttribute},
        {"[abi(\"C\"), abi(\"C\")] func f(): void {}", core::DiagnosticKind::SemanticDuplicateAbi},
        {"[abi(\"C\")] import \"C\" func f(): void;", core::DiagnosticKind::SemanticDuplicateAbi},
        {"[abi(\"C\")] export \"C\" func f(): void {}", core::DiagnosticKind::SemanticDuplicateAbi},
        {"[abi(\"C++\")] func f(): void {}", core::DiagnosticKind::SemanticUnsupportedAbi},
        {"import \"C++\" func f(): void;", core::DiagnosticKind::SemanticUnsupportedAbi},
        {"import \"C\\0other\" func f(): void;", core::DiagnosticKind::SemanticUnsupportedAbi},
        {"import \"\" func f(): void;", core::DiagnosticKind::SemanticUnsupportedAbi},
        {"[link(name = \"alias\")] export \"C\" func f(): void {}", core::DiagnosticKind::SemanticUnsupportedFunctionAttribute},
        {"[tag] func f(): void {}", core::DiagnosticKind::SemanticUnsupportedFunctionAttribute},
        {"[abi(\"C\")] func f(Value: i128): void {}", core::DiagnosticKind::SemanticUnsupportedAbiSignature},
        {"export \"C\" func f(Value: f16): void {}", core::DiagnosticKind::SemanticUnsupportedAbiSignature},
        {"export \"C\" func f(Value: &i32): void {}", core::DiagnosticKind::SemanticUnsupportedAbiSignature},
    };
    for (const auto &[Source, Expected] : Cases)
    {
      SCOPED_TRACE(Source);
      FunctionAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, Expected);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
    }
  }

  // C declarations preserve decoded linkage, pointer types, parameter names and declaration-only ownership.
  TEST(SemanticFunctionAnalyzerTest, AnalyzesNativePrintfImport)
  {
    FunctionAnalysis Input("import \"\\x43\" func printf(msg: *u8): i32;");
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
    EXPECT_TRUE(FunctionValue.isNativeImport());
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

  // Void definitions receive a return, including nested empty blocks; prototypes remain bodyless.
  TEST(SemanticFunctionAnalyzerTest, BuildsEmptyDefinitionsAndPrototypes)
  {
    FunctionAnalysis Input("import \"C\" func declared(x: (i64), flag: bool, y: f32, p: **u8, r: &i16): void; func empty(): void { {} } export \"C\" func c(): void {}");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->entryBlock().values().size(), 3U);
    auto &Declared = static_cast<Function &>(*Result->entryBlock().values()[0]);
    EXPECT_FALSE(Declared.hasBody());
    EXPECT_EQ(Declared.languageLinkage(), LanguageLinkage::C);
    ASSERT_EQ(Declared.parameters().size(), 5U);
    EXPECT_EQ(Declared.parameters()[4]->type().typeKind(), TypeKind::Reference);
    for (std::size_t Index = 1; Index < 3; ++Index)
    {
      auto &Definition = static_cast<Function &>(*Result->entryBlock().values()[Index]);
      ASSERT_TRUE(Definition.hasBody());
      ASSERT_EQ(Definition.entryBlock()->values().size(), 1U);
      EXPECT_TRUE(ReturnInstruction::classof(Definition.entryBlock()->values()[0].get()));
    }
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
  }

  // Fixed builtin scalar spellings resolve to canonical types, including wide integers and all supported floats.
  TEST(SemanticFunctionAnalyzerTest, ResolvesBuiltinScalarSignatures)
  {
    FunctionAnalysis Input("import \"C\" func f(a: i8, b: i16, c: i32, d: i64, e: i128, f: u8, g: u16, h: u32, i: u64, j: u128, k: f16, l: f32, m: f64, n: bool): *void;");
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
    FunctionAnalysis Input("func f(x: i32): void {} func f(x: u8): void {} func g(renamed: i32): void {}");
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
    FunctionAnalysis Input("func outer(x: i32): void { func inner(y: u8): void {} } func after(x: f64): void {}");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto &Outer = static_cast<const Function &>(*Result->entryBlock().values()[0]);
    ASSERT_EQ(Outer.entryBlock()->values().size(), 2U);
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
        {"func f(x: Missing): void {}", core::DiagnosticKind::SemanticUnknownType},
        {"func f(): Missing;", core::DiagnosticKind::SemanticUnknownType},
        {"func f(x: void): void {}", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: type): void {}", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: &void): void {}", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: *type): void {}", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: i32, x: u8): void {}", core::DiagnosticKind::SemanticDuplicateParameterName},
        {"func f(x: i032): void {}", core::DiagnosticKind::SemanticUnknownType},
        {"func f(x: f128): void {}", core::DiagnosticKind::SemanticUnknownType},
        {"func f(x: i999999999999999999999): void {}", core::DiagnosticKind::SemanticUnknownType},
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
    FunctionAnalysis Input("func f(x: i32, x: u8, x: bool): void {}");
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

  // Runtime defaults, variadic parameters and unsupported signature types are never silently accepted.
  TEST(SemanticFunctionAnalyzerTest, DiagnosesUnsupportedSignatureFeatures)
  {
    const char *Cases[] = {
        "func f(x: i32 = 1): void {}",
        "func f(x: i32...): void {}",
        "func f(): type;",
        "func f(x: i32[4]): void {}",
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
        "func f(x: i32): i32 { return 0; } func f(x: i32): u8 { return 0; }",
        "import \"C\" func f(x: i32): void; import \"C\" func f(x: u8): void;",
        "func f(x: i32): void {} import \"C\" func f(x: u8): void;",
        "import \"C\" func f(x: i32): void; func f(x: u8): void {}",
        "func f(x: i32): void {} import \"C\" func f(x: i32): void;",
        "import \"C\" func f(x: i32): void; func f(x: i32): void {}",
        "import \"C\" func f(x: i32): void; func f(x: i32): void {}",
        "func f(x: i32): void {} import \"C\" func f(x: i32): void;",
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

  // Repeated C declarations are recoverable name conflicts until merging is supported.
  TEST(SemanticFunctionAnalyzerTest, RejectsRedeclarationMergingWithUserDiagnostic)
  {
    const char *Cases[] = {
        "import \"C\" func f(x: i32): void; export \"C\" func f(y: i32): void {}",
        "import \"C\" func f(x: i32): void; import \"C\" func f(y: i32): void;",
        "export \"C\" func f(x: i32): void {} import \"C\" func f(y: i32): void;",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      FunctionAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticDuplicateName);
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
    }
  }

  // A failed body discards its nested functions, while later declarations retain the enclosing insertion point and scope.
  TEST(SemanticFunctionAnalyzerTest, DiscardsFailedBodiesAndContinuesSiblings)
  {
    FunctionAnalysis Input("func bad(): i32 { func child(): void {} func invalid(x: Missing): void {} } func good(): void {}");
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

  // Hello-world produces a writable C string copy, a typed C call and an i32 return of zero.
  TEST(SemanticFunctionAnalyzerTest, AnalyzesHelloWorldThroughFunctionBodies)
  {
    FunctionAnalysis Input("import \"C\" func printf(msg: *u8): i32; func main(): i32 { printf(\"hello, world\"); return 0; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    ASSERT_EQ(Result->entryBlock().values().size(), 2U);
    const auto &Printf = static_cast<const Function &>(*Result->entryBlock().values()[0]);
    const auto &Main = static_cast<const Function &>(*Result->entryBlock().values()[1]);
    ASSERT_EQ(Main.entryBlock()->values().size(), 3U);
    const auto &Values = Main.entryBlock()->values();
    ASSERT_TRUE(CStringInstruction::classof(Values[0].get()));
    const auto &String = static_cast<const CStringInstruction &>(*Values[0]);
    EXPECT_EQ(String.source().value(), "hello, world");
    EXPECT_EQ(String.source().nullTerminatedValue().size(), 13U);
    EXPECT_EQ(String.source().nullTerminatedValue().back(), '\0');
    EXPECT_EQ(&String.type(), &Printf.parameters()[0]->type());
    ASSERT_TRUE(CallInstruction::classof(Values[1].get()));
    const auto &Call = static_cast<const CallInstruction &>(*Values[1]);
    EXPECT_EQ(Call.directCallee(), &Printf);
    EXPECT_EQ(Call.directCallee()->languageLinkage(), LanguageLinkage::C);
    ASSERT_EQ(Call.arguments().size(), 1U);
    EXPECT_EQ(Call.arguments()[0], &String);
    ASSERT_TRUE(ReturnInstruction::classof(Values[2].get()));
    const auto &Return = static_cast<const ReturnInstruction &>(*Values[2]);
    ASSERT_TRUE(IntegerConstant::classof(Return.returnedValue()));
    EXPECT_EQ(&Return.returnedValue()->type(), &Main.functionType().returnType());
    EXPECT_EQ(static_cast<const IntegerConstant &>(*Return.returnedValue()).value().words()[0], 0U);
    EXPECT_EQ(Return.function(), &Main);
  }

  // Integer literals use the expected signed width, all lexical bases, and full 128-bit precision.
  TEST(SemanticFunctionAnalyzerTest, ChecksIntegerLiteralBoundaries)
  {
    struct Case
    {
        const char *TypeName;
        const char *Literal;
        std::uint64_t Low;
        std::uint64_t High;
    };
    const Case Cases[] = {
        {"i8", "-128", 128, 0},
        {"i8", "+127", 127, 0},
        {"u8", "0xff", 255, 0},
        {"u8", "0b11111111", 255, 0},
        {"u8", "0o377", 255, 0},
        {"i32", "-2147483648", 0x80000000ULL, 0},
        {"i64", "-9223372036854775808", 0x8000000000000000ULL, 0},
        {"u64", "18446744073709551615", 0xffffffffffffffffULL, 0},
        {"i128", "-170141183460469231731687303715884105728", 0, 0x8000000000000000ULL},
        {"i128", "170141183460469231731687303715884105727", 0xffffffffffffffffULL, 0x7fffffffffffffffULL},
        {"u128", "340282366920938463463374607431768211455", 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {"i128", "-1", 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {"i16", "-(-32767)", 32767, 0},
    };
    for (const Case &Entry : Cases)
    {
      const std::string Source = std::string("func f(): ") + Entry.TypeName + " { return " + Entry.Literal + "; }";
      SCOPED_TRACE(Source);
      FunctionAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      Module *Result = Input.analyze();
      ASSERT_NE(Result, nullptr);
      const auto &FunctionValue = static_cast<const Function &>(*Result->entryBlock().values()[0]);
      const auto &Return = static_cast<const ReturnInstruction &>(*FunctionValue.entryBlock()->values()[0]);
      ASSERT_TRUE(IntegerConstant::classof(Return.returnedValue()));
      const auto &Constant = static_cast<const IntegerConstant &>(*Return.returnedValue());
      EXPECT_EQ(&Constant.type(), &FunctionValue.functionType().returnType());
      EXPECT_EQ(Constant.value().words()[0], Entry.Low);
      if (Constant.value().bitWidth() == 128)
      {
        EXPECT_EQ(Constant.value().words()[1], Entry.High);
      }
    }
  }

  // Invalid calls, returns and literals produce recoverable source errors without publishing the failed function.
  TEST(SemanticFunctionAnalyzerTest, DiagnosesInvalidExpressionsAndReturns)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"func f(): i32 { return missing; }", core::DiagnosticKind::SemanticUnknownName},
        {"func f(x: i32): void { x(); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(): void { 0(); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func g(x: i32): void {} func f(): void { g(); }", core::DiagnosticKind::SemanticArgumentCount},
        {"func g(): void {} func f(): void { g(1); }", core::DiagnosticKind::SemanticArgumentCount},
        {"func g(x: i32): void {} func f(): void { g(true); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(): i32 {}", core::DiagnosticKind::SemanticMissingReturn},
        {"func f(): i32 { return; }", core::DiagnosticKind::SemanticMissingReturn},
        {"func f(): void { return 0; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(): void {} func g(): i32 { return f(); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"return 0;", core::DiagnosticKind::SemanticReturnOutsideFunction},
        {"func f(): i32 { { return 0; } 1; }", core::DiagnosticKind::SemanticUnreachableStatement},
        {"func f(): i8 { return 128; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): i8 { return -129; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): u8 { return 256; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): u8 { return -1; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): i128 { return 170141183460469231731687303715884105728; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): u128 { return 340282366920938463463374607431768211456; }", core::DiagnosticKind::SemanticIntegerOutOfRange},
        {"func f(): i32 { return \"text\"; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(): *u8 { return \"text\"; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func g(x: *u8): void {} func f(): void { g(\"text\"); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"import \"C\" func g(x: *u8): void; func f(): void { g(\"a\\0b\"); }", core::DiagnosticKind::SemanticEmbeddedNull},
        {"import \"C\" func g(x: *i8): void; func f(): void { g(\"text\"); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f(x: i32): void { func inner(): i32 { return x; } }", core::DiagnosticKind::SemanticInvalidCapture},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      FunctionAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics()[0].Kind, Entry.Kind);
    }
  }

  // Calls preserve argument evaluation order and bind parameter uses and recursive callees to their owners.
  TEST(SemanticFunctionAnalyzerTest, AnalyzesNestedAndRecursiveCalls)
  {
    FunctionAnalysis Input("func echo(x: i32): i32 { return x; } func recurse(x: i32): i32 { return recurse(x); } func main(): i32 { return (echo)(echo(7)); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto &Echo = static_cast<const Function &>(*Result->entryBlock().values()[0]);
    EXPECT_EQ(static_cast<const ReturnInstruction &>(*Echo.entryBlock()->values()[0]).returnedValue(), Echo.parameters()[0].get());
    const auto &Recursive = static_cast<const Function &>(*Result->entryBlock().values()[1]);
    const auto &SelfCall = static_cast<const CallInstruction &>(*Recursive.entryBlock()->values()[0]);
    EXPECT_EQ(SelfCall.directCallee(), &Recursive);
    EXPECT_EQ(SelfCall.arguments()[0], Recursive.parameters()[0].get());
    const auto &Main = static_cast<const Function &>(*Result->entryBlock().values()[2]);
    ASSERT_EQ(Main.entryBlock()->values().size(), 3U);
    const auto &Inner = static_cast<const CallInstruction &>(*Main.entryBlock()->values()[0]);
    const auto &Outer = static_cast<const CallInstruction &>(*Main.entryBlock()->values()[1]);
    EXPECT_EQ(Inner.directCallee(), &Echo);
    EXPECT_EQ(Outer.arguments()[0], &Inner);
    EXPECT_EQ(static_cast<const ReturnInstruction &>(*Main.entryBlock()->values()[2]).returnedValue(), &Outer);
  }

  // Overloads prefer exact types and i32 literals; other fitting integer widths remain equally ranked.
  TEST(SemanticFunctionAnalyzerTest, SelectsOverloadsWithoutSpeculativeCalls)
  {
    FunctionAnalysis Input("func pick(x: i32): i32 { return 0; } func pick(x: u8): i32 { return 0; } func test(x: u8): i32 { pick(1); return pick(x); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto &Values = static_cast<const Function &>(*Result->entryBlock().values()[2]).entryBlock()->values();
    ASSERT_EQ(Values.size(), 3U);
    EXPECT_EQ(static_cast<const CallInstruction &>(*Values[0]).directCallee(), Result->entryBlock().values()[0].get());
    EXPECT_EQ(static_cast<const CallInstruction &>(*Values[1]).directCallee(), Result->entryBlock().values()[1].get());
    const char *Invalid[] = {
        "func pick(x: i8): void {} func pick(x: u8): void {} func f(): void { pick(1); }",
        "func pick(x: i32, y: u8): void {} func pick(x: u8, y: i32): void {} func f(): void { pick(1, 1); }",
        "func pick(x: i32): void {} func pick(x: u8): void {} func f(): void { pick; }",
    };
    for (const char *Source : Invalid)
    {
      FunctionAnalysis Ambiguous(Source);
      ASSERT_TRUE(Ambiguous.Parsed.succeeded());
      EXPECT_EQ(Ambiguous.analyze(), nullptr);
      ASSERT_EQ(Ambiguous.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Ambiguous.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticAmbiguousName);
    }
    FunctionAnalysis NoMatch("func pick(x: i32): void {} func pick(x: u8): void {} func f(): void { pick(false); }");
    ASSERT_TRUE(NoMatch.Parsed.succeeded());
    EXPECT_EQ(NoMatch.analyze(), nullptr);
    ASSERT_EQ(NoMatch.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(NoMatch.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::SemanticNoMatchingOverload);
  }

  // Equal decoded strings share only the immutable source; each C argument owns a distinct copy operation.
  TEST(SemanticFunctionAnalyzerTest, CopiesCStringArgumentsPerCall)
  {
    FunctionAnalysis Input("import \"C\" func sink(x: *u8): void; func f(): void { sink(\"hello\"); sink(\"\\x68ello\"); sink(\"\"); return; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    const auto &Values = static_cast<const Function &>(*Result->entryBlock().values()[1]).entryBlock()->values();
    ASSERT_EQ(Values.size(), 7U);
    const auto &First = static_cast<const CStringInstruction &>(*Values[0]);
    const auto &Second = static_cast<const CStringInstruction &>(*Values[2]);
    EXPECT_NE(&First, &Second);
    EXPECT_EQ(&First.source(), &Second.source());
    EXPECT_EQ(static_cast<const CStringInstruction &>(*Values[4]).source().nullTerminatedValue().size(), 1U);
    EXPECT_EQ(static_cast<const CallInstruction &>(*Values[1]).arguments()[0], &First);
    EXPECT_EQ(static_cast<const CallInstruction &>(*Values[3]).arguments()[0], &Second);
  }

  // Expression depth includes call targets and parentheses and is refreshed on each analysis.
  TEST(SemanticFunctionAnalyzerTest, BoundsExpressionDepth)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set("2"));
    FunctionAnalysis AtLimit("func f(): i32 { return (0); }");
    FunctionAnalysis OverLimit("func f(): i32 { return ((0)); }");
    ASSERT_TRUE(AtLimit.Parsed.succeeded());
    ASSERT_TRUE(OverLimit.Parsed.succeeded());
    EXPECT_NE(AtLimit.analyze(), nullptr);
    EXPECT_DEATH(OverLimit.analyze(), "internal compiler error\\[INK-S0014\\]");
    ASSERT_TRUE(Environment.set("3"));
    EXPECT_NE(OverLimit.analyze(), nullptr);
  }

  // A visible function or parameter in a type position is rejected as a value, respecting lexical shadowing.
  TEST(SemanticFunctionAnalyzerTest, ResolvesNamesBeforeBuiltinFallback)
  {
    const char *Cases[] = {
        "func T(): void {} func f(x: T): void {}",
        "func outer(i32: bool): void { func inner(x: i32): void {} }",
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
    FunctionAnalysis Input(std::string("import \"C\" func f(x: ") + std::string(260, '*') + "u8): void;", Limits);
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
    FunctionAnalysis AtLimit("import \"C\" func f(x: *u8): void;");
    FunctionAnalysis OverLimit("import \"C\" func f(x: **u8): void;");
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
    FunctionAnalysis Function("func f(): void {}");
    ASSERT_TRUE(Empty.Parsed.succeeded());
    ASSERT_TRUE(Function.Parsed.succeeded());
    EXPECT_NE(Empty.analyze(), nullptr);
    EXPECT_DEATH(Function.analyze(), "internal compiler error\\[INK-S0014\\]");
  }
} // namespace ink::semantic::test
