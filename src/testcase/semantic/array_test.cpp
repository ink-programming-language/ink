#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "../core/environment_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::semantic::test
{
  using namespace ir;
  using namespace execution;

  namespace
  {
    class ArrayAnalysis final
    {
      public:
        explicit ArrayAnalysis(std::string_view Source)
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        Module *analyze()
        {
          return Analyzer{}.analyze(Context, Parsed);
        }

        const Function *entry(Module &Owner)
        {
          const auto *Binding = NameResolver(Context).lookupMember(Owner, Context.namePool().find("Entry"));
          return Binding && Binding->targets().size() == 1 && Function::classof(Binding->targets().front()) ? static_cast<const Function *>(Binding->targets().front()) : nullptr;
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    void expectArrayExecution(std::string_view Source, std::uint64_t Expected, std::uint32_t Width = 32)
    {
      SCOPED_TRACE(Source);
      ArrayAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      Module *Owner = Input.analyze();
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      const Function *Entry = Input.entry(*Owner);
      ASSERT_NE(Entry, nullptr);
      ExecutionEngine Engine(Input.Context.irContext());
      {
        const auto Result = Engine.execute(*Entry);
        ASSERT_TRUE(Result) << static_cast<int>(Result.Status);
        ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Integer);
        EXPECT_EQ(Result.Value.integer().bitWidth(), Width);
        EXPECT_EQ(Result.Value.integer().bits().words().front(), Expected);
      }
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
      EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
    }

    void expectArrayDiagnostic(std::string_view Source, core::DiagnosticKind Kind)
    {
      SCOPED_TRACE(Source);
      ArrayAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      const auto Entries = Input.Diagnostics.diagnostics();
      EXPECT_TRUE(std::any_of(Entries.begin(), Entries.end(), [Kind](const core::Diagnostic &Entry)
      {
        return Entry.Kind == Kind;
      }));
    }

    void expectArrayExecutionFailure(std::string_view Source, ExecutionStatus Expected)
    {
      SCOPED_TRACE(Source);
      ArrayAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      Module *Owner = Input.analyze();
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      const Function *Entry = Input.entry(*Owner);
      ASSERT_NE(Entry, nullptr);
      ExecutionEngine Engine(Input.Context.irContext());
      EXPECT_EQ(Engine.execute(*Entry).Status, Expected);
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
      EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
    }
  } // namespace

  // Unconstrained integer arrays infer i32 and permit first, last and temporary-array reads.
  TEST(SemanticArrayTest, InfersIntegerArraysAndReadsLiteralElements)
  {
    expectArrayExecution("func Entry(): i32 { var A = [20, 1, 22]; return A[0] + A[2]; }", 42);
    expectArrayExecution("func Entry(): i32 { return [10, 42, 99][1]; }", 42);
  }

  // An explicit element type applies to every integer literal, including nested and negative literals.
  TEST(SemanticArrayTest, ContextuallyTypesArrayElements)
  {
    expectArrayExecution("func Entry(): u8 { var A: [u8; 2] = [0, 255]; return A[1]; }", 255, 8);
    expectArrayExecution("func Entry(): i8 { var A: [i8; 2] = [-128, 127]; return A[0]; }", 128, 8);
    expectArrayExecution("func Entry(): u8 { var A: [[u8; 2]; 2] = [[1, 2], [3, 42]]; return A[1][1]; }", 42, 8);
  }

  // A typed nonliteral element supplies the type of preceding deferred integer literals.
  TEST(SemanticArrayTest, InfersElementTypeFromTypedValue)
  {
    expectArrayExecution("func Entry(): u8 { var Value: u8 = 42; var A = [0, Value]; return A[1]; }", 42, 8);
    expectArrayExecution("comptime var Value: u8 = 42; comptime var A = [0, Value]; func Entry(): u8 { return A[1]; }", 42, 8);
  }

  // Boolean arrays retain their element type rather than coercing values to integers.
  TEST(SemanticArrayTest, SupportsBooleanElements)
  {
    expectArrayExecution("func Entry(): i32 { var A = [false, true]; if (A[1]) { return 42; } return 0; }", 42);
  }

  // A repetition count is evaluated at compile time and repeated values retain the requested element width.
  TEST(SemanticArrayTest, BuildsRepeatedArraysWithCompileTimeLength)
  {
    expectArrayExecution("func Entry(): i32 { var A = [42; 3]; return A[2]; }", 42);
    expectArrayExecution("comptime const Count: u8 = 2; func Entry(): u8 { var A: [u8; Count + 1] = [42; Count + 1]; return A[2]; }", 42, 8);
  }

  // An empty literal uses context, while zero repetition can infer its element type from its value.
  TEST(SemanticArrayTest, SupportsZeroLengthArrays)
  {
    expectArrayExecution("func Empty(): [u8; 0] { return []; } func Entry(): i32 { var A: [u8; 0] = []; var B = [1; 0]; var C = Empty(); return 42; }", 42);
    expectArrayExecution("func Entry(): i32 { var A = [[0; 0]; 2]; var B = A[1]; return 42; }", 42);
  }

  // Runtime repeated values execute once even when the array contains several copies or no elements.
  TEST(SemanticArrayTest, EvaluatesRepeatedValueExactlyOnce)
  {
    expectArrayExecution("func Next(Count: *i32): i32 { *Count = *Count + 1; return 10; } func Entry(): i32 { var Count = 0; var A = [Next(&Count); 4]; return A[0] + A[1] + A[2] + A[3] + Count; }", 41);
    expectArrayExecution("func Next(Count: *i32): i32 { *Count = *Count + 1; return 10; } func Entry(): i32 { var Count = 0; var A = [Next(&Count); 0]; return Count; }", 1);
  }

  // Literal elements execute in source order and each element is captured when it is evaluated.
  TEST(SemanticArrayTest, EvaluatesLiteralElementsInOrder)
  {
    expectArrayExecution("func Next(Count: *i32): i32 { *Count = *Count + 1; return *Count; } func Entry(): i32 { var Count = 0; var A = [Next(&Count), Next(&Count), Next(&Count)]; return A[0] + A[1] + A[2] + Count; }", 9);
  }

  // Indexed writes and aliases to indexed elements refer to the same underlying array storage.
  TEST(SemanticArrayTest, MutatesElementsThroughIndexAndAddress)
  {
    expectArrayExecution("func Store(Value: *i32): void { *Value = 40; } func Entry(): i32 { var A = [0, 0]; Store(&A[1]); var P = &(A[1]); *P = A[1] + 2; return A[1]; }", 42);
    expectArrayExecution("func Entry(): i32 { var A = [[1, 2], [3, 4]]; var P = &A[1][0]; *P = 40; A[0][1] = 2; return A[1][0] + A[0][1]; }", 42);
  }

  // An index-producing call executes once before the assignment RHS and is reused for the final store.
  TEST(SemanticArrayTest, EvaluatesAssignmentIndexBeforeRightOperand)
  {
    expectArrayExecution("func Pick(Count: *i32): i32 { *Count = *Count + 1; return 1; } func Next(Count: *i32): i32 { *Count = *Count + 10; return 31; } func Entry(): i32 { var Count = 0; var A = [0, 0]; A[Pick(&Count)] = Next(&Count); return A[1] + Count; }", 42);
  }

  // Reading an element observes mutations performed by the index expression before the element load.
  TEST(SemanticArrayTest, LoadsElementAfterEvaluatingIndex)
  {
    expectArrayExecution("func Pick(Value: *i32): i32 { *Value = 42; return 0; } func Entry(): i32 { var A = [0]; return A[Pick(&A[0])]; }", 42);
  }

  // Initializing or assigning an array copies its value; later element writes do not change previous snapshots.
  TEST(SemanticArrayTest, PreservesIndependentArrayCopies)
  {
    expectArrayExecution("func Entry(): i32 { var A = [10, 20]; var B = A; A[0] = 100; B[1] = 32; return B[0] + B[1]; }", 42);
    expectArrayExecution("func Entry(): i32 { var A = [[10], [20]]; var B = A; A[0][0] = 100; B[1][0] = 32; return B[0][0] + B[1][0]; }", 42);
    expectArrayExecution("func Entry(): i32 { var A = [10, 20]; var B = [0, 0]; B = A; B[0] = 22; return A[0] + B[0] + 10; }", 42);
  }

  // Whole-array reassignment updates existing element aliases instead of changing their storage identity.
  TEST(SemanticArrayTest, PreservesElementAliasAcrossWholeArrayAssignment)
  {
    expectArrayExecution("func Entry(): i32 { var A = [0, 0]; var P = &A[1]; A = [1, 42]; return *P; }", 42);
  }

  // By-value parameters, returned arrays and temporary result indexing preserve full aggregate values.
  TEST(SemanticArrayTest, PassesAndReturnsArraysByValue)
  {
    expectArrayExecution("func Make(Value: i32): [i32; 2] { return [1, Value]; } func Pick(A: [i32; 2]): i32 { return A[1]; } func Entry(): i32 { var A = Make(42); return Pick(A); }", 42);
    expectArrayExecution("func Make(): [i32; 2] { return [1, 42]; } func Entry(): i32 { return Make()[1]; }", 42);
    expectArrayExecution("func Pick(A: [u8; 2]): u8 { return A[1]; } func Entry(): u8 { return Pick([1, 255]); }", 255, 8);
  }

  // The CLI module pipeline accepts array signature lengths computable before module initialization.
  TEST(SemanticArrayTest, PredeclaresArraySignaturesInModulePipeline)
  {
    ArrayAnalysis Input("func Make(): [i32; 1 + 1] { return [0, 42]; } func Entry(): i32 { return Make()[1]; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    const Analyzer::ModuleInput Modules[] = {{"main", &Input.Parsed}};
    Module *Owner = Analyzer{}.analyzeModules(Input.Context, Modules, "main");
    ASSERT_NE(Owner, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.entry(*Owner);
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const auto Result = Engine.execute(*Entry);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits().words().front(), 42U);
  }

  // Module compile-time bindings are unavailable during signature predeclaration and yield a user diagnostic.
  TEST(SemanticArrayTest, DiagnosesModuleBindingInPredeclaredArrayLength)
  {
    ArrayAnalysis Input("comptime const Count = 2; func Entry(A: [i32; Count]): i32 { return A[0]; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    const Analyzer::ModuleInput Modules[] = {{"main", &Input.Parsed}};
    EXPECT_EQ(Analyzer{}.analyzeModules(Input.Context, Modules, "main"), nullptr);
    const auto Entries = Input.Diagnostics.diagnostics();
    EXPECT_TRUE(std::any_of(Entries.begin(), Entries.end(), [](const core::Diagnostic &Entry)
    {
      return Entry.Kind == core::DiagnosticKind::SemanticUnknownName;
    }));
  }

  // Pointer-to-array indirection retains array element locations and supports nested address/dereference pairs.
  TEST(SemanticArrayTest, IndexesDereferencedArrayPointer)
  {
    expectArrayExecution("func Set(A: *[i32; 2]): void { (*A)[1] = 42; } func Entry(): i32 { var A = [0, 0]; Set(&A); return (*&A)[1]; }", 42);
  }

  // Compile-time array bindings support reads, nested writes, compound updates and ordinary value copies.
  TEST(SemanticArrayTest, EvaluatesCompileTimeArrayBindings)
  {
    expectArrayExecution("comptime var A = [10, 20]; comptime var B = A; comptime { A[0] = 100; B[1] += 12; } func Entry(): i32 { return B[0] + B[1]; }", 42);
    expectArrayExecution("comptime var A = [[1, 2], [3, 4]]; comptime { A[1][0] = 40; } func Entry(): i32 { return A[1][0] + A[0][1]; }", 42);
    expectArrayExecution("comptime var Index = 0; comptime var A = [0, 0]; comptime { A[Index++] = A[Index] = 21; } func Entry(): i32 { return A[0] + A[1]; }", 42);
  }

  // Repetition in the AST evaluator executes its value once and freezes arrays at the semantic boundary.
  TEST(SemanticArrayTest, FreezesCompileTimeRepeatedArrays)
  {
    expectArrayExecution("comptime var Count: i32 = 0; comptime var A = [++Count; 3]; func Entry(): i32 { return A[0] + A[1] + A[2] + Count; }", 4);
    expectArrayExecution("comptime func Make(): [i32; 2] { var A = [0, 0]; A[1] = 42; return A; } func Entry(): i32 { var A = comptime Make(); return A[1]; }", 42);
  }

  // Signed and unsigned indices of every supported width address the same valid element.
  TEST(SemanticArrayTest, AcceptsAllIntegerIndexWidths)
  {
    for (std::string_view Type : {"i8", "i16", "i32", "i64", "i128", "u8", "u16", "u32", "u64", "u128"})
    {
      expectArrayExecution("func Entry(): i32 { var A = [0, 42]; var Index: " + std::string(Type) + " = 1; return A[Index]; }", 42);
    }
  }

  // Constant array bindings remain readable while writes and mutable element borrows are rejected.
  TEST(SemanticArrayTest, EnforcesConstantElementAccess)
  {
    expectArrayExecution("func Entry(): i32 { const A = [1, 42]; return A[1]; }", 42);
    expectArrayDiagnostic("func Entry(): i32 { const A = [1]; A[0] = 2; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
    expectArrayDiagnostic("func Entry(): i32 { const A = [1]; var P = &A[0]; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
    expectArrayDiagnostic("comptime const A = [1]; comptime { A[0] = 2; }", core::DiagnosticKind::SemanticInvalidAssignment);
  }

  // A partial element write cannot initialize an otherwise uninitialized aggregate.
  TEST(SemanticArrayTest, RequiresInitializedArrayBeforeElementAccess)
  {
    expectArrayDiagnostic("func Entry(): i32 { var A: [i32; 2]; return A[0]; }", core::DiagnosticKind::SemanticUninitializedRead);
    expectArrayDiagnostic("func Entry(): i32 { var A: [i32; 2]; A[0] = 1; return 0; }", core::DiagnosticKind::SemanticUninitializedRead);
    expectArrayExecution("func Entry(): i32 { var A: [i32; 2]; A = [0, 42]; return A[1]; }", 42);
  }

  // Empty literals without context and invalid element or length types produce user diagnostics.
  TEST(SemanticArrayTest, RejectsUninferrableAndMismatchedArrayTypes)
  {
    expectArrayDiagnostic("func Entry(): void { var A = []; }", core::DiagnosticKind::SemanticEmptyArrayNeedsType);
    expectArrayDiagnostic("func Entry(): void { var A: [u8; 1] = [256]; }", core::DiagnosticKind::SemanticIntegerOutOfRange);
    expectArrayDiagnostic("func Entry(): void { var A: [i32; 1] = [1, 2]; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectArrayDiagnostic("func Entry(): void { var A = [1, true]; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectArrayDiagnostic("func Entry(): void { var A = [[1], [2, 3]]; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectArrayDiagnostic("func Entry(): void { var A: [void; 0] = []; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectArrayDiagnostic("func Entry(): void { var A: [type; 0] = []; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectArrayDiagnostic("func Value(): i32 { return 0; } func Entry(): void { var A = [Value]; }", core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Array lengths reject negative, noninteger, runtime and unrepresentable values before allocation.
  TEST(SemanticArrayTest, RejectsInvalidArrayLengths)
  {
    expectArrayDiagnostic("func Entry(): void { var A = [0; -1]; }", core::DiagnosticKind::SemanticInvalidArrayLength);
    expectArrayDiagnostic("func Entry(): void { var A = [0; true]; }", core::DiagnosticKind::SemanticInvalidArrayLength);
    expectArrayDiagnostic("func Entry(): void { var A = [0; 1.5]; }", core::DiagnosticKind::SemanticInvalidArrayLength);
    expectArrayDiagnostic("func Entry(): void { var A: [i32; -1]; }", core::DiagnosticKind::SemanticInvalidArrayLength);
    expectArrayDiagnostic("func Entry(): void { var A = [0; 18446744073709551616]; }", core::DiagnosticKind::SemanticInvalidArrayLength);
    expectArrayDiagnostic("func Entry(): void { var A = [0; 18446744073709551615]; }", core::DiagnosticKind::SemanticArrayTooLarge);
    expectArrayDiagnostic("func Entry(Count: i32): void { var A = [0; Count]; }", core::DiagnosticKind::ExecutionRuntimeValue);
  }

  // Array materialization counts runtime element storage and nested lengths before allocating aggregate vectors.
  TEST(SemanticArrayTest, RejectsOversizedMaterializationBeforeAllocation)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_EXECUTION_MAX_STORAGE_BYTES");
    ASSERT_TRUE(Environment.set("4096"));
    expectArrayDiagnostic("func Entry(): void { var A = [0; 1000]; }", core::DiagnosticKind::SemanticArrayTooLarge);
    expectArrayDiagnostic("func Entry(): void { var A = [[0; 16]; 16]; }", core::DiagnosticKind::SemanticArrayTooLarge);
    expectArrayDiagnostic("func Entry(A: [[i32; 16]; 16]): void {}", core::DiagnosticKind::SemanticArrayTooLarge);
  }

  // Indexing requires an array and an integer; floats, booleans, strings and pointer indexing are diagnosed.
  TEST(SemanticArrayTest, RejectsNonarrayAndNonintegerIndexOperands)
  {
    expectArrayDiagnostic("func Entry(): i32 { return 1[0]; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectArrayDiagnostic("func Entry(): i32 { var A = [42]; return A[true]; }", core::DiagnosticKind::SemanticInvalidArrayIndex);
    expectArrayDiagnostic("func Entry(): i32 { var A = [42]; return A[1.0]; }", core::DiagnosticKind::SemanticInvalidArrayIndex);
    expectArrayDiagnostic("func Entry(): i32 { var A = [42]; return A[\"0\"]; }", core::DiagnosticKind::SemanticInvalidArrayIndex);
    expectArrayDiagnostic("func Entry(P: *i32): i32 { return P[0]; }", core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Literal and compile-time out-of-range indices are rejected for reads, writes and address expressions.
  TEST(SemanticArrayTest, RejectsConstantOutOfBoundsIndices)
  {
    expectArrayDiagnostic("func Entry(): i32 { var A = [42]; return A[-1]; }", core::DiagnosticKind::SemanticArrayIndexOutOfBounds);
    expectArrayDiagnostic("func Entry(): i32 { var A = [42]; return A[1]; }", core::DiagnosticKind::SemanticArrayIndexOutOfBounds);
    expectArrayDiagnostic("func Entry(): void { var A = [42]; A[1] = 0; }", core::DiagnosticKind::SemanticArrayIndexOutOfBounds);
    expectArrayDiagnostic("func Entry(): void { var A = [42]; var P = &A[1]; }", core::DiagnosticKind::SemanticArrayIndexOutOfBounds);
    expectArrayDiagnostic("func Entry(): void { var A: [u8; 0] = []; var P = &A[0]; }", core::DiagnosticKind::SemanticArrayIndexOutOfBounds);
    expectArrayDiagnostic("comptime var A = [42]; comptime { A[-1] = 0; }", core::DiagnosticKind::SemanticArrayIndexOutOfBounds);
  }

  // Dynamic negative, upper-bound, empty and wide unsigned indices fail without an unchecked memory access.
  TEST(SemanticArrayTest, ChecksDynamicBoundsForReadsWritesAndAddresses)
  {
    expectArrayExecutionFailure("func Entry(): i32 { var A = [42]; var Index = -1; return A[Index]; }", ExecutionStatus::IndexOutOfBounds);
    expectArrayExecutionFailure("func Entry(): i32 { var A = [42]; var Index = 1; A[Index] = 0; return 0; }", ExecutionStatus::IndexOutOfBounds);
    expectArrayExecutionFailure("func Entry(): i32 { var A = [42]; var Index = 1; var P = &A[Index]; return 0; }", ExecutionStatus::IndexOutOfBounds);
    expectArrayExecutionFailure("func Entry(): i32 { var A = [0; 0]; var Index = 0; return A[Index]; }", ExecutionStatus::IndexOutOfBounds);
    expectArrayExecutionFailure("func Entry(): i32 { var A = [42]; var Index: u128 = 18446744073709551616; return A[Index]; }", ExecutionStatus::IndexOutOfBounds);
    expectArrayExecutionFailure("func Pick(A: [i32; 1], Index: i32): i32 { return A[Index]; } func Entry(): i32 { return Pick([42], 1); }", ExecutionStatus::IndexOutOfBounds);
  }

  // An escaped element address may be copied and discarded without dereferencing its expired storage.
  TEST(SemanticArrayTest, AllowsDiscardingEscapedArrayElementAddress)
  {
    expectArrayExecution("func Escape(): *i32 { var A = [42]; return &A[0]; } func Entry(): i32 { var Address = Escape(); return 42; }", 42);
  }
} // namespace ink::semantic::test
