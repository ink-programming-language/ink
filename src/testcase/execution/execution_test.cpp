#include "ink/execution/execution_engine.h"
#include "ink/ir/model/constant.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/operand.h"
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

    class CountingPointerValue final : public RuntimeValue
    {
      public:
        CountingPointerValue(const ir::Type &ValueType, const void *Value, std::size_t &PointerReadCount)
            : ValueType(&ValueType),
              Value(Value),
              PointerReadCount(&PointerReadCount)
        {
        }

        RuntimeValueKind kind() const noexcept override
        {
          return RuntimeValueKind::Pointer;
        }

        const ir::Type &type() const noexcept override
        {
          return *ValueType;
        }

        const void *pointer() const noexcept override
        {
          ++*PointerReadCount;
          return Value;
        }

      private:
        const ir::Type *ValueType;
        const void *Value;
        std::size_t *PointerReadCount;
    };

    class UncheckedIntegerValue final : public RuntimeValue
    {
      public:
        UncheckedIntegerValue(const ir::Type &ValueType, std::uint64_t Value)
            : ValueType(&ValueType),
              Value(Value)
        {
        }

        RuntimeValueKind kind() const noexcept override
        {
          return RuntimeValueKind::Integer;
        }

        const ir::Type &type() const noexcept override
        {
          return *ValueType;
        }

        std::optional<std::uint64_t> integer() const noexcept override
        {
          return Value;
        }

      private:
        const ir::Type *ValueType;
        std::uint64_t Value;
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

    ir::Module makeHelloWorldModule(ir::IRContext &Context, std::string ExternalName = "capture_output")
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
      RuntimeValueArena Values;
      const RuntimeValueRef Integer = Values.integerValue(I32Type, 42);
      const std::uint8_t Byte = 7;
      const RuntimeValueRef Pointer = Values.pointerValue(PointerType, &Byte);
      const RuntimeValueRef First = Values.integerValue(I32Type, 1);
      const RuntimeValueRef Second = Values.integerValue(I32Type, 2);
      const RuntimeValueRef Pair = Values.aggregateValue(PairType, {First, Second});

      ASSERT_NE(Integer, nullptr);
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(Pair, nullptr);
      EXPECT_EQ(&Integer->type(), &I32Type);
      EXPECT_EQ(Integer->kind(), RuntimeValueKind::Integer);
      EXPECT_EQ(Integer->integer(), 42u);
      EXPECT_EQ(Integer->pointer(), nullptr);
      EXPECT_EQ(Pointer->kind(), RuntimeValueKind::Pointer);
      EXPECT_EQ(Pointer->pointer(), &Byte);
      EXPECT_FALSE(Pointer->integer().has_value());
      ASSERT_EQ(Pair->fieldCount(), 2u);
      ASSERT_NE(Pair->field(0), nullptr);
      EXPECT_EQ(Pair->field(0)->integer(), 1u);
      EXPECT_EQ(Pair->field(1)->integer(), 2u);
      EXPECT_EQ(Pair->field(2), nullptr);
      EXPECT_TRUE(Values.owns(Integer));
      EXPECT_TRUE(Values.owns(Pair));
    }

    // Verifies that arena factories reject payload/type mismatches and aggregate fields that are null, mistyped, or owned by another arena.
    TEST(RuntimeValueTest, RejectsInvalidFactoriesAndForeignAggregateFields)
    {
      TestContext Context;
      const ir::Type &VoidType = Context.IR.getType(ir::TypeKind::Void);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &I32Type});
      RuntimeValueArena Values;
      RuntimeValueArena ForeignValues;
      const RuntimeValueRef First = Values.integerValue(I32Type, 1);
      const RuntimeValueRef Second = Values.integerValue(I32Type, 2);
      const RuntimeValueRef Byte = Values.integerValue(ByteType, 2);
      const RuntimeValueRef Foreign = ForeignValues.integerValue(I32Type, 3);

      EXPECT_EQ(Values.voidValue(I32Type), nullptr);
      EXPECT_EQ(Values.integerValue(VoidType, 1), nullptr);
      EXPECT_EQ(Values.integerValue(PointerType, 1), nullptr);
      EXPECT_EQ(Values.pointerValue(I32Type, nullptr), nullptr);
      EXPECT_EQ(Values.aggregateValue(PairType, {First}), nullptr);
      EXPECT_EQ(Values.aggregateValue(PairType, {First, nullptr}), nullptr);
      EXPECT_EQ(Values.aggregateValue(PairType, {First, Byte}), nullptr);
      EXPECT_EQ(Values.aggregateValue(PairType, {First, Foreign}), nullptr);
      EXPECT_NE(Values.aggregateValue(PairType, {First, Second}), nullptr);
    }

    // Verifies the exact accepted integer domains for bool, byte, i32, and the native pointer-sized integer type.
    TEST(RuntimeValueTest, EnforcesIntegerPayloadRanges)
    {
      TestContext Context;
      const ir::Type &BoolType = Context.IR.getType(ir::TypeKind::Bool);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      RuntimeValueArena Values;
      const std::uint64_t I32Minimum = static_cast<std::uint64_t>(static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::min()));
      const std::uint64_t I32Maximum = static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max());

      EXPECT_NE(Values.integerValue(BoolType, 0), nullptr);
      EXPECT_NE(Values.integerValue(BoolType, 1), nullptr);
      EXPECT_EQ(Values.integerValue(BoolType, 2), nullptr);
      EXPECT_NE(Values.integerValue(ByteType, std::numeric_limits<std::uint8_t>::max()), nullptr);
      EXPECT_EQ(Values.integerValue(ByteType, static_cast<std::uint64_t>(std::numeric_limits<std::uint8_t>::max()) + 1), nullptr);
      EXPECT_NE(Values.integerValue(I32Type, I32Minimum), nullptr);
      EXPECT_NE(Values.integerValue(I32Type, I32Maximum), nullptr);
      EXPECT_EQ(Values.integerValue(I32Type, I32Maximum + 1), nullptr);
      EXPECT_NE(Values.integerValue(PointerSizeType, std::numeric_limits<std::size_t>::max()), nullptr);
      if constexpr (sizeof(std::size_t) < sizeof(std::uint64_t))
      {
        EXPECT_EQ(Values.integerValue(PointerSizeType, static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) + 1), nullptr);
      }
    }

    // Verifies that cloning recursively moves a shared aggregate graph into another arena while preserving graph sharing and borrowed pointer payloads.
    TEST(RuntimeValueTest, DeepClonesAggregateGraphsIntoAnotherArena)
    {
      TestContext Context;
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &PointerType});
      const ir::StructType &OuterType = Context.IR.createStructType("Outer", {&PairType, &PairType});
      const std::uint8_t Byte = 9;
      RuntimeValueArena ClonedValues;
      RuntimeValueRef ClonedOuter = nullptr;
      {
        RuntimeValueArena SourceValues;
        const RuntimeValueRef Integer = SourceValues.integerValue(I32Type, 17);
        const RuntimeValueRef Pointer = SourceValues.pointerValue(PointerType, &Byte);
        const RuntimeValueRef Pair = SourceValues.aggregateValue(PairType, {Integer, Pointer});
        const RuntimeValueRef Outer = SourceValues.aggregateValue(OuterType, {Pair, Pair});

        ClonedOuter = ClonedValues.clone(*Outer);

        ASSERT_NE(ClonedOuter, nullptr);
        EXPECT_NE(ClonedOuter, Outer);
        EXPECT_NE(ClonedOuter->field(0), Pair);
      }

      ASSERT_TRUE(ClonedValues.owns(ClonedOuter));
      ASSERT_NE(ClonedOuter->field(0), nullptr);
      EXPECT_EQ(ClonedOuter->field(0), ClonedOuter->field(1));
      EXPECT_TRUE(ClonedValues.owns(ClonedOuter->field(0)));
      ASSERT_NE(ClonedOuter->field(0)->field(0), nullptr);
      ASSERT_NE(ClonedOuter->field(0)->field(1), nullptr);
      EXPECT_TRUE(ClonedValues.owns(ClonedOuter->field(0)->field(0)));
      EXPECT_TRUE(ClonedValues.owns(ClonedOuter->field(0)->field(1)));
      EXPECT_EQ(ClonedOuter->field(0)->field(0)->integer(), 17u);
      EXPECT_EQ(ClonedOuter->field(0)->field(1)->pointer(), &Byte);
    }

    // Verifies that ExecutionContext composes around the shared compilation diagnostics service and owns its native registry.
    TEST(ExecutionContextTest, SharesCompilationDiagnosticEngine)
    {
      TestContext Context;

      EXPECT_EQ(&Context.Execution.compilationContext(), &Context.Compilation);
      EXPECT_EQ(&Context.Execution.diagnosticEngine(), &Context.Compilation.diagnosticEngine());
      EXPECT_TRUE(Context.Execution.nativeSymbols().symbols().empty());
    }

    // Verifies that native symbols registered in one ExecutionContext do not leak into another context.
    TEST(ExecutionContextTest, OwnsIndependentNativeSymbolRegistries)
    {
      TestContext First;
      TestContext Second;
      const NativeFunctionAddress FirstAddress = reinterpret_cast<NativeFunctionAddress>(&captureOutput);
      const NativeFunctionAddress SecondAddress = reinterpret_cast<NativeFunctionAddress>(&recordValue);

      ASSERT_TRUE(First.Execution.nativeSymbols().registerSymbol("probe", FirstAddress));
      ASSERT_TRUE(Second.Execution.nativeSymbols().registerSymbol("probe", SecondAddress));
      EXPECT_EQ(First.Execution.nativeSymbols().findAddress("probe"), FirstAddress);
      EXPECT_EQ(Second.Execution.nativeSymbols().findAddress("probe"), SecondAddress);
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

    // Verifies that repeated initialization is idempotent and does not mutate the native symbol registry.
    TEST(ExecutionEngineTest, InitializesIdempotently)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult First = Engine.initialize();
      const std::size_t FirstSymbolCount = Context.Execution.nativeSymbols().symbols().size();
      const InitializationResult Second = Engine.initialize();

      EXPECT_TRUE(First.succeeded());
      EXPECT_TRUE(Second.succeeded());
      EXPECT_EQ(Context.Execution.nativeSymbols().symbols().size(), FirstSymbolCount);
    }

    // Verifies that Hello World crosses the extern boundary through libffi while preserving the byte constant and ptrsize arguments.
    TEST(ExecutionEngineTest, ExecutesExternHelloWorldThroughLibffi)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      CapturedOutput.clear();
      const NativeFunctionAddress CaptureAddress = reinterpret_cast<NativeFunctionAddress>(&captureOutput);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("capture_output", CaptureAddress));

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Initialization.succeeded());
      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::Void));
      EXPECT_EQ(CapturedOutput, "Hello, world!\n");
      EXPECT_EQ(Context.Execution.nativeSymbols().findAddress("capture_output"), CaptureAddress);
    }

    // Verifies that bool, byte, i32, ptrsize, and pointer arguments plus a ptrsize result are marshalled through libffi without representation leakage.
    TEST(ExecutionEngineTest, MarshalsEveryCAbiPrimitiveExternType)
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
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("combine_primitive_values", reinterpret_cast<NativeFunctionAddress>(&combinePrimitiveValues)));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(&Result.returnValue()->type(), &Context.IR.getType(ir::TypeKind::PointerSize));
      EXPECT_EQ(Result.returnValue()->integer(), 15u);
    }

    // Verifies that a module byte-constant pointer retains tracked provenance through an extern and expires on Engine shutdown.
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
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("identity_pointer", reinterpret_cast<NativeFunctionAddress>(&identityPointer)));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_TRUE(Result.returnValue()->memoryAlive());
      ASSERT_NE(Result.returnValue()->pointer(), nullptr);
      EXPECT_EQ(*static_cast<const char *>(Result.returnValue()->pointer()), 'x');
      EXPECT_FALSE(Result.returnValue()->integer().has_value());
      EXPECT_TRUE(Engine.shutdown().succeeded());
      EXPECT_FALSE(Result.returnValue()->memoryAlive());
      EXPECT_EQ(Result.returnValue()->pointer(), nullptr);
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
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_value", reinterpret_cast<NativeFunctionAddress>(&recordValue)));
      RecordedValue = 0;

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->type().kind(), ir::TypeKind::Void);
      EXPECT_EQ(RecordedValue, 73);
    }

    // Verifies that an internal call creates a callee frame, maps parameter %0, and returns the caller's SSA result.
    TEST(ExecutionEngineTest, ExecutesInternalCallAndSsaReturn)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
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

      const ExecutionResult Result = [&Engine, &I32Type]()
      {
        RuntimeValueArena Arguments;
        return Engine.execute("identity", {Arguments.integerValue(I32Type, 91)});
      }();

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->integer(), 91u);
    }

    // Verifies that an unused borrowed argument is not eagerly cloned or asked to materialize its pointer payload.
    TEST(ExecutionEngineTest, DoesNotCloneUnusedBorrowedArgument)
    {
      TestContext Context;
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      ir::Module ModuleValue(Context.IR);
      ir::Function Ignore(I32Type);
      Ignore.Name = "ignore";
      Ignore.ParameterTypes = {&PointerType};
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      auto Return = std::make_unique<ir::ReturnInstruction>();
      Return->ReturnValue = std::make_unique<ir::IntegerConstant>(I32Type, 42);
      Entry.Instructions.push_back(std::move(Return));
      Ignore.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Ignore));
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      const std::uint8_t Byte = 7;
      std::size_t PointerReadCount = 0;
      const CountingPointerValue Argument(PointerType, &Byte, PointerReadCount);

      const ExecutionResult Result = Engine.execute("ignore", {&Argument});

      ASSERT_TRUE(Result.succeeded());
      EXPECT_EQ(Result.returnValue()->integer(), 42u);
      EXPECT_EQ(PointerReadCount, 0u);
    }

    // Verifies that removing eager argument cloning does not allow an unused borrowed integer with an out-of-range payload to enter execution.
    TEST(ExecutionEngineTest, RejectsOutOfRangeUnusedBorrowedArgument)
    {
      TestContext Context;
      const ir::Type &BoolType = Context.IR.getType(ir::TypeKind::Bool);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      ir::Module ModuleValue(Context.IR);
      ir::Function Ignore(I32Type);
      Ignore.Name = "ignore";
      Ignore.ParameterTypes = {&BoolType};
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      auto Return = std::make_unique<ir::ReturnInstruction>();
      Return->ReturnValue = std::make_unique<ir::IntegerConstant>(I32Type, 42);
      Entry.Instructions.push_back(std::move(Return));
      Ignore.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Ignore));
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      const UncheckedIntegerValue Argument(BoolType, 2);

      const ExecutionResult Result = Engine.execute("ignore", {&Argument});

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::EntryArgumentInvalid);
    }

    // Verifies that parameter SSA value zero and the immediately following instruction result at value one map directly to consecutive frame indices.
    TEST(ExecutionEngineTest, UsesDenseSsaValueIdsAsFrameIndicesAfterParameters)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @identity(i32 %0) {\n"
          "entry:\n"
          "  ret i32 %0\n"
          "}\n"
          "define i32 @call_identity(i32 %0) {\n"
          "entry:\n"
          "  %1 = call i32 @identity(i32 %0)\n"
          "  ret i32 %1\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());
      RuntimeValueArena Arguments;
      const RuntimeValueRef Argument = Arguments.integerValue(Context.IR.getType(ir::TypeKind::I32), 37);

      const ExecutionResult Result = Engine.execute("call_identity", {Argument});

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->integer(), 37u);
    }

    // Verifies that result copies and moves retain the arena owning the returned value and clear moved-from raw references.
    TEST(ExecutionEngineTest, KeepsReturnedValueAliveAcrossResultCopiesAndMoves)
    {
      TestContext Context;
      ExecutionResult Original;
      {
        ir::Module ModuleValue = makeInternalCallModule(Context.IR);
        ExecutionEngine Engine(Context.Execution, ModuleValue);
        Original = Engine.execute("main");
      }
      ASSERT_TRUE(Original.succeeded());
      const RuntimeValueRef Expected = Original.returnValue();

      ExecutionResult Copied = Original;
      ExecutionResult CopyAssigned;
      CopyAssigned = Original;
      ExecutionResult Moved(std::move(Original));
      ExecutionResult MoveAssigned;
      MoveAssigned = std::move(Copied);

      EXPECT_EQ(Original.returnValue(), nullptr);
      EXPECT_EQ(Copied.returnValue(), nullptr);
      ASSERT_TRUE(Moved.succeeded());
      ASSERT_TRUE(MoveAssigned.succeeded());
      ASSERT_TRUE(CopyAssigned.succeeded());
      EXPECT_EQ(Moved.returnValue(), Expected);
      EXPECT_EQ(MoveAssigned.returnValue(), Expected);
      EXPECT_EQ(CopyAssigned.returnValue(), Expected);
      EXPECT_EQ(Moved.returnValue()->integer(), 42u);
      EXPECT_EQ(MoveAssigned.returnValue()->integer(), 42u);
      EXPECT_EQ(CopyAssigned.returnValue()->integer(), 42u);
    }

    // Verifies that entry invocation rejects both an incorrect argument count and a RuntimeValue with the wrong canonical Type object.
    TEST(ExecutionEngineTest, RejectsInvalidEntryArguments)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      RuntimeValueArena Arguments;

      const ExecutionResult WrongCount = Engine.execute("identity");
      const ExecutionResult WrongType = Engine.execute("identity", {Arguments.integerValue(Context.IR.getType(ir::TypeKind::Byte), 1)});

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
      ASSERT_NE(Result.returnValue(), nullptr);
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

      const ExecutionResult Result = [&Engine, &I32Type, &PairType]()
      {
        RuntimeValueArena Arguments;
        const RuntimeValueRef First = Arguments.integerValue(I32Type, 7);
        const RuntimeValueRef Second = Arguments.integerValue(I32Type, 8);
        const RuntimeValueRef Argument = Arguments.aggregateValue(PairType, {First, Second});
        return Engine.execute("echo", {Argument});
      }();

      ASSERT_TRUE(Result.succeeded()) << (Result.diagnostics().empty() ? std::string() : executionMessage(Result.diagnostics()));
      ASSERT_NE(Result.returnValue(), nullptr);
      ASSERT_EQ(Result.returnValue()->fieldCount(), 2u);
      EXPECT_EQ(Result.returnValue()->field(0)->integer(), 7u);
      EXPECT_EQ(Result.returnValue()->field(1)->integer(), 8u);
    }

    // Verifies that insertvalue lazily imports a borrowed aggregate into the execution arena before constructing its replacement value.
    TEST(ExecutionEngineTest, LazilyImportsBorrowedAggregateForInsertValue)
    {
      TestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {i32, i32}\n"
          "define %Pair @replace_second(%Pair %0) {\n"
          "entry:\n"
          "  %1 = insertvalue %Pair %0, i32 99, 1\n"
          "  ret %Pair %1\n"
          "}\n";
      ir::DeserializeResult Deserialized = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Deserialized.succeeded());
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::StructType &PairType = *Deserialized.module()->StructTypes[0];
      ExecutionEngine Engine(Context.Execution, *Deserialized.module());

      const ExecutionResult Result = [&Engine, &I32Type, &PairType]()
      {
        RuntimeValueArena Arguments;
        const RuntimeValueRef First = Arguments.integerValue(I32Type, 7);
        const RuntimeValueRef Second = Arguments.integerValue(I32Type, 8);
        const RuntimeValueRef Pair = Arguments.aggregateValue(PairType, {First, Second});
        return Engine.execute("replace_second", {Pair});
      }();

      ASSERT_TRUE(Result.succeeded()) << (Result.diagnostics().empty() ? std::string() : executionMessage(Result.diagnostics()));
      ASSERT_NE(Result.returnValue(), nullptr);
      ASSERT_EQ(Result.returnValue()->fieldCount(), 2u);
      EXPECT_EQ(Result.returnValue()->field(0)->integer(), 7u);
      EXPECT_EQ(Result.returnValue()->field(1)->integer(), 99u);
    }

    // Verifies that an arena rejects a struct with missing fields and that a null value cannot enter an execution frame.
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
      RuntimeValueArena Arguments;
      const RuntimeValueRef Field = Arguments.integerValue(Context.IR.getType(ir::TypeKind::I32), 1);
      const RuntimeValueRef Malformed = Arguments.aggregateValue(PairType, {Field});
      ASSERT_EQ(Malformed, nullptr);
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
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("increment_pair", reinterpret_cast<NativeFunctionAddress>(&incrementPair)));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded()) << (Result.diagnostics().empty() ? std::string() : executionMessage(Result.diagnostics()));
      ASSERT_NE(Result.returnValue(), nullptr);
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
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("capture_output", reinterpret_cast<NativeFunctionAddress>(&captureOutput)));

      const ExecutionResult Result = Engine.execute("capture_output");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      const core::Diagnostic &DiagnosticEntry = Result.diagnostics()[0];
      EXPECT_EQ(DiagnosticEntry.Kind, core::DiagnosticKind::EntryFunctionMustBeDefined);
      EXPECT_EQ(DiagnosticEntry.classification(), core::DiagnosticClass::User);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 1u);
      expectStringArgument(DiagnosticEntry, 0, core::DiagnosticArgumentName::FunctionName, "capture_output");
      EXPECT_EQ(executionMessage(Result.diagnostics()), "entry function @capture_output must be defined in InkIR");
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

    // Verifies that an unreachable block does not prevent a multi-block function from returning through its entry block.
    TEST(ExecutionEngineTest, IgnoresUnreachableBasicBlock)
    {
      TestContext Context;
      ir::Module ModuleValue = makeMultipleBlockModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.diagnostics().empty());
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

    // Verifies that a declared but unreachable external function neither changes the context registry nor prevents execution.
    TEST(ExecutionEngineTest, DoesNotResolveUnusedExternalFunction)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR, "ink_test_unused_external");
      ModuleValue.Functions[1].Blocks[0].Instructions.erase(ModuleValue.Functions[1].Blocks[0].Instructions.begin());
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult Result = Engine.execute("main");

      EXPECT_TRUE(Initialization.succeeded());
      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Context.Execution.nativeSymbols().symbols().empty());
    }

    // Verifies that an external address missing from this ExecutionContext is reported only when its call is reached.
    TEST(ExecutionEngineTest, ReportsUnresolvedExternalThroughExecutionContext)
    {
      TestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR, "ink_test_symbol_that_does_not_exist");
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Initialization.succeeded());
      EXPECT_TRUE(Initialization.diagnostics().empty());
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

    // Verifies that a missing external lookup is retried after its address is registered in the same ExecutionContext.
    TEST(ExecutionEngineTest, ResolvesExternalRegisteredAfterPreviousFailure)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule(Context.IR);
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      CapturedOutput.clear();

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult Missing = Engine.execute("main");
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("capture_output", reinterpret_cast<NativeFunctionAddress>(&captureOutput)));
      const ExecutionResult Resolved = Engine.execute("main");

      ASSERT_TRUE(Initialization.succeeded());
      ASSERT_FALSE(Missing.succeeded());
      ASSERT_EQ(Missing.diagnostics().size(), 1u);
      EXPECT_EQ(Missing.diagnostics()[0].Kind, core::DiagnosticKind::ExternalFunctionNotFound);
      EXPECT_TRUE(Resolved.succeeded());
      EXPECT_EQ(CapturedOutput, "Hello, world!\n");
    }

    // Verifies that execution rejects a module whose frozen target differs from the ExecutionContext target.
    TEST(ExecutionEngineTest, RejectsMismatchedModuleAndExecutionTargets)
    {
      core::CompilationContext ModuleCompilation(core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian));
      ir::IRContext ModuleIR(ModuleCompilation);
      const ir::DeserializeResult Parsed = ir::deserialize(ModuleIR, "inkir 1\ndefine void @main() {\nentry:\n  ret void\n}\n");
      core::CompilationContext ExecutionCompilation(core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::BigEndian));
      ExecutionContext Execution(ExecutionCompilation);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Execution, *Parsed.module());

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ExecutionTargetMismatch);
    }

    // Verifies that a ptrsize entry argument is revalidated against the destination target rather than its source arena.
    TEST(ExecutionEngineTest, RejectsEntryPointerSizeOutsideTargetRange)
    {
      const core::TargetContext Target32(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian);
      core::CompilationContext Compilation(Target32);
      ir::IRContext IR(Compilation);
      ExecutionContext Execution(Compilation);
      const ir::DeserializeResult Parsed = ir::deserialize(IR, "inkir 1\ndefine ptrsize @main(ptrsize %0) {\nentry:\n  ret ptrsize %0\n}\n");
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Execution, *Parsed.module());
      RuntimeValueArena SourceValues(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const RuntimeValueRef TooWide = SourceValues.integerValue(IR.getType(ir::TypeKind::PointerSize), 0x100000000ULL);
      ASSERT_NE(TooWide, nullptr);

      const ExecutionResult Result = Engine.execute("main", {TooWide});

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::EntryArgumentInvalid);
    }

    // Verifies that a bounded global address remains available to typed load when executing for a non-native target with compatible byte representation.
    TEST(ExecutionEngineTest, LoadsGlobalAddressForSyntheticTarget)
    {
      const core::TargetContext Native = core::TargetContext::native();
      core::CompilationContext Compilation(core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
      ir::IRContext IR(Compilation);
      ExecutionContext Execution(Compilation);
      const std::string Text =
          "inkir 1\n"
          "@data = private constant [1 x byte] c\"x\"\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = load byte, const byte* @data[0]\n"
          "  ret byte %0\n"
          "}\n";
      const ir::DeserializeResult Parsed = ir::deserialize(IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Execution, *Parsed.module());

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->integer(), static_cast<std::uint64_t>('x'));
    }

    // Verifies that a synthetic target rejects an external call before evaluating its global-address argument or crossing libffi.
    TEST(ExecutionEngineTest, RejectsExternalCallForSyntheticTarget)
    {
      const core::TargetContext Native = core::TargetContext::native();
      core::CompilationContext Compilation(core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
      ir::IRContext IR(Compilation);
      ExecutionContext Execution(Compilation);
      const std::string Text =
          "inkir 1\n"
          "@data = private constant [1 x byte] c\"x\"\n"
          "declare extern \"C\" void @external(const byte*) [sideeffect]\n"
          "define void @main() {\n"
          "entry:\n"
          "  call void @external(const byte* @data[0])\n"
          "  ret void\n"
          "}\n";
      const ir::DeserializeResult Parsed = ir::deserialize(IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Execution, *Parsed.module());

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ExternalFunctionTargetUnsupported);
    }
  } // namespace
} // namespace ink::execution
