#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "ink/semantic/operator_method.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>

namespace ink::semantic::test
{
  namespace
  {
    class ClassAnalysis final
    {
      public:
        explicit ClassAnalysis(std::string_view Source)
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ir::Module *analyze()
        {
          const Analyzer::ModuleInput Input{"main", &Parsed};
          return Analyzer{}.analyzeModules(Context, std::span<const Analyzer::ModuleInput>(&Input, 1), "main");
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    void expectClassExecution(std::string_view Source, std::uint64_t Expected)
    {
      SCOPED_TRACE(Source);
      ClassAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      ir::Module *Module = Input.analyze();
      ASSERT_NE(Module, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      const auto *Binding = NameResolver(Input.Context).lookupMember(*Module, Input.Context.namePool().find("Entry"));
      ASSERT_NE(Binding, nullptr);
      ASSERT_EQ(Binding->targets().size(), 1U);
      const auto *Function = static_cast<const ir::Function *>(Binding->targets().front());
      execution::ExecutionEngine Engine(Input.Context.irContext());
      {
        const auto Result = Engine.execute(*Function);
        ASSERT_TRUE(Result) << static_cast<int>(Result.Status);
        ASSERT_EQ(Result.Value.kind(), execution::ExecutionValueKind::Integer);
        EXPECT_EQ(Result.Value.integer().bits().words().front(), Expected);
      }
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
      EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
    }

    void expectClassDiagnostic(std::string_view Source, core::DiagnosticKind Kind)
    {
      SCOPED_TRACE(Source);
      ClassAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      const auto Diagnostics = Input.Diagnostics.diagnostics();
      EXPECT_TRUE(std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == Kind;
      }));
    }
  } // namespace

  // Positional construction fills omitted fields from defaults and supports writable field projections.
  TEST(SemanticClassTest, ConstructsDefaultsAndReadsWritesFields)
  {
    expectClassExecution("class P { field X: i32; field Y: i32 = 2; }; func Entry(): i32 { var A = P(38); A.X = A.X + 2; return A.X + A.Y; }", 42);
    expectClassExecution("class Empty {}; func Entry(): i32 { var E = Empty(); return 42; }", 42);
  }

  // Methods receive the same writable object through this and can call later-declared methods.
  TEST(SemanticClassTest, MutatesThisAndCallsOtherMethods)
  {
    expectClassExecution("class P { field X: i32; func run(N: i32): i32 { this.set(N); return this.X; } func set(N: i32): void { this.X = this.X + N; } }; func Entry(): i32 { var A = P(20); var V = A.run(22); return A.X; }", 42);
    expectClassExecution("class P { field X: i32; func get(): i32 { return this->X; } }; func Entry(): i32 { var A = P(42); var Ptr = &A; return Ptr->get(); }", 42);
  }

  // Value parameters and const objects allow field reads while ordinary copying preserves independent values.
  TEST(SemanticClassTest, CopiesParametersReturnsAndConstReads)
  {
    expectClassExecution("class P { field X: i32; }; func copy(A: P): P { return A; } func read(A: P): i32 { return A.X; } func Entry(): i32 { var A = P(42); const B = copy(A); A.X = 9; return read(B); }", 42);
  }

  // Nested class and array projections share root storage, including whole-object assignment through an existing field address.
  TEST(SemanticClassTest, PreservesNestedFieldAddresses)
  {
    expectClassExecution("class P { field X: i32; }; class Box { field Value: P; field Items: [i32; 2]; }; func Entry(): i32 { var B = Box(P(1), [2, 3]); var Ptr = &B.Value.X; B = Box(P(40), [1, 2]); B.Items[1] = 2; return *Ptr + B.Items[1]; }", 42);
    expectClassExecution("class P { field X: i32; }; func Entry(): i32 { var A = [P(1), P(2)]; A[1].X = 42; return A[1].X; }", 42);
  }

  // Temporary method receivers materialize once and keep their mutable changes local to the temporary.
  TEST(SemanticClassTest, CallsMethodsOnTemporaryValues)
  {
    expectClassExecution("class P { field X: i32; func add(N: i32): i32 { this.X = this.X + N; return this.X; } }; func Entry(): i32 { return P(40).add(2); }", 42);
  }

