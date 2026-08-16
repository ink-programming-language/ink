#include "ink/execution/execution_engine.h"
#include "ink/ir/context.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace ink::execution
{
  namespace
  {
    struct TestContext
    {
        core::CompilationContext Compilation;
        ir::IRContext IR{Compilation};
        ExecutionContext Execution{Compilation};
    };

    std::string CapturedOutput;
    std::int32_t RecordedValue = 0;

    struct NativePair
    {
        std::uint8_t First;
        std::int32_t Second;
    };

    extern "C" std::int32_t captureOutput(const std::uint8_t *Data, std::size_t Size)
    {
      if (Size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
      {
        return -1;
      }
      CapturedOutput.append(reinterpret_cast<const char *>(Data), Size);
      return static_cast<std::int32_t>(Size);
    }

    extern "C" NativePair incrementPair(NativePair Value)
    {
      ++Value.First;
      Value.Second += 2;
      return Value;
    }

    extern "C" std::size_t combinePrimitiveValues(std::uint8_t BoolValue, std::uint8_t ByteValue, std::int32_t I32Value, std::size_t PointerSizeValue, const std::uint8_t *PointerValue)
    {
      return static_cast<std::size_t>(BoolValue) + static_cast<std::size_t>(ByteValue) + static_cast<std::size_t>(I32Value) + PointerSizeValue + (PointerValue == nullptr ? 0 : *PointerValue);
    }

    extern "C" const std::uint8_t *identityPointer(const std::uint8_t *Value)
    {
      return Value;
    }

    extern "C" void recordValue(std::int32_t Value)
    {
      RecordedValue = Value;
    }

    ir::Module makeHelloWorldModule(ir::IRContext &Context, std::string ExternalName = "print")
    {
      const ir::Type &VoidType = Context.getType(ir::TypeKind::Void);
      const ir::Type &I32Type = Context.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = Context.getType(ir::TypeKind::PointerSize);
      const ir::Type &ConstBytePointerType = Context.getType(ir::TypeKind::ConstBytePointer);
      ir::Module Result(Context);
      Result.ByteConstants.push_back({"str.0", "Hello, world!\n"});

      ir::Function WriteStdout(I32Type);
      WriteStdout.Name = std::move(ExternalName);
      WriteStdout.Kind = ir::FunctionKind::External;
      WriteStdout.Convention = ir::CallingConvention::C;
      WriteStdout.ParameterTypes = {&ConstBytePointerType, &PointerSizeType};
      WriteStdout.HasSideEffects = true;
      Result.Functions.push_back(std::move(WriteStdout));

      auto Call = std::make_unique<ir::CallInstruction>(I32Type);
      Call->Result = ir::ValueId{0};
      Call->Callee = ir::FunctionId{0};
      Call->Arguments.push_back(std::make_unique<ir::GlobalAddressOperand>(ConstBytePointerType, ir::GlobalId{0}, 0));
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(PointerSizeType, 14));

      ir::Function Main(VoidType);
      Main.Name = "main";
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(Call));
      Entry.Instructions.push_back(std::make_unique<ir::ReturnInstruction>());
      Main.Blocks.push_back(std::move(Entry));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    ir::Module makeInternalCallModule(ir::IRContext &Context)
    {
      const ir::Type &I32Type = Context.getType(ir::TypeKind::I32);
      ir::Module Result(Context);

      ir::Function Identity(I32Type);
      Identity.Name = "identity";
      Identity.ParameterTypes = {&I32Type};
      ir::BasicBlock IdentityEntry;
      IdentityEntry.Name = "entry";
      auto IdentityReturn = std::make_unique<ir::ReturnInstruction>();
      IdentityReturn->ReturnValue = std::make_unique<ir::ValueOperand>(I32Type, ir::ValueId{0});
      IdentityEntry.Instructions.push_back(std::move(IdentityReturn));
      Identity.Blocks.push_back(std::move(IdentityEntry));
      Result.Functions.push_back(std::move(Identity));

      auto Call = std::make_unique<ir::CallInstruction>(I32Type);
      Call->Result = ir::ValueId{0};
      Call->Callee = ir::FunctionId{0};
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(I32Type, 42));
      auto MainReturn = std::make_unique<ir::ReturnInstruction>();
      MainReturn->ReturnValue = std::make_unique<ir::ValueOperand>(I32Type, ir::ValueId{0});

      ir::Function Main(I32Type);
      Main.Name = "main";
      ir::BasicBlock MainEntry;
      MainEntry.Name = "entry";
      MainEntry.Instructions.push_back(std::move(Call));
      MainEntry.Instructions.push_back(std::move(MainReturn));
      Main.Blocks.push_back(std::move(MainEntry));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    ir::Module makeRecursiveModule(ir::IRContext &Context)
    {
      const ir::Type &VoidType = Context.getType(ir::TypeKind::Void);
      ir::Module Result(Context);

      auto Call = std::make_unique<ir::CallInstruction>(VoidType);
      Call->Callee = ir::FunctionId{0};
      ir::Function Recurse(VoidType);
      Recurse.Name = "recurse";
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(Call));
      Entry.Instructions.push_back(std::make_unique<ir::ReturnInstruction>());
      Recurse.Blocks.push_back(std::move(Entry));
      Result.Functions.push_back(std::move(Recurse));
      return Result;
    }

    ir::Module makeMultipleBlockModule(ir::IRContext &Context)
    {
      const ir::Type &VoidType = Context.getType(ir::TypeKind::Void);
      ir::Module Result(Context);
      ir::Function Main(VoidType);
      Main.Name = "main";
      ir::BasicBlock First;
      First.Name = "first";
      First.Instructions.push_back(std::make_unique<ir::ReturnInstruction>());
      Main.Blocks.push_back(std::move(First));
      ir::BasicBlock Second;
      Second.Name = "second";
      Second.Instructions.push_back(std::make_unique<ir::ReturnInstruction>());
      Main.Blocks.push_back(std::move(Second));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    std::string executionMessage(const std::vector<core::Diagnostic> &Diagnostics)
    {
      return Diagnostics.empty() ? std::string() : core::DiagnosticFormatter().format(Diagnostics.front()).Message;
    }

    void expectStringArgument(const core::Diagnostic &DiagnosticEntry, std::size_t Index, core::DiagnosticArgumentName Name, std::string_view ExpectedValue)
    {
      ASSERT_LT(Index, DiagnosticEntry.Arguments.size());
      EXPECT_EQ(DiagnosticEntry.Arguments[Index].Name, Name);
      const std::string *ActualValue = std::get_if<std::string>(&DiagnosticEntry.Arguments[Index].Value);
      ASSERT_NE(ActualValue, nullptr);
      EXPECT_EQ(*ActualValue, ExpectedValue);
    }

    void expectUnsignedArgument(const core::Diagnostic &DiagnosticEntry, std::size_t Index, core::DiagnosticArgumentName Name, std::uint64_t ExpectedValue)
    {
      ASSERT_LT(Index, DiagnosticEntry.Arguments.size());
      EXPECT_EQ(DiagnosticEntry.Arguments[Index].Name, Name);
      const std::uint64_t *ActualValue = std::get_if<std::uint64_t>(&DiagnosticEntry.Arguments[Index].Value);
      ASSERT_NE(ActualValue, nullptr);
      EXPECT_EQ(*ActualValue, ExpectedValue);
    }

    // Verifies that primitive and aggregate RuntimeValue instances expose only the payload appropriate for their canonical IR type.
    TEST(RuntimeValueTest, PreservesTypedPrimitiveAndAggregatePayloads)
    {
      TestContext Context;
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &I32Type});
      const RuntimeValue Integer = RuntimeValue::integerValue(I32Type, 42);
      const std::uint8_t Byte = 7;
      const RuntimeValue Pointer = RuntimeValue::pointerValue(PointerType, &Byte);
      const RuntimeValue Pair = RuntimeValue::aggregateValue(PairType, {RuntimeValue::integerValue(I32Type, 1), RuntimeValue::integerValue(I32Type, 2)});

      EXPECT_EQ(&Integer.type(), &I32Type);
      EXPECT_EQ(Integer.integer(), 42u);
      EXPECT_EQ(Integer.pointer(), nullptr);
      EXPECT_EQ(Pointer.pointer(), &Byte);
      EXPECT_FALSE(Pointer.integer().has_value());
      ASSERT_EQ(Pair.fieldCount(), 2u);
      ASSERT_NE(Pair.field(0), nullptr);
      EXPECT_EQ(Pair.field(0)->integer(), 1u);
      EXPECT_EQ(Pair.field(1)->integer(), 2u);
      EXPECT_EQ(Pair.field(2), nullptr);
    }

    // Verifies that ExecutionContext composes around the shared compilation diagnostics service.
    TEST(ExecutionContextTest, SharesCompilationDiagnosticEngine)
    {
      TestContext Context;

      EXPECT_EQ(&Context.Execution.compilationContext(), &Context.Compilation);
      EXPECT_EQ(&Context.Execution.diagnosticEngine(), &Context.Compilation.diagnosticEngine());
    }

    // Verifies that native symbols retain both lookup directions and that platform lookup registers a system write function once.
    TEST(NativeSymbolRegistryTest, RegistersNamesAddressesAndPlatformSymbols)
    {
      NativeSymbolRegistry Registry;
      const NativeFunctionAddress CaptureAddress = reinterpret_cast<NativeFunctionAddress>(&captureOutput);
      const NativeFunctionAddress PairAddress = reinterpret_cast<NativeFunctionAddress>(&incrementPair);

      EXPECT_FALSE(Registry.registerSymbol("", CaptureAddress));
      EXPECT_FALSE(Registry.registerSymbol("null", nullptr));
      EXPECT_TRUE(Registry.registerSymbol("capture", CaptureAddress));
      EXPECT_TRUE(Registry.registerSymbol("capture", CaptureAddress));
      EXPECT_FALSE(Registry.registerSymbol("capture", PairAddress));
      EXPECT_TRUE(Registry.registerSymbol("capture_alias", CaptureAddress));
      EXPECT_EQ(Registry.findAddress("capture"), CaptureAddress);
      EXPECT_EQ(Registry.findAddress("missing"), nullptr);
      ASSERT_TRUE(Registry.findName(CaptureAddress).has_value());
      EXPECT_EQ(*Registry.findName(CaptureAddress), "capture");
      EXPECT_FALSE(Registry.findName(PairAddress).has_value());
#if defined(_WIN32)
      EXPECT_TRUE(Registry.resolveAndRegister("WriteFile"));
      EXPECT_NE(Registry.findAddress("WriteFile"), nullptr);
#elif defined(__linux__)
      EXPECT_TRUE(Registry.resolveAndRegister("write"));
      EXPECT_NE(Registry.findAddress("write"), nullptr);
#endif
    }

    // Verifies that repeated initialization is idempotent and registers the built-in print runtime symbol exactly once.
    TEST(ExecutionEngineTest, InitializesIdempotently)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult First = Engine.initialize();
      const std::size_t FirstSymbolCount = Engine.nativeSymbols().symbols().size();
      const InitializationResult Second = Engine.initialize();

      EXPECT_TRUE(First.succeeded());
      EXPECT_TRUE(Second.succeeded());
      EXPECT_NE(Engine.nativeSymbols().findAddress("print"), nullptr);
      EXPECT_EQ(Engine.nativeSymbols().symbols().size(), FirstSymbolCount);
    }

    // Verifies that Hello World crosses the extern boundary through libffi while preserving the byte constant and ptrsize arguments.
    TEST(ExecutionEngineTest, ExecutesExternHelloWorldThroughLibffi)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR, "print");
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      CapturedOutput.clear();
      const NativeFunctionAddress CaptureAddress = reinterpret_cast<NativeFunctionAddress>(&captureOutput);
      ASSERT_TRUE(Engine.nativeSymbols().registerSymbol("print", CaptureAddress));

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Initialization.succeeded());
      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::Void));
      EXPECT_EQ(CapturedOutput, "Hello, world!\n");
      EXPECT_EQ(Engine.nativeSymbols().findAddress("print"), CaptureAddress);
    }

    // Verifies that bool, byte, i32, ptrsize, and pointer arguments plus a ptrsize result are marshalled through libffi without representation leakage.
    TEST(ExecutionEngineTest, MarshalsEveryPrimitiveExternType)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "@value = private constant [1 x byte] c\"\\05\"\n"
          "declare extern \"C\" ptrsize @combine_primitive_values(bool, byte, i32, ptrsize, const byte*)\n"
          "define ptrsize @main() {\n"
          "entry:\n"
          "  %0 = call ptrsize @combine_primitive_values(bool 1, byte 2, i32 3, ptrsize 4, const byte* @value[0])\n"
          "  ret ptrsize %0\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());
      ASSERT_TRUE(Engine.nativeSymbols().registerSymbol("combine_primitive_values", reinterpret_cast<NativeFunctionAddress>(&combinePrimitiveValues)));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::PointerSize));
      EXPECT_EQ(Result.returnValue()->integer(), 15u);
    }

    // Verifies that a null-free const byte pointer can be returned from an extern and remains the same address in RuntimeValue.
    TEST(ExecutionEngineTest, MarshalsPointerExternResult)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "@value = private constant [1 x byte] c\"x\"\n"
          "declare extern \"C\" const byte* @identity_pointer(const byte*)\n"
          "define const byte* @main() {\n"
          "entry:\n"
          "  %0 = call const byte* @identity_pointer(const byte* @value[0])\n"
          "  ret const byte* %0\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      const void *ExpectedAddress = Deserialized.module()->ByteConstants[0].Data.data();
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());
      ASSERT_TRUE(Engine.nativeSymbols().registerSymbol("identity_pointer", reinterpret_cast<NativeFunctionAddress>(&identityPointer)));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->pointer(), ExpectedAddress);
      EXPECT_FALSE(Result.returnValue()->integer().has_value());
    }

    // Verifies that a void extern receives its argument, produces no SSA result, and still yields a typed void function result.
    TEST(ExecutionEngineTest, ExecutesVoidExternCall)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "declare extern \"C\" void @record_value(i32) [sideeffect]\n"
          "define void @main() {\n"
          "entry:\n"
          "  call void @record_value(i32 73)\n"
          "  ret void\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());
      ASSERT_TRUE(Engine.nativeSymbols().registerSymbol("record_value", reinterpret_cast<NativeFunctionAddress>(&recordValue)));
      RecordedValue = 0;

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->type().kind(), ir::TypeKind::Void);
      EXPECT_EQ(RecordedValue, 73);
    }

    // Verifies that the default runtime extern writes Hello World through the platform stdout interface and returns the written byte count.
    TEST(ExecutionEngineTest, ExecutesHelloWorldThroughDefaultPlatformRuntime)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR);
      ir::Function &Main = ModuleValue.Functions[1];
      Main.ResultType = &Context.IR.getType(ir::TypeKind::I32);
      auto &Return = static_cast<ir::ReturnInstruction &>(*Main.Blocks[0].Instructions[1]);
      Return.ReturnValue = std::make_unique<ir::ValueOperand>(Context.IR.getType(ir::TypeKind::I32), ir::ValueId{0});
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::I32));
      EXPECT_EQ(Result.returnValue()->integer(), 14u);
      EXPECT_NE(Engine.nativeSymbols().findAddress("print"), nullptr);
    }

    // Verifies that an internal call creates a callee frame, maps parameter %0, and returns the caller's SSA result.
    TEST(ExecutionEngineTest, ExecutesInternalCallAndSsaReturn)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::I32));
      EXPECT_EQ(Result.returnValue()->integer(), 42u);
    }

    // Verifies that a definition can be used directly as the entry and that its RuntimeValue argument is mapped to parameter SSA value %0.
    TEST(ExecutionEngineTest, ExecutesEntryWithRuntimeArgument)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);

      const ExecutionResult Result = Engine.execute("identity", {RuntimeValue::integerValue(I32Type, 91)});

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->integer(), 91u);
    }

    // Verifies that entry invocation rejects both an incorrect argument count and a RuntimeValue with the wrong canonical Type object.
    TEST(ExecutionEngineTest, RejectsInvalidEntryArguments)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult WrongCount = Engine.execute("identity");
      const ExecutionResult WrongType = Engine.execute("identity", {RuntimeValue::integerValue(Context.IR.getType(ir::TypeKind::Byte), 1)});

      ASSERT_FALSE(WrongCount.succeeded());
      ASSERT_EQ(WrongCount.diagnostics().size(), 1u);
      const core::Diagnostic &WrongCountDiagnostic = WrongCount.diagnostics()[0];
      EXPECT_EQ(WrongCountDiagnostic.Kind, core::DiagnosticKind::EntryArgumentCountMismatch);
      EXPECT_EQ(WrongCountDiagnostic.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(WrongCountDiagnostic.Arguments.size(), 3u);
      expectStringArgument(WrongCountDiagnostic, 0, core::DiagnosticArgumentName::FunctionName, "identity");
      expectUnsignedArgument(WrongCountDiagnostic, 1, core::DiagnosticArgumentName::ExpectedCount, 1);
      expectUnsignedArgument(WrongCountDiagnostic, 2, core::DiagnosticArgumentName::ActualCount, 0);
      EXPECT_EQ(executionMessage(WrongCount.diagnostics()), "entry function @identity received 0 arguments; expected 1");

      ASSERT_FALSE(WrongType.succeeded());
      ASSERT_EQ(WrongType.diagnostics().size(), 1u);
      const core::Diagnostic &WrongTypeDiagnostic = WrongType.diagnostics()[0];
      EXPECT_EQ(WrongTypeDiagnostic.Kind, core::DiagnosticKind::EntryArgumentInvalid);
      EXPECT_EQ(WrongTypeDiagnostic.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(WrongTypeDiagnostic.Arguments.size(), 2u);
      expectStringArgument(WrongTypeDiagnostic, 0, core::DiagnosticArgumentName::FunctionName, "identity");
      expectUnsignedArgument(WrongTypeDiagnostic, 1, core::DiagnosticArgumentName::ArgumentIndex, 0);
      EXPECT_EQ(executionMessage(WrongType.diagnostics()), "argument 0 of entry function @identity has the wrong type or runtime shape");
    }

    // Verifies that zeroinitializer recursively constructs nested struct fields that can be extracted one aggregate level at a time.
    TEST(ExecutionEngineTest, ExecutesNestedStructZeroInitializer)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {i32}\n"
          "%Outer = type {byte, %Inner}\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = extractvalue %Outer zeroinitializer, 1\n"
          "  %1 = extractvalue %Inner %0, 0\n"
          "  ret i32 %1\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->integer(), 0u);
    }

    // Verifies that a struct RuntimeValue can enter and leave an InkIR definition by value without crossing the native ABI boundary.
    TEST(ExecutionEngineTest, PassesStructRuntimeValueThroughInternalFunction)
    {
      TestContext Context;
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &I32Type});
      ir::Module ModuleValue(Context.IR);
      ModuleValue.StructTypes.push_back(&PairType);
      ir::Function Echo(PairType);
      Echo.Name = "echo";
      Echo.ParameterTypes = {&PairType};
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      auto Return = std::make_unique<ir::ReturnInstruction>();
      Return->ReturnValue = std::make_unique<ir::ValueOperand>(PairType, ir::ValueId{0});
      Entry.Instructions.push_back(std::move(Return));
      Echo.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Echo));
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      const RuntimeValue Argument = RuntimeValue::aggregateValue(PairType, {RuntimeValue::integerValue(I32Type, 7), RuntimeValue::integerValue(I32Type, 8)});

      const ExecutionResult Result = Engine.execute("echo", {Argument});

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      ASSERT_EQ(Result.returnValue()->fieldCount(), 2u);
      EXPECT_EQ(Result.returnValue()->field(0)->integer(), 7u);
      EXPECT_EQ(Result.returnValue()->field(1)->integer(), 8u);
    }

    // Verifies that an entry argument with the correct StructType but missing fields is rejected before a malformed aggregate enters an execution frame.
    TEST(ExecutionEngineTest, RejectsMalformedStructRuntimeArgument)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {i32, i32}\n"
          "define %Pair @echo(%Pair %0) {\n"
          "entry:\n"
          "  ret %Pair %0\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      const ir::StructType &PairType = *Deserialized.module()->StructTypes[0];
      const RuntimeValue Malformed = RuntimeValue::aggregateValue(PairType, {RuntimeValue::integerValue(Context.IR.getType(ir::TypeKind::I32), 1)});
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());

      const ExecutionResult Result = Engine.execute("echo", {Malformed});

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::EntryArgumentInvalid);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 2u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "echo");
      expectUnsignedArgument(DiagnosticEntry, 1, core::DiagnosticArgumentName::ArgumentIndex, 0);
      EXPECT_EQ(executionMessage(Result.diagnostics()), "argument 0 of entry function @echo has the wrong type or runtime shape");
    }

    // Verifies that struct SSA construction and extraction cross a real libffi by-value argument and result boundary with platform ABI layout.
    TEST(ExecutionEngineTest, ExecutesStructArgumentAndResultThroughLibffi)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "declare extern \"C\" %Pair @increment_pair(%Pair)\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = insertvalue %Pair zeroinitializer, byte 20, 0\n"
          "  %1 = insertvalue %Pair %0, i32 40, 1\n"
          "  %2 = call %Pair @increment_pair(%Pair %1)\n"
          "  %3 = extractvalue %Pair %2, 1\n"
          "  ret i32 %3\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());
      ASSERT_TRUE(Engine.nativeSymbols().registerSymbol("increment_pair", reinterpret_cast<NativeFunctionAddress>(&incrementPair)));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::I32));
      EXPECT_EQ(Result.returnValue()->integer(), 42u);
    }

    // Verifies that executing an unknown entry fails after successful initialization and produces an execution diagnostic.
    TEST(ExecutionEngineTest, ReportsMissingEntryFunction)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("missing");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::EntryFunctionNotFound);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "missing");
      EXPECT_EQ(executionMessage(Result.diagnostics()), "entry function @missing does not exist");
    }

    // Verifies that a resolved extern cannot be selected as the interpreter entry because it has no InkIR body.
    TEST(ExecutionEngineTest, RejectsExternalEntryFunction)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("print");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::EntryFunctionMustBeDefined);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "print");
      EXPECT_EQ(executionMessage(Result.diagnostics()), "entry function @print must be defined in InkIR");
    }

    // Verifies that recursive InkIR calls stop at the engine depth limit and report the active function rather than overflowing the host stack.
    TEST(ExecutionEngineTest, ReportsMaximumCallDepth)
    {
      TestContext Context;
      ir::Module ModuleValue = makeRecursiveModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("recurse");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::CallDepthLimitExceeded);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 2u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "recurse");
      expectUnsignedArgument(DiagnosticEntry, 1, core::DiagnosticArgumentName::CallDepthLimit, 256);
      EXPECT_EQ(executionMessage(Result.diagnostics()), "maximum InkIR call depth 256 exceeded in function @recurse");
    }

    // Verifies that the interpreter explicitly rejects multi-block functions until branch and control-flow execution are implemented.
    TEST(ExecutionEngineTest, RejectsMultipleBasicBlocks)
    {
      TestContext Context;
      ir::Module ModuleValue = makeMultipleBlockModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::MultipleBasicBlocksUnsupported);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 2u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "main");
      expectUnsignedArgument(DiagnosticEntry, 1, core::DiagnosticArgumentName::BlockCount, 2);
      EXPECT_EQ(executionMessage(Result.diagnostics()), "function @main cannot execute until InkIR has control-flow instructions because it contains 2 basic blocks");
    }

    // Verifies that initialization forwards verifier diagnostics and never attempts to execute an invalid definition without a body.
    TEST(ExecutionEngineTest, RejectsInvalidModuleDuringInitialization)
    {
      TestContext Context;
      const ir::Type &VoidType = Context.IR.getType(ir::TypeKind::Void);
      ir::Module ModuleValue(Context.IR);
      ir::Function Main(VoidType);
      Main.Name = "main";
      ModuleValue.Functions.push_back(std::move(Main));
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::IrDefinedFunctionHasNoBasicBlocks);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::InternalCompilerError);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "main");
      EXPECT_EQ(executionMessage(Result.diagnostics()), "defined function @main must contain at least one basic block");
    }

    // Verifies that unresolved externs stop initialization and broadcast a structured execution diagnostic.
    TEST(ExecutionEngineTest, ReportsUnresolvedExternalThroughExecutionContext)
    {
      TestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR, "ink_test_symbol_that_does_not_exist");
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::ExternalFunctionNotFound);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "ink_test_symbol_that_does_not_exist");
      EXPECT_EQ(Consumer.diagnostics(), Result.diagnostics());
      EXPECT_EQ(core::DiagnosticFormatter().format(DiagnosticEntry).Message, "could not resolve external function @ink_test_symbol_that_does_not_exist");
    }
  } // namespace
} // namespace ink::execution
