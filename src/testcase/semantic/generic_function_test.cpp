#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "ink/parser/parser.h"
#include "ink/ir/linkage.h"
#include "ink/ir/module/module_serialization.h"
#include "ink/execution/engine/execution_engine.h"
#include "../core/environment_test_support.h"

#include <gtest/gtest.h>

namespace ink::semantic::test
{
  namespace
  {
    class GenericAnalysis
    {
      public:
        explicit GenericAnalysis(std::string_view Source)
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ir::Module *analyze(bool Modules = true)
        {
          const Analyzer::ModuleInput Input{"main", &Parsed};
          return Modules ? Analyzer{}.analyzeModules(Context, {&Input, 1}, "main") : Analyzer{}.analyze(Context, Parsed);
        }

        execution::ExecutionValueResult run(ir::Module &Module)
        {
          NameResolver Resolver(Context);
          const auto *Binding = Resolver.lookupMember(Module, Context.namePool().find("main"));
          if (!Binding || Binding->targets().size() != 1 || !ir::Function::classof(Binding->targets().front()))
          {
            return {};
          }
          return Context.comptimeState().Engine.execute(static_cast<const ir::Function &>(*Binding->targets().front()));
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    void expectResult(std::string_view Source, std::uint64_t Expected, bool Modules = true)
    {
      GenericAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      auto *Module = Input.analyze(Modules);
      ASSERT_NE(Module, nullptr) << Source;
      EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
      const auto Result = Input.run(*Module);
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.integer().bits().words().front(), Expected);
    }
  } // namespace

  // Both analyzer entry points instantiate type parameters and execute concrete integer and bool signatures.
  TEST(SemanticGenericFunctionTest, ExplicitTypeArguments)
  {
    constexpr std::string_view Source = "func id[T: type](Value: T): T { return Value; } func main(): i32 { if (id::[bool](true)) { return id::[i32](42); } return 0; }";
    expectResult(Source, 42);
    expectResult(Source, 42, false);
  }

  // Canonical substitutions share one function despite differing call arguments and aliases of the function value.
  TEST(SemanticGenericFunctionTest, ReusesInstancesAndSupportsFunctionValues)
  {
    GenericAnalysis Input("func id[T: type](Value: T): T { return Value; } func main(): i32 { var f = id::[i32]; return f(20) + id::[i32](22); }");
    auto *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_EQ(Input.Context.genericState().Definitions.size(), 1U);
    const auto &Instances = Input.Context.genericState().Definitions.begin()->second.Instances;
    ASSERT_EQ(Instances.size(), 1U);
    EXPECT_EQ(Instances.front()->State, GenericState::Phase::Ready);
    EXPECT_FALSE(Instances.front()->Function->genericIdentity().empty());
    const auto Result = Input.run(*Module);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits().words().front(), 42U);
  }

  // Value parameters are normalized to their declared types and distinguish otherwise identical signatures.
  TEST(SemanticGenericFunctionTest, IntegerAndBooleanValueArguments)
  {
    expectResult("func offset[N: i32](Value: i32): i32 { return Value + N; } func select[B: bool](): i32 { comptime if (B) { return 2; } else { return 0; } } func main(): i32 { return offset::[20](20) + select::[true](); }", 42);
  }

  // Dependent value parameter types and defaults use preceding generic bindings in the definition environment.
  TEST(SemanticGenericFunctionTest, DependentDefaultsAndNamedArguments)
  {
    expectResult("func add[T: type = i32, N: T = 2](Value: T): T { return Value + N; } func main(): i32 { return add::[](18) + add::[N = 2, T = i32](20); }", 42);
  }

  // Pointer and fixed-array signatures substitute element types and compile-time lengths before IR generation.
  TEST(SemanticGenericFunctionTest, PointerAndArrayTypes)
  {
    expectResult("func read[T: type](Value: *T): T { return *Value; } func first[T: type, N: u64](Values: [T; N]): T { return Values[0]; } func main(): i32 { var Value: i32 = 20; return read::[i32](&Value) + first::[i32, 2]([22, 3]); }", 42);
  }