  // Operator methods use a value snapshot, accept const operands and require boolean comparison results.
  TEST(SemanticClassTest, DispatchesDunderOperatorsOnSnapshots)
  {
    expectClassExecution("class P { field X: i32; func __add__(Other: P): P { this.X = this.X + Other.X; return P(this.X); } func __eq__(Other: P): bool { return this.X == Other.X; } func __neg__(): i32 { return this.X; } }; func Entry(): i32 { const A = P(20); const B = A + P(22); if (B == P(42)) { return -B; } return 0; }", 42);
    expectClassExecution("class P { field X: i32; func __sub__(N: i32): i32 { return this.X + N; } func __mul__(N: i32): i32 { return this.X + N; } }; func Entry(): i32 { const A = P(40); return A * 2; }", 42);
    expectClassExecution("class P { field X: i64; func __add__(N: i64): i64 { return this.X + N; } }; comptime var Result = P(40) + 2; func Entry(): i64 { return Result; }", 42);
  }

  // Compile-time object fields and methods use the same class operations as runtime functions.
  TEST(SemanticClassTest, EvaluatesCompileTimeClassesAndMethods)
  {
    expectClassExecution("class P { field X: i32 = 20; func add(N: i32): i32 { this.X = this.X + N; return this.X; } }; comptime var A = P(); comptime { A.X = 40; } comptime var Result = A.add(2); func Entry(): i32 { return Result; }", 42);
    expectClassExecution("class P { field X: i32; }; comptime func make(): P { var A = P(40); A.X = A.X + 2; return A; } func Entry(): i32 { const A = comptime make(); return A.X; }", 42);
    expectClassExecution("class P { field X: i32; }; class B { field Items: [P; 2]; }; comptime var A = B([P(1), P(2)]); comptime { A.Items[1].X = 42; } func Entry(): i32 { return A.Items[1].X; }", 42);
    expectClassExecution("class P { field X: i32; func add(N: i32): void { this.X = this.X + N; } }; class B { field Items: [P; 2]; }; comptime var A = B([P(1), P(40)]); comptime A.Items[1].add(2); func Entry(): i32 { return A.Items[1].X; }", 42);
  }

  // Class names are predeclared before function signatures, and pointers permit recursive nominal types.
  TEST(SemanticClassTest, PredeclaresTypesAndSupportsPointerRecursion)
  {
    expectClassExecution("func read(A: P): i32 { return A.X; } class P; class P { field X: i32; }; class Link { field Next: *Link; }; func Entry(): i32 { return read(P(42)); }", 42);
    expectClassExecution("class Outer { class Inner { field X: i32; }; field V: Inner; }; func Entry(): i32 { var A = Outer(Outer.Inner(42)); return A.V.X; }", 42);
  }

  // Private members are accessible within their class, but not through external construction, reads or method calls.
  TEST(SemanticClassTest, EnforcesPrivateMembers)
  {
    expectClassExecution("class P { private field X: i32 = 40; private func secret(): i32 { return this.X + 2; } func read(): i32 { return this.secret(); } }; func Entry(): i32 { var A = P(); return A.read(); }", 42);
    expectClassDiagnostic("class P { private field X: i32 = 1; }; func Entry(): i32 { return P().X; }", core::DiagnosticKind::SemanticInvalidMember);
    expectClassDiagnostic("class P { private field X: i32; }; func Entry(): i32 { var A = P(1); return 0; }", core::DiagnosticKind::SemanticInvalidConstruction);
    expectClassDiagnostic("class P { private func hidden(): i32 { return 1; } }; func Entry(): i32 { return P().hidden(); }", core::DiagnosticKind::SemanticInvalidMember);
  }

