#include "ink/execution/execution_engine.h"
#include "ink/execution/runtime/runtime_symbols.h"
#include "ink/ir/model/context.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct NativeIoTestContext
    {
        core::CompilationContext Compilation;
        ir::IRContext IR{Compilation};
        ExecutionContext Execution{Compilation};
    };

    std::vector<std::uint8_t> CapturedBytes;

    extern "C" std::int32_t testRead(std::int32_t Descriptor, std::uint8_t *Data, std::size_t Size)
    {
      constexpr std::array<std::uint8_t, 3> Bytes = {0x00, 0x7F, 0xFF};
      if (Descriptor != 0 || (Data == nullptr && Size != 0))
      {
        return -1;
      }
      const std::size_t Count = std::min(Size, Bytes.size());
      if (Count != 0)
      {
        std::copy_n(Bytes.begin(), Count, Data);
      }
      return static_cast<std::int32_t>(Count);
    }

    extern "C" std::int32_t testWrite(std::int32_t Descriptor, const std::uint8_t *Data, std::size_t Size)
    {
      if (Descriptor != 2 || (Data == nullptr && Size != 0))
      {
        return -1;
      }
      CapturedBytes.clear();
      if (Size != 0)
      {
        CapturedBytes.assign(Data, Data + Size);
      }
      return static_cast<std::int32_t>(Size);
    }

    ir::Module makeReadModule(ir::IRContext &Context)
    {
      const ir::Type &I32Type = Context.getType(ir::TypeKind::I32);
      const ir::Type &BytePointerType = Context.getType(ir::TypeKind::BytePointer);
      const ir::Type &PointerSizeType = Context.getType(ir::TypeKind::PointerSize);
      ir::Module Result(Context);

      ir::Function Read(I32Type);
      Read.Name = "read";
      Read.Kind = ir::FunctionKind::External;
      Read.Convention = ir::CallingConvention::C;
      Read.ParameterTypes = {&I32Type, &BytePointerType, &PointerSizeType};
      Read.HasSideEffects = true;
      Result.Functions.push_back(std::move(Read));

      auto Call = std::make_unique<ir::CallInstruction>(I32Type);
      Call->Result = ir::ValueId{2};
      Call->Callee = ir::FunctionId{0};
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(I32Type, 0));
      Call->Arguments.push_back(std::make_unique<ir::ValueOperand>(BytePointerType, ir::ValueId{0}));
      Call->Arguments.push_back(std::make_unique<ir::ValueOperand>(PointerSizeType, ir::ValueId{1}));
      auto Return = std::make_unique<ir::ReturnInstruction>();
      Return->ReturnValue = std::make_unique<ir::ValueOperand>(I32Type, ir::ValueId{2});

      ir::Function Main(I32Type);
      Main.Name = "main";
      Main.ParameterTypes = {&BytePointerType, &PointerSizeType};
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(Call));
      Entry.Instructions.push_back(std::move(Return));
      Main.Blocks.push_back(std::move(Entry));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    ir::Module makeWriteModule(ir::IRContext &Context)
    {
      const ir::Type &I32Type = Context.getType(ir::TypeKind::I32);
      const ir::Type &ConstBytePointerType = Context.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &PointerSizeType = Context.getType(ir::TypeKind::PointerSize);
      ir::Module Result(Context);
      Result.ByteConstants.push_back({"data", std::string("\0\x7F\xFF", 3)});

      ir::Function Write(I32Type);
      Write.Name = "write";
      Write.Kind = ir::FunctionKind::External;
      Write.Convention = ir::CallingConvention::C;
      Write.ParameterTypes = {&I32Type, &ConstBytePointerType, &PointerSizeType};
      Write.HasSideEffects = true;
      Result.Functions.push_back(std::move(Write));

      auto Call = std::make_unique<ir::CallInstruction>(I32Type);
      Call->Result = ir::ValueId{0};
      Call->Callee = ir::FunctionId{0};
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(I32Type, 2));
      Call->Arguments.push_back(std::make_unique<ir::GlobalAddressOperand>(ConstBytePointerType, ir::GlobalId{0}, 0));
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(PointerSizeType, 3));
      auto Return = std::make_unique<ir::ReturnInstruction>();
      Return->ReturnValue = std::make_unique<ir::ValueOperand>(I32Type, ir::ValueId{0});

      ir::Function Main(I32Type);
      Main.Name = "main";
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(Call));
      Entry.Instructions.push_back(std::move(Return));
      Main.Blocks.push_back(std::move(Entry));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    void expectI32Result(const ExecutionResult &Result, std::int32_t Expected)
    {
      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      const std::optional<std::uint64_t> Value = Result.returnValue()->integer();
      ASSERT_TRUE(Value.has_value());
      EXPECT_EQ(static_cast<std::int32_t>(*Value), Expected);
    }

    // Verifies that read and write use the exact native addresses injected into their ExecutionContext and preserve binary bytes through libffi.
    TEST(NativeIoTest, ExecutesInjectedReadAndWriteAddresses)
    {
      NativeIoTestContext Context;
      const NativeFunctionAddress ReadAddress = reinterpret_cast<NativeFunctionAddress>(&testRead);
      const NativeFunctionAddress WriteAddress = reinterpret_cast<NativeFunctionAddress>(&testWrite);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("read", ReadAddress));
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("write", WriteAddress));
      ir::Module ReadModule = makeReadModule(Context.IR);
      ir::Module WriteModule = makeWriteModule(Context.IR);
      ExecutionEngine ReadEngine(Context.Execution, ReadModule);
      ExecutionEngine WriteEngine(Context.Execution, WriteModule);
      std::array<std::uint8_t, 5> Buffer = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
      RuntimeValueArena Arguments;
      RuntimeValueRef Pointer = Arguments.mutablePointerValue(Context.IR.getType(ir::TypeKind::BytePointer), Buffer.data());
      RuntimeValueRef Size = Arguments.integerValue(Context.IR.getType(ir::TypeKind::PointerSize), Buffer.size());

      expectI32Result(ReadEngine.execute("main", {Pointer, Size}), 3);
      expectI32Result(WriteEngine.execute("main"), 3);
      EXPECT_EQ(Buffer, (std::array<std::uint8_t, 5>{0x00, 0x7F, 0xFF, 0xAA, 0xAA}));
      EXPECT_EQ(CapturedBytes, (std::vector<std::uint8_t>{0x00, 0x7F, 0xFF}));
      EXPECT_EQ(Context.Execution.nativeSymbols().findAddress("read"), ReadAddress);
      EXPECT_EQ(Context.Execution.nativeSymbols().findAddress("write"), WriteAddress);
    }

    // Verifies that platform runtime I/O addresses are added only to the selected ExecutionContext and do not leak into an empty context.
    TEST(NativeIoTest, RegistersPlatformIoOnlyInSelectedExecutionContext)
    {
      NativeIoTestContext Execute;
      NativeIoTestContext Comptime;

      ASSERT_TRUE(registerRuntimeSymbols(Execute.Execution.nativeSymbols()));
      EXPECT_NE(Execute.Execution.nativeSymbols().findAddress("read"), nullptr);
      EXPECT_NE(Execute.Execution.nativeSymbols().findAddress("write"), nullptr);
      EXPECT_EQ(Comptime.Execution.nativeSymbols().findAddress("read"), nullptr);
      EXPECT_EQ(Comptime.Execution.nativeSymbols().findAddress("write"), nullptr);
    }
  } // namespace
} // namespace ink::execution