  // Nominal class arguments preserve value-copy behavior and member access in the instantiated body.
  TEST(SemanticGenericFunctionTest, NominalClassArguments)
  {
    expectResult("class Box { field Value: i32; func __init__(Initial: i32): void { this.Value = Initial; } }; func id[T: type](Value: T): T { return Value; } func main(): i32 { var Original = Box(42); var Copy = id::[Box](Original); Original.Value = 1; return Copy.Value; }", 42);
  }

  // Recursive and mutually recursive generic calls reuse signatures while bodies are still being checked.
  TEST(SemanticGenericFunctionTest, RecursiveAndMutuallyRecursiveInstances)
  {
    expectResult("func a[T: type](N: i32, Value: T): T { if (N == 0) { return Value; } return b::[T](N + -1, Value); } func b[T: type](N: i32, Value: T): T { return a::[T](N, Value); } func main(): i32 { return a::[i32](4, 42); }", 42);
  }

  // Compile-time calls and ordinary calls use the same closed IR while discarded static branches stay unchecked.
  TEST(SemanticGenericFunctionTest, ComptimeAndDiscardedBranches)
  {
    expectResult("func value[B: bool](): i32 { comptime if (B) { return 21; } else { return missing; } } comptime func add[T: type](Left: T, Right: T): T { return Left + Right; } func main(): i32 { return comptime(add::[i32](value::[true](), value::[true]())); }", 42);
  }

  // Unused generic bodies and unselected overload bodies are never lowered or diagnosed.
  TEST(SemanticGenericFunctionTest, ChecksOnlySelectedBodies)
  {
    expectResult("func unused[T: type](): i32 { return unknown; } func choose[T: type](Value: i32): i32 { return Value; } func choose[T: type](Value: bool): i32 { return unknown; } func main(): i32 { return choose::[i32](42); }", 42);
  }

  // A local ordinary function and its generic overload remain distinct name-lookup categories.
  TEST(SemanticGenericFunctionTest, CoexistsWithOrdinaryOverloads)
  {
    expectResult("func pick[T: type](Value: T): T { return Value; } func pick(Value: i32): i32 { return Value + 1; } func main(): i32 { return pick(20) + pick::[i32](21); }", 42);
  }

  // Nested generic definitions retain outer substitutions and do not capture the caller's shadowing locals.
  TEST(SemanticGenericFunctionTest, NestedGenericsAndLexicalLookup)
  {
    expectResult("func outer[T: type](Value: T): T { func inner[U: type](Item: U): U { return Item; } return inner::[T](Value); } func main(): i32 { var T: i32 = 0; return outer::[i32](42); }", 42);
  }

  // Value arguments evaluate at the application site; equal frozen values reuse one instance after mutation.
  TEST(SemanticGenericFunctionTest, FreezesValueArguments)
  {
    expectResult("func value[N: i32](): i32 { return N; } func main(): i32 { comptime var N: i32 = 20; var A = value::[N](); comptime { N = 22; } return A + value::[N](); }", 42);
  }