  // Readonly receivers and this shadowing are rejected before methods can mutate unintended storage.
  TEST(SemanticClassTest, RejectsReadonlyReceiversAndThisShadowing)
  {
    expectClassDiagnostic("class P { func read(): i32 { return 1; } }; func Entry(): i32 { const A = P(); return A.read(); }", core::DiagnosticKind::SemanticInvalidMember);
    expectClassDiagnostic("class P { func read(this: i32): i32 { return 1; } };", core::DiagnosticKind::SemanticDuplicateParameterName);
    expectClassDiagnostic("class P { func read(): i32 { var this = 1; return this; } };", core::DiagnosticKind::SemanticDuplicateName);
    expectClassDiagnostic("class P { func read(): i32 { func nested(this: i32): i32 { return this; } return 1; } };", core::DiagnosticKind::SemanticDuplicateParameterName);
    expectClassDiagnostic("class P { func read(): i32 { func this(): i32 { return 1; } return 1; } };", core::DiagnosticKind::SemanticDuplicateName);
  }

  // Layout cycles, incomplete by-value fields and invalid construction are diagnosed as source errors.
  TEST(SemanticClassTest, RejectsInvalidLayoutsAndConstruction)
  {
    expectClassDiagnostic("class P { field Self: P; };", core::DiagnosticKind::SemanticInvalidClass);
    expectClassDiagnostic("class A; class B { field Value: A; };", core::DiagnosticKind::SemanticInvalidClass);
    expectClassDiagnostic("class P { field X: i32; }; func Entry(): i32 { var A = P(); return 0; }", core::DiagnosticKind::SemanticInvalidConstruction);
    expectClassDiagnostic("class P { field X: i32; field X: i32; };", core::DiagnosticKind::SemanticDuplicateName);
    expectClassDiagnostic("class P[T: type] {};", core::DiagnosticKind::SemanticInvalidClass);
    expectClassDiagnostic("class A { field BValue: B; }; class B { field AValue: A; };", core::DiagnosticKind::SemanticInvalidClass);
    expectClassDiagnostic("class A; func read(Value: A): void {}", core::DiagnosticKind::SemanticInvalidClass);
    expectClassDiagnostic("class P { func f(): i32 { return 1; } func f(): i32 { return 2; } };", core::DiagnosticKind::SemanticDuplicateName);
    expectClassDiagnostic("class P { field f: i32; func f(): i32 { return 1; } };", core::DiagnosticKind::SemanticDuplicateName);
    expectClassDiagnostic("class P { func __iadd__(Other: P): P { return Other; } }; func Entry(): i32 { var A = P(); A += P(); return 0; }", core::DiagnosticKind::SemanticInvalidMember);
    expectClassDiagnostic("class P { func __eq__(Other: P): i32 { return 1; } }; func Entry(): bool { return P() == P(); }", core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Both analysis entries reject unresolved nominal types reachable through signatures, storage or nested pointer fields.
  TEST(SemanticClassTest, RequiresCompleteRuntimeTypeGraphs)
  {
    constexpr std::string_view Invalid[] = {
        "class Node; func take(P: *Node): i32 { return 0; } func Entry(): i32 { return 42; }",
        "class Node; func identity(P: *Node): *Node { return P; }",
        "class Node; func Entry(): i32 { var P: *Node; return 42; }",
        "class Node; func Entry(): i32 { var Items: [*Node; 2]; return 42; }",
        "class Node; class Wrapper { field Link: *Node; }; func take(Value: Wrapper): void {}",
    };
    for (const std::string_view Source : Invalid)
    {
      for (const bool Modules : {false, true})
      {
        SCOPED_TRACE(Source);
        SCOPED_TRACE(Modules);
        ClassAnalysis Input(Source);
        ASSERT_TRUE(Input.Parsed.succeeded());
        EXPECT_EQ(Modules ? Input.analyze() : Analyzer{}.analyze(Input.Context, Input.Parsed), nullptr);
        const auto Diagnostics = Input.Diagnostics.diagnostics();
        EXPECT_TRUE(std::any_of(Diagnostics.begin(), Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
        {
          return Diagnostic.Kind == core::DiagnosticKind::SemanticInvalidClass;
        }));
      }
    }
    constexpr std::string_view Valid[] = {
        "class Node; func Entry(): i32 { return 42; }",
        "class Node; func take(P: *Node): i32 { return 0; } class Node { field Next: *Node; }; func Entry(): i32 { return 42; }",
    };
    for (const std::string_view Source : Valid)
    {
      for (const bool Modules : {false, true})
      {
        SCOPED_TRACE(Source);
        SCOPED_TRACE(Modules);
        ClassAnalysis Input(Source);
        ASSERT_TRUE(Input.Parsed.succeeded());
        EXPECT_NE(Modules ? Input.analyze() : Analyzer{}.analyze(Input.Context, Input.Parsed), nullptr);
        EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
      }
    }
  }

  // In-place dunder names remain distinct from snapshot operations, and short-circuit operators expose no overload hook.
  TEST(SemanticClassTest, DefinesStableOperatorMethodMapping)
  {
    EXPECT_EQ(operatorMethodName(tokenizer::TokenKind::Plus, OperatorMethodKind::Binary), "__add__");
    EXPECT_EQ(operatorMethodName(tokenizer::TokenKind::PlusAssign, OperatorMethodKind::InPlace), "__iadd__");
    EXPECT_EQ(operatorMethodName(tokenizer::TokenKind::Minus, OperatorMethodKind::Unary), "__neg__");
    EXPECT_TRUE(operatorMethodName(tokenizer::TokenKind::AmpAmp, OperatorMethodKind::Binary).empty());
    for (const auto &Mapping : OperatorMethodMappings)
    {
      EXPECT_EQ(operatorMethodName(Mapping.Operator, Mapping.Kind), Mapping.Name);
      EXPECT_EQ(operatorMethodEnabled(Mapping.Operator, Mapping.Kind), Mapping.Kind == OperatorMethodKind::Binary || Mapping.Kind == OperatorMethodKind::Unary);
    }
    EXPECT_EQ(operatorMethodName(tokenizer::TokenKind::Plus, OperatorMethodKind::Reflected), "__radd__");
    EXPECT_EQ(specialMethodName(SpecialMethodKind::GetItem), "__getitem__");
    EXPECT_EQ(specialMethodName(SpecialMethodKind::SetItem), "__setitem__");
    EXPECT_EQ(specialMethodName(SpecialMethodKind::Call), "__call__");
    EXPECT_EQ(specialMethodName(SpecialMethodKind::ToBool), "__bool__");
    for (const auto &Mapping : SpecialMethodMappings)
    {
      EXPECT_EQ(specialMethodName(Mapping.Kind), Mapping.Name);
      EXPECT_FALSE(specialMethodEnabled(Mapping.Kind));
    }
  }

  // Receiver and argument side effects execute once and in source order; overloads use explicit parameter types.
  TEST(SemanticClassTest, PreservesMethodEvaluationOrderAndOverloads)
  {
    expectClassExecution("class P { field X: i32; func add(N: i32): i32 { this.X = this.X + N; return this.X; } func add(N: bool): i32 { return 0; } }; func receiver(C: *i32, P: *P): *P { *C = *C + 1; return P; } func argument(C: *i32): i32 { *C = *C + 1; return *C; } func Entry(): i32 { var Count = 0; var A = P(38); var Result = receiver(&Count, &A)->add(argument(&Count)); return Result + Count; }", 42);
  }

  // Imported class names can appear in signatures and preserve their defining module's methods and private checks.
  TEST(SemanticClassTest, ImportsClassTypesAcrossModules)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Library = parser::parse(Frontend, tokenizer::tokenize(Frontend, "class P { field X: i32 = 40; func add(N: i32): i32 { this.X = this.X + N; return this.X; } };"));
    auto Main = parser::parse(Frontend, tokenizer::tokenize(Frontend, "from library import P; func read(A: P): i32 { return A.X; } func Entry(): i32 { var A = P(); A.add(2); return read(A); }"));
    ASSERT_TRUE(Library.succeeded());
    ASSERT_TRUE(Main.succeeded());
    SemanticContext Context(Compilation);
    const Analyzer::ModuleInput Inputs[] = {{"main", &Main}, {"library", &Library}};
    auto *Module = Analyzer{}.analyzeModules(Context, Inputs, "main");
    ASSERT_NE(Module, nullptr);
    const auto *Binding = NameResolver(Context).lookupMember(*Module, Context.namePool().find("Entry"));
    ASSERT_NE(Binding, nullptr);
    execution::ExecutionEngine Engine(Context.irContext());
    const auto Result = Engine.execute(static_cast<const ir::Function &>(*Binding->targets().front()));
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits().words().front(), 42U);
  }
} // namespace ink::semantic::test
