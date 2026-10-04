#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

#ifdef _WIN32
#define INK_TEST_ADDRESS_EXPORT extern "C" __declspec(dllexport)
#else
#define INK_TEST_ADDRESS_EXPORT extern "C" __attribute__((visibility("default")))
#endif

INK_TEST_ADDRESS_EXPORT void inkTestAddressStore(std::int32_t *Value) noexcept
{
  *Value = 42;
}

INK_TEST_ADDRESS_EXPORT std::int32_t inkTestAddressAlias(std::int32_t *Left, std::int32_t *Right) noexcept
{
  *Left = 40;
  *Right += 2;
  return Left == Right ? 100 : 0;
}

INK_TEST_ADDRESS_EXPORT std::int32_t *inkTestAddressIdentity(std::int32_t *Value) noexcept
{
  return Value;
}

#undef INK_TEST_ADDRESS_EXPORT

namespace ink::semantic::test
{
  using namespace ink::ir;
  using namespace ink::execution;

  namespace
  {
    class AddressAnalysis final
    {
      public:
        explicit AddressAnalysis(std::string_view Source)
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

    void expectIntegerExecution(std::string_view Source, std::uint64_t Expected)
    {
      SCOPED_TRACE(Source);
      AddressAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      Module *Module = Input.analyze();
      ASSERT_NE(Module, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      const Function *Entry = Input.entry(*Module);
      ASSERT_NE(Entry, nullptr);
      ExecutionEngine Engine(Input.Context.irContext());
      const auto Result = Engine.execute(*Entry);
      ASSERT_TRUE(Result) << static_cast<int>(Result.Status);
      ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Integer);
      ASSERT_EQ(Result.Value.integer().bitWidth(), 32U);
      const auto Bits = Result.Value.integer().bits();
      ASSERT_EQ(Bits.words().size(), 1U);
      EXPECT_EQ(Bits.words().front(), Expected);
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
    }

    void expectDiagnostic(std::string_view Source, core::DiagnosticKind Kind)
    {
      SCOPED_TRACE(Source);
      AddressAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      const auto Diagnostics = Input.Diagnostics.diagnostics();
      EXPECT_TRUE(std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
                              {
                                return Diagnostic.Kind == Kind;
                              }));
    }

    void expectEscapedAddress(std::string_view Source)
    {
      SCOPED_TRACE(Source);
      AddressAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      Module *Module = Input.analyze();
      ASSERT_NE(Module, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      const Function *Entry = Input.entry(*Module);
      ASSERT_NE(Entry, nullptr);
      ExecutionEngine Engine(Input.Context.irContext());
      EXPECT_EQ(Engine.execute(*Entry).Status, ExecutionStatus::Success);
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
      EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
    }
  } // namespace

  // Taking an initialized local's address preserves parenthesized lvalues and nested dereference/address pairs.
  TEST(SemanticAddressOfTest, ReadsParenthesizedLocalThroughNestedAddressOperators)
  {
    expectIntegerExecution("func Entry(): i32 { var Value: i32 = 42; return *&(*&((Value))); }", 42);
  }

  // A callee can update caller-owned storage through a pointer parameter, and later direct loads observe the update.
  TEST(SemanticAddressOfTest, WritesCallerLocalThroughPointerParameter)
  {
    expectIntegerExecution("func Add(Pointer: *i32): void { *Pointer = *Pointer + 3; } func Entry(): i32 { var Value: i32 = 36; Add(&Value); Add(&Value); return Value; }", 42);
  }

  // Two independently formed addresses of the same local share one storage cell across reads and writes.
  TEST(SemanticAddressOfTest, PreservesAliasesOfOneLocal)
  {
    expectIntegerExecution("func Entry(): i32 { var Value: i32 = 0; var First = &Value; var Second = &Value; *First = 40; *Second = *First + 2; return Value; }", 42);
  }

  // A constant pointer binding still permits modification of its mutable pointee without rebinding the pointer.
  TEST(SemanticAddressOfTest, AllowsPointeeWriteThroughConstantPointerBinding)
  {
    expectIntegerExecution("func Entry(): i32 { var Value: i32 = 0; const Pointer = &Value; *Pointer = 42; return Value; }", 42);
  }

  // A pointer local is itself addressable, so replacing it through a pointer-to-pointer changes later dereferences.
  TEST(SemanticAddressOfTest, ReassignsPointerLocalThroughItsAddress)
  {
    expectIntegerExecution("func Entry(): i32 { var Left: i32 = 10; var Right: i32 = 0; var Pointer = &Left; var Slot = &Pointer; *Slot = &Right; **Slot = 32; return Left + Right; }", 42);
  }