  // Invalid declarations and applications report ordinary diagnostics instead of unsupported-syntax ICEs.
  TEST(SemanticGenericFunctionTest, RejectsInvalidGenerics)
  {
    const std::pair<std::string_view, core::DiagnosticKind> Cases[] = {
        {"func f[T: type, T: type](): void {}", core::DiagnosticKind::SemanticDuplicateParameterName},
        {"func f[T: type](T: i32): void {}", core::DiagnosticKind::SemanticDuplicateParameterName},
        {"func f[T: type](): void;", core::DiagnosticKind::SemanticFunctionRequiresBody},
        {"import \"C\" func f[T: type](): void;", core::DiagnosticKind::SemanticInvalidGeneric},
        {"func f[T: type](): void {} func main(): void { f(); }", core::DiagnosticKind::SemanticGenericArguments},
        {"func f[T: type](): void {} func main(): void { f::[](); }", core::DiagnosticKind::SemanticGenericArguments},
        {"func f[T: type](): void {} func main(): void { f::[i32, i64](); }", core::DiagnosticKind::SemanticGenericArguments},
        {"func f[T: type](): void {} func main(): void { f::[Unknown = i32](); }", core::DiagnosticKind::SemanticGenericArguments},
        {"func f[N: i32](): void {} func main(): void { f::[true](); }", core::DiagnosticKind::SemanticGenericArguments},
        {"func f[N: *i32](): void {} func main(): void { f::[0](); }", core::DiagnosticKind::SemanticInvalidGeneric},
        {"func f(): void {} func main(): void { f::[i32](); }", core::DiagnosticKind::SemanticGenericArguments},
        {"comptime func f[T: type](): i32 { return 42; } func main(): i32 { return f::[i32](); }", core::DiagnosticKind::SemanticComptimeFunctionAtRuntime},
        {"func f[T: type](Value: T): T { return false; } func main(): i32 { return f::[i32](42); }", core::DiagnosticKind::SemanticTypeMismatch},
        {"func f[T: type](Value: T): T {} func main(): i32 { return f::[i32](42); }", core::DiagnosticKind::SemanticMissingReturn},
        {"func f[T: type](Value: T): T { return Value; } func main(): i32 { return f::[i32](); }", core::DiagnosticKind::SemanticArgumentCount},
        {"func f[T: type](Value: i32): i32 { return 1; } func f[U: type](Value: U): i32 { return 2; } func main(): i32 { return f::[i32](42); }", core::DiagnosticKind::SemanticAmbiguousName},
    };
    for (const auto &[Source, Diagnostic] : Cases)
    {
      SCOPED_TRACE(Source);
      GenericAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
      EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, Diagnostic);
    }
  }

  // Expanding a new value specialization on every recursive call fails within the shared resource budget.
  TEST(SemanticGenericFunctionTest, LimitsUnboundedInstantiation)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_GENERIC_DEPTH_LIMIT");
    ASSERT_TRUE(Limit.set("8"));
    GenericAnalysis Input("func expand[N: i32](): i32 { return expand::[N + 1](); } func main(): i32 { return expand::[0](); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Input.Diagnostics.diagnostics().back().Kind, core::DiagnosticKind::SemanticGenericLimit);
  }
  // Generic environments freeze visible compile-time values before later writes and caller shadowing.
  TEST(SemanticGenericFunctionTest, CapturesDefinitionEnvironment)
  {
    expectResult("comptime var Base: i32 = 40; func add[T: type](Value: T): T { return Value + Base; } comptime { Base = 100; } func main(): i32 { var Base: i32 = 200; return add::[i32](2); }", 42);
    expectResult("func main(): i32 { comptime var Base: i32 = 40; func add[T: type](Value: T): T { return Value + Base; } comptime { Base = 100; } return add::[i32](2); }", 42);
  }

  // Named arguments preserve source-order effects and share evaluation across overload candidates.
  TEST(SemanticGenericFunctionTest, EvaluatesArgumentsOnceInSourceOrder)
  {
    expectResult("func value[A: i32, B: i32](X: i32): i32 { return A + B + X; } func value[A: i32, B: i32](X: bool): i32 { return 0; } func main(): i32 { comptime var Count: i32 = 0; var Result = value::[B = ++Count, A = ++Count](37); return Result + Count; }", 42);
  }

  // The instance-count limit covers all declarations even when each instantiation is shallow.
  TEST(SemanticGenericFunctionTest, LimitsTotalInstances)
  {
    core::test::ScopedEnvironmentVariable Limit("INK_SEMANTIC_GENERIC_INSTANCE_LIMIT");
    ASSERT_TRUE(Limit.set("1"));
    GenericAnalysis Input("func value[N: i32](): i32 { return N; } func main(): i32 { return value::[20]() + value::[22](); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Input.Diagnostics.diagnostics().back().Kind, core::DiagnosticKind::SemanticGenericLimit);
  }

  // Renaming generic parameters or changing only the return type cannot create another overload.
  TEST(SemanticGenericFunctionTest, RejectsDuplicateGenericSignatures)
  {
    GenericAnalysis Input("func id[T: type](Value: T): T { return Value; } func id[U: type](Other: U): i32 { return 0; }");
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticDuplicateName);
  }

  // Nested generics cannot capture runtime parameters, and runtime locals cannot become generic arguments.
  TEST(SemanticGenericFunctionTest, RejectsRuntimeDependencies)
  {
    GenericAnalysis Capture("func outer(Value: i32): i32 { func inner[T: type](): i32 { return Value; } return inner::[i32](); }");
    EXPECT_EQ(Capture.analyze(), nullptr);
    ASSERT_FALSE(Capture.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Capture.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticInvalidCapture);
    GenericAnalysis Argument("func value[N: i32](): i32 { return N; } func main(): i32 { var N: i32 = 42; return value::[N](); }");
    EXPECT_EQ(Argument.analyze(), nullptr);
    EXPECT_FALSE(Argument.Diagnostics.diagnostics().empty());
  }
  // Concrete instances preserve their symbols and unused argument types through both IR archive formats.
  TEST(SemanticGenericFunctionTest, RoundTripsGenericIR)
  {
    GenericAnalysis Input("func value[T: type, N: i32](): i32 { return N; } func main(): i32 { return value::[u128, 20]() + value::[bool, 22](); }");
    auto *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    const parser::ParseResult *Source = &Input.Parsed;
    for (bool Binary : {false, true})
    {
      const auto Encoded = Binary ? ir::serializeModuleBinary(*Module, {}, {&Source, 1}) : ir::serializeModuleText(*Module, {}, {&Source, 1});
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      core::CompilationContext Compilation;
      ir::IRContext Restored(Compilation);
      const auto Decoded = Binary ? ir::deserializeModuleBinary(Restored, Encoded.Bytes) : ir::deserializeModuleText(Restored, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const ir::Function *Main = nullptr;
      std::vector<std::string> Symbols;
      for (const auto &Value : Decoded.ModuleValue->entryBlock().values())
      {
        if (!ir::Function::classof(Value.get()))
        {
          continue;
        }
        const auto &Function = static_cast<const ir::Function &>(*Value);
        if (Restored.namePool().text(Function.name()) == "main")
        {
          Main = &Function;
        }
        else
        {
          EXPECT_EQ(Function.genericArgumentTypes().size(), 2U);
          EXPECT_EQ(ir::functionSymbol(Function).Name, Function.genericIdentity());
          Symbols.push_back(Function.genericIdentity());
        }
      }
      ASSERT_NE(Main, nullptr);
      ASSERT_EQ(Symbols.size(), 2U);
      EXPECT_NE(Symbols[0], Symbols[1]);
      execution::ExecutionEngine Engine(Restored);
      const auto Result = Engine.execute(*Main);
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.integer().bits().words().front(), 42U);
    }
  }

  // Aliases and qualified applications use the defining module's private helpers despite caller shadowing.
  TEST(SemanticGenericFunctionTest, ImportsGenericDefinitions)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Library = parser::parse(Frontend, tokenizer::tokenize(Frontend, "private func helper(Value: i32): i32 { return Value + 1; } func add[T: type](Value: T): T { return helper(Value); }"));
    auto App = parser::parse(Frontend, tokenizer::tokenize(Frontend, "from library import add as alias; import library as lib; func helper(Value: i32): i32 { return 0; } func main(): i32 { return alias::[i32](20) + lib.add::[i32](20); }"));
    ASSERT_TRUE(Library.succeeded());
    ASSERT_TRUE(App.succeeded());
    SemanticContext Context(Compilation);
    const Analyzer::ModuleInput Inputs[] = {
        {"app", &App},
        {"library", &Library},
    };
    auto *Module = Analyzer{}.analyzeModules(Context, Inputs, "app");
    ASSERT_NE(Module, nullptr);
    NameResolver Resolver(Context);
    const auto *Main = static_cast<const ir::Function *>(Resolver.lookupMember(*Module, Context.namePool().find("main"))->targets().front());
    const auto Result = Context.comptimeState().Engine.execute(*Main);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits().words().front(), 42U);
    ASSERT_EQ(Context.genericState().Definitions.size(), 1U);
    EXPECT_EQ(Context.genericState().Definitions.begin()->second.Instances.size(), 1U);
  }
  // Full-width value arguments retain signedness and all 128 bits in the cache and linkage identity.
  TEST(SemanticGenericFunctionTest, PreservesIntegerValueBoundaries)
  {
    expectResult("func value[T: type, N: T](): T { return N; } func main(): i32 { if (value::[i8, -128]() == -128 && value::[u128, 340282366920938463463374607431768211455]() == 340282366920938463463374607431768211455) { return 42; } return 0; }", 42);
    GenericAnalysis Input("func value[N: i8](): i8 { return N; } func main(): i8 { return value::[128](); }");
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticGenericArguments);
  }

  // A recursive signature request is diagnosed before a pending instance can acquire a body.
  TEST(SemanticGenericFunctionTest, RejectsSignatureCycles)
  {
    GenericAnalysis Input("func size[N: i32](): [i32; size::[N]()] { return []; } func main(): void { size::[0](); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticGenericCycle);
  }

  // Overload probing cannot execute omitted defaults with side effects in rejected candidates.
  TEST(SemanticGenericFunctionTest, RejectsEffectfulOverloadDefaults)
  {
    GenericAnalysis Input("comptime func next(): i32 { return 1; } func value[N: i32 = next()](X: i32): i32 { return N; } func value[N: i32 = next()](X: bool): i32 { return N; } func main(): i32 { return value::[](1); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_FALSE(Input.Diagnostics.diagnostics().empty());
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::SemanticGenericArguments);
  }

  // Qualified class types remain types inside pointer and array generic arguments across modules.
  TEST(SemanticGenericFunctionTest, SupportsQualifiedCompositeTypeArguments)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Library = parser::parse(Frontend, tokenizer::tokenize(Frontend, "class Box { field Value: i32; func __init__(Value: i32): void { this.Value = Value; } }; func id[T: type](Value: T): T { return Value; }"));
    auto App = parser::parse(Frontend, tokenizer::tokenize(Frontend, "import library as lib; func main(): i32 { var Box = lib.Box(42); var Pointer = lib.id::[*lib.Box](&Box); var Values = lib.id::[[lib.Box; 1]]([Box]); return (*Pointer).Value + Values[0].Value + -42; }"));
    ASSERT_TRUE(Library.succeeded());
    ASSERT_TRUE(App.succeeded());
    SemanticContext Context(Compilation);
    const Analyzer::ModuleInput Inputs[] = {
        {"app", &App},
        {"library", &Library},
    };
    auto *Module = Analyzer{}.analyzeModules(Context, Inputs, "app");
    ASSERT_NE(Module, nullptr);
    NameResolver Resolver(Context);
    const auto *Main = static_cast<const ir::Function *>(Resolver.lookupMember(*Module, Context.namePool().find("main"))->targets().front());
    const auto Result = Context.comptimeState().Engine.execute(*Main);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits().words().front(), 42U);
  }

  // Private generic declarations cannot be accessed through either import syntax.
  TEST(SemanticGenericFunctionTest, RejectsPrivateGenericImports)
  {
    for (const auto Source : {"from library import hidden; func main(): i32 { return hidden::[i32](42); }", "import library as lib; func main(): i32 { return lib.hidden::[i32](42); }"})
    {
      SCOPED_TRACE(Source);
      core::CompilationContext Compilation;
      core::FrontendContext Frontend(Compilation);
      auto Library = parser::parse(Frontend, tokenizer::tokenize(Frontend, "private func hidden[T: type](Value: T): T { return Value; }"));
      auto App = parser::parse(Frontend, tokenizer::tokenize(Frontend, Source));
      ASSERT_TRUE(Library.succeeded());
      ASSERT_TRUE(App.succeeded());
      core::CollectingDiagnosticConsumer Diagnostics;
      Compilation.diagnosticEngine().addConsumer(Diagnostics);
      SemanticContext Context(Compilation);
      const Analyzer::ModuleInput Inputs[] = {
          {"library", &Library},
          {"app", &App},
      };
      EXPECT_EQ(Analyzer{}.analyzeModules(Context, Inputs, "app"), nullptr);
      ASSERT_FALSE(Diagnostics.diagnostics().empty());
      EXPECT_EQ(Diagnostics.diagnostics().front().classification(), core::DiagnosticClass::User);
    }
  }
} // namespace ink::semantic::test
