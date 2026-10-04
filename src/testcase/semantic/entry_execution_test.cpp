#include "../execution/ffi/external_call_test_support.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/memory/execution_heap.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::semantic::test
{
  using namespace ink::ir;
  using namespace ink::execution;

  namespace
  {
    class EntryAnalysis final
    {
      public:
        explicit EntryAnalysis(std::string_view Source)
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

        const Function *function(Module &Owner, std::string_view Name)
        {
          const auto *Binding = NameResolver(Context).lookupMember(Owner, Context.namePool().find(Name));
          if (!Binding || Binding->targets().size() != 1 || !Function::classof(Binding->targets().front()))
          {
            return nullptr;
          }
          return static_cast<const Function *>(Binding->targets().front());
        }

        ExecutionValueRef integer(ExecutionHeap &Heap, std::uint32_t Width, bool Signed, std::uint64_t Bits)
        {
          const auto *Type = Context.typePool().getType<TypeKind::Integer>(Width, Signed);
          return Heap.integer(*Type, ExecutionInteger(Width, Bits));
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    void expectInteger(const ExecutionValueResult &Result, std::uint32_t Width, std::uint64_t Expected)
    {
      ASSERT_TRUE(Result);
      ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Integer);
      const auto Bits = Result.Value.integer().bits();
      EXPECT_EQ(Bits.bitWidth(), Width);
      ASSERT_EQ(Bits.words().size(), 1U);
      EXPECT_EQ(Bits.words().front(), Expected);
    }
  } // namespace

  // Ordinary source functions execute nested calls and mutable locals using different runtime arguments on every invocation.
  TEST(SemanticEntryExecutionTest, ExecutesNestedCallsAndIsolatesLocalStorage)
  {
    const std::string Source = "func Bump(Input: i32): i32 { var Local = Input; Local = Local + 3; return Local; } func Entry(Input: i32): i32 { var First = Bump(Input); var Second = Bump(First); return First + Second; }";
    EntryAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    ExecutionValueRef Arguments[] = {Input.integer(Engine.heap(), 32, true, 5)};
    const auto ConstantCount = Input.Context.constantPool().size();
    const auto First = Engine.execute(*Entry, Arguments);
    expectInteger(First, 32, 19);
    Arguments[0] = Input.integer(Engine.heap(), 32, true, 20);
    expectInteger(Engine.execute(*Entry, Arguments), 32, 49);
    expectInteger(First, 32, 19);
    EXPECT_EQ(Input.Context.constantPool().size(), ConstantCount);
  }

  // A local function declaration becomes an executable function value before the enclosing entry calls it.
  TEST(SemanticEntryExecutionTest, ExecutesFunctionDeclaredInsideEntry)
  {
    EntryAnalysis Input("func Entry(): i32 { func Inner(): i32 { return 7; } return Inner(); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    expectInteger(Engine.execute(*Entry), 32, 7);
  }

  // Boolean and floating runtime parameters survive ordinary local storage and return without being integer constants.
  TEST(SemanticEntryExecutionTest, PreservesBooleanAndFloatingRuntimePayloads)
  {
    EntryAnalysis Input("func EchoBool(Input: bool): bool { var Copy = Input; return Copy; } func EchoFloat(Input: f32): f32 { var Copy = Input; return Copy; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Bool = Input.function(*Module, "EchoBool");
    const Function *Float = Input.function(*Module, "EchoFloat");
    ASSERT_NE(Bool, nullptr);
    ASSERT_NE(Float, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const ExecutionValueRef BoolArguments[] = {Engine.heap().boolean(Input.Context.typePool().getType<TypeKind::Bool>(), true)};
    const auto BoolResult = Engine.execute(*Bool, BoolArguments);
    ASSERT_TRUE(BoolResult);
    ASSERT_EQ(BoolResult.Value.kind(), ExecutionValueKind::Boolean);
    EXPECT_TRUE(BoolResult.Value.boolean());
    const auto *FloatType = Input.Context.typePool().getType<TypeKind::Float>(32);
    const FloatBits Bits(32, std::bit_cast<std::uint32_t>(1.5F));
    const ExecutionValueRef FloatArguments[] = {Engine.heap().floating(*FloatType, Bits)};
    const auto FloatResult = Engine.execute(*Float, FloatArguments);
    ASSERT_TRUE(FloatResult);
    ASSERT_EQ(FloatResult.Value.kind(), ExecutionValueKind::Float);
    EXPECT_EQ(FloatResult.Value.floating(), Bits);
  }

  // An ordinary void entry reaches its generated return after executing its local declaration.
  TEST(SemanticEntryExecutionTest, ExecutesVoidEntryWithoutAResultPayload)
  {
    EntryAnalysis Input("func Entry(): void { var Local: i32 = 3; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const auto Result = Engine.execute(*Entry);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.kind(), ExecutionValueKind::Void);
  }

#if defined(_WIN32) || defined(__linux__)
  // Parsed ordinary functions pass runtime fd/count locals to native writes, then use both call results in integer addition.
  TEST(SemanticEntryExecutionTest, WritesRealBytesThroughRuntimeParametersAndNestedCalls)
  {
    execution::test::NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    const std::string CountType = "u" + std::to_string(execution::test::WriteCountWidth);
    const std::string ReturnType = execution::test::writeReturnType();
    std::string Source = execution::test::writeDeclaration();
    Source += "func Emit(Fd: i32, Count: " + CountType + "): " + ReturnType + " { var Destination = Fd; var Length = Count; var Written = " + std::string(execution::test::WriteSymbol) + "(Destination, \"中文\", Length); return Written + 1; }\n";
    Source += "func Entry(Fd: i32, Count: " + CountType + "): " + ReturnType + " { var First = Emit(Fd, Count); var Second = Emit(Fd, Count); return First + Second; }";
    EntryAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const ExecutionValueRef Arguments[] = {Input.integer(Engine.heap(), 32, true, Pipe.writer()), Input.integer(Engine.heap(), execution::test::WriteCountWidth, false, 6)};
    expectInteger(Engine.execute(*Entry, Arguments), execution::test::WriteReturnWidth, 14);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "\xe4\xb8\xad\xe6\x96\x87\xe4\xb8\xad\xe6\x96\x87");
  }

  // Reusing one call result does not replay its write, while executing the ordinary entry again performs a fresh write.
  TEST(SemanticEntryExecutionTest, ExecutesEachRuntimeCallExactlyOncePerInvocation)
  {
    execution::test::NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    const std::string CountType = "u" + std::to_string(execution::test::WriteCountWidth);
    const std::string Source = execution::test::writeDeclaration() + "func Entry(Fd: i32, Count: " + CountType + "): " + execution::test::writeReturnType() + " { var Written = " + std::string(execution::test::WriteSymbol) + "(Fd, \"X\", Count); return Written + Written; }";
    EntryAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const ExecutionValueRef Arguments[] = {Input.integer(Engine.heap(), 32, true, Pipe.writer()), Input.integer(Engine.heap(), execution::test::WriteCountWidth, false, 1)};
    expectInteger(Engine.execute(*Entry, Arguments), execution::test::WriteReturnWidth, 2);
    expectInteger(Engine.execute(*Entry, Arguments), execution::test::WriteReturnWidth, 2);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "XX");
  }

  // A native pointer returned into a live CString buffer flows through an ordinary local into the next external call.
  TEST(SemanticEntryExecutionTest, PassesNativePointerReturnsThroughRuntimeLocalStorage)
  {
    const std::uint32_t SizeWidth = sizeof(std::size_t) * 8;
    const std::string Source = "import \"C\" func strchr(Text: *u8, Needle: i32): *u8; import \"C\" func strlen(Text: *u8): u" + std::to_string(SizeWidth) + "; func Entry(): u" + std::to_string(SizeWidth) + " { var Tail = strchr(\"prefix:tail\", 58); return strlen(Tail); }";
    EntryAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    expectInteger(Engine.execute(*Entry), SizeWidth, 5);
  }

  // A callee-local CString address may escape without being dereferenced or kept alive.
  TEST(SemanticEntryExecutionTest, AllowsDiscardingEscapedCStringPointer)
  {
    const std::uint32_t SizeWidth = sizeof(std::size_t) * 8;
    const std::string Source = "import \"C\" func strchr(Text: *u8, Needle: i32): *u8; import \"C\" func strlen(Text: *u8): u" + std::to_string(SizeWidth) + "; func Find(): *u8 { return strchr(\"prefix:tail\", 58); } func Entry(): u" + std::to_string(SizeWidth) + " { var Tail = Find(); return 0; }";
    EntryAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Entry = Input.function(*Module, "Entry");
    ASSERT_NE(Entry, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    EXPECT_EQ(Engine.execute(*Entry).Status, ExecutionStatus::Success);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
  }
#endif
} // namespace ink::semantic::test