  // Addressing a dereference evaluates a pointer-producing call exactly once and preserves its returned address.
  TEST(SemanticAddressOfTest, EvaluatesAddressOfDereferenceOperandOnce)
  {
    expectIntegerExecution("func Pick(Value: *i32, Count: *i32): *i32 { *Count = *Count + 1; return Value; } func Entry(): i32 { var Value: i32 = 0; var Count: i32 = 0; var Pointer = &*Pick(&Value, &Count); *Pointer = 41; return Value + Count; }", 42);
  }

  // Indirect assignment evaluates its destination once before evaluating its side-effecting right operand.
  TEST(SemanticAddressOfTest, EvaluatesIndirectAssignmentDestinationOnce)
  {
    expectIntegerExecution("func Pick(Value: *i32, Count: *i32): *i32 { *Count = *Count + 1; return Value; } func Next(Count: *i32): i32 { *Count = *Count + 10; return 31; } func Entry(): i32 { var Value: i32 = 0; var Count: i32 = 0; *Pick(&Value, &Count) = Next(&Count); return Value + Count; }", 42);
  }

  // A local initialized by a preceding assignment becomes a valid address operand.
  TEST(SemanticAddressOfTest, AllowsAddressAfterDefiniteInitialization)
  {
    expectIntegerExecution("func Entry(): i32 { var Value: i32; Value = 40; var Pointer = &Value; *Pointer = *Pointer + 2; return Value; }", 42);
  }

  // Pointer operations inside compile-time functions execute in IR and freeze only their final scalar result.
  TEST(SemanticAddressOfTest, ExecutesAddressOperationsInsideCompileTimeFunction)
  {
    expectIntegerExecution("comptime func Calculate(): i32 { var Value: i32 = 40; var Pointer = &Value; *Pointer = *Pointer + 2; return Value; } func Entry(): i32 { return comptime Calculate(); }", 42);
  }

  // Returning a callee-local raw address does not access or keep its storage alive.
  TEST(SemanticAddressOfTest, AllowsReturningAnUnownedLocalAddress)
  {
    expectEscapedAddress("func Escape(): *i32 { var Value: i32 = 42; return &Value; } func Entry(): *i32 { return Escape(); }");
  }

  // An address requires storage; literals, arithmetic results, call results, and function declarations are not lvalues.
  TEST(SemanticAddressOfTest, RejectsAddressesOfTemporaryAndFunctionValues)
  {
    const std::string_view Sources[] = {
        "func Entry(): i32 { var Pointer = &42; return 0; }",
        "func Entry(): i32 { var Value: i32 = 1; var Pointer = &(Value + 1); return 0; }",
        "func Value(): i32 { return 1; } func Entry(): i32 { var Pointer = &Value(); return 0; }",
        "func Value(): i32 { return 1; } func Entry(): i32 { var Pointer = &Value; return 0; }",
    };
    for (std::string_view Source : Sources)
    {
      expectDiagnostic(Source, core::DiagnosticKind::SemanticInvalidAddressOperand);
    }
  }

  // The read/write pointer syntax cannot expose a constant binding through an address that permits mutation.
  TEST(SemanticAddressOfTest, RejectsMutableAddressOfConstantBinding)
  {
    expectDiagnostic("func Entry(): i32 { const Value: i32 = 1; var Pointer = &Value; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
    expectDiagnostic("func Entry(): i32 { var Value: i32 = 1; const Pointer = &Value; var Slot = &Pointer; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
  }

  // Value parameters have no addressable local slot, so their direct address is diagnosed explicitly.
  TEST(SemanticAddressOfTest, RejectsDirectAddressOfValueParameter)
  {
    expectDiagnostic("func Entry(Value: i32): i32 { var Pointer = &Value; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
  }

  // Native output parameters are not definite-initialization contracts, so taking an uninitialized local's address is rejected.
  TEST(SemanticAddressOfTest, RejectsAddressBeforeDefiniteInitialization)
  {
    expectDiagnostic("func Entry(): i32 { var Value: i32; var Pointer = &Value; return 0; }", core::DiagnosticKind::SemanticUninitializedRead);
    expectDiagnostic("func Entry(Condition: bool): i32 { var Value: i32; if (Condition) { Value = 1; } var Pointer = &Value; return 0; }", core::DiagnosticKind::SemanticUninitializedRead);
  }

  // A pointer variable must itself be initialized before either a dereference read or an indirect assignment.
  TEST(SemanticAddressOfTest, RejectsUninitializedPointerUse)
  {
    expectDiagnostic("func Entry(): i32 { var Pointer: *i32; return *Pointer; }", core::DiagnosticKind::SemanticUninitializedRead);
    expectDiagnostic("func Entry(): i32 { var Pointer: *i32; *Pointer = 1; return 0; }", core::DiagnosticKind::SemanticUninitializedRead);
  }

  // Scalar operands and void pointers cannot provide the typed storage needed by a dereference.
  TEST(SemanticAddressOfTest, RejectsDereferenceWithoutTypedPointer)
  {
    expectDiagnostic("func Entry(): i32 { return *42; }", core::DiagnosticKind::SemanticInvalidDereference);
    expectDiagnostic("func Entry(): i32 { var Value: i32 = 1; *Value = 2; return 0; }", core::DiagnosticKind::SemanticInvalidDereference);
    expectDiagnostic("func Entry(Pointer: *void): void { *Pointer; }", core::DiagnosticKind::SemanticInvalidDereference);
  }

  // The pointee type controls both indirect stores and pointer initialization without implicit scalar reinterpretation.
  TEST(SemanticAddressOfTest, RejectsMismatchedPointeeValues)
  {
    expectDiagnostic("func Entry(): i32 { var Value: i32 = 0; var Pointer = &Value; *Pointer = true; return 0; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectDiagnostic("func Entry(): i32 { var Value: i32 = 0; var Pointer: *u32 = &Value; return 0; }", core::DiagnosticKind::SemanticTypeMismatch);
  }

  // Compile-time binding descriptors and transient compile-time addresses never escape into runtime IR.
  TEST(SemanticAddressOfTest, RejectsAddressAcrossCompileTimeBoundary)
  {
    expectDiagnostic("comptime var Value: i32 = 0; func Entry(): i32 { var Pointer = &Value; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
    expectDiagnostic("func Entry(): i32 { var Value: i32 = 0; var Pointer = comptime &Value; return 0; }", core::DiagnosticKind::SemanticInvalidAddressOperand);
  }

  // A nested function cannot borrow storage belonging to another runtime function activation.
  TEST(SemanticAddressOfTest, RejectsAddressOfCapturedRuntimeLocal)
  {
    expectDiagnostic("func Entry(): i32 { var Value: i32 = 0; func Inner(): *i32 { return &Value; } return 0; }", core::DiagnosticKind::SemanticInvalidCapture);
  }

#if defined(_WIN32) || defined(__linux__)
  // Native code writes the local's real memory and later Ink loads observe the new value without changing earlier snapshots.
  TEST(SemanticAddressOfTest, ObservesNativeWritesWithoutChangingEarlierLoads)
  {
    expectIntegerExecution("import \"C\" func inkTestAddressStore(Value: *i32): void; func Entry(): i32 { var Value: i32 = 5; var Previous = Value; inkTestAddressStore(&Value); return Value + Previous; }", 47);
  }

  // Repeated borrows of one local reach native code as exactly the same address and share its mutations.
  TEST(SemanticAddressOfTest, PreservesAliasedNativeArguments)
  {
    expectIntegerExecution("import \"C\" func inkTestAddressAlias(Left: *i32, Right: *i32): i32; func Entry(): i32 { var Value: i32 = 0; var Alias = &Value; var Same = inkTestAddressAlias(&Value, Alias); return Same + Value; }", 142);
  }

  // Returning an argument address from C restores its managed provenance and permits typed Ink writes.
  TEST(SemanticAddressOfTest, RecoversManagedAddressReturnedFromNativeCode)
  {
    expectIntegerExecution("import \"C\" func inkTestAddressIdentity(Value: *i32): *i32; func Entry(): i32 { var Value: i32 = 0; var Returned = inkTestAddressIdentity(&Value); *Returned = 42; return Value; }", 42);
  }

  // A native-returned local address still expires when its owner function returns.
  TEST(SemanticAddressOfTest, AllowsReturningNativeIdentityAddress)
  {
    expectEscapedAddress("import \"C\" func inkTestAddressIdentity(Value: *i32): *i32; func Escape(): *i32 { var Value: i32 = 0; return inkTestAddressIdentity(&Value); } func Entry(): *i32 { return Escape(); }");
  }
#endif
} // namespace ink::semantic::test
