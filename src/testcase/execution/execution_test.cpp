#include "ink/execution/execution_engine.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace ink::execution
{
  namespace
  {
    struct TestContext
    {
      core::CompilationContext Compilation;
      ExecutionContext Execution{Compilation};
    };

    std::string CapturedOutput;

    extern "C" std::int32_t captureOutput(const std::uint8_t *Data, std::size_t Size)
    {
      if (Size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
      {
        return -1;
      }
      CapturedOutput.append(reinterpret_cast<const char *>(Data), Size);
      return static_cast<std::int32_t>(Size);
    }

    ir::Module makeHelloWorldModule(std::string ExternalName = "ink_rt_write_stdout")
    {
      ir::Module Result;
      Result.ByteConstants.push_back({"str.0", "Hello, world!\n"});

      ir::Function WriteStdout;
      WriteStdout.Name = std::move(ExternalName);
      WriteStdout.Kind = ir::FunctionKind::External;
      WriteStdout.Convention = ir::CallingConvention::C;
      WriteStdout.ResultType = ir::TypeKind::I32;
      WriteStdout.ParameterTypes = {ir::TypeKind::ConstBytePointer, ir::TypeKind::PointerSize};
      WriteStdout.HasSideEffects = true;
      Result.Functions.push_back(std::move(WriteStdout));

      auto Call = std::make_unique<ir::CallInstruction>();
      Call->Result = ir::ValueId{0};
      Call->ResultType = ir::TypeKind::I32;
      Call->Callee = ir::FunctionId{0};
      Call->Arguments.push_back(std::make_unique<ir::GlobalAddressOperand>(ir::TypeKind::ConstBytePointer, ir::GlobalId{0}, 0));
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(ir::TypeKind::PointerSize, 14));

      ir::Function Main;
      Main.Name = "main";
      ir::BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(Call));
      Entry.Instructions.push_back(std::make_unique<ir::ReturnInstruction>());
      Main.Blocks.push_back(std::move(Entry));
      Result.Functions.push_back(std::move(Main));
      return Result;
    }

    ir::Module makeInternalCallModule()
    {
      ir::Module Result;

      ir::Function Identity;
      Identity.Name = "identity";
      Identity.ResultType = ir::TypeKind::I32;
      Identity.ParameterTypes = {ir::TypeKind::I32};
      ir::BasicBlock IdentityEntry;
      IdentityEntry.Name = "entry";
      auto IdentityReturn = std::make_unique<ir::ReturnInstruction>();
      IdentityReturn->ReturnValue = std::make_unique<ir::ValueOperand>(ir::TypeKind::I32, ir::ValueId{0});
      IdentityEntry.Instructions.push_back(std::move(IdentityReturn));
      Identity.Blocks.push_back(std::move(IdentityEntry));
      Result.Functions.push_back(std::move(Identity));

      auto Call = std::make_unique<ir::CallInstruction>();
      Call->Result = ir::ValueId{0};
      Call->ResultType = ir::TypeKind::I32;
      Call->Callee = ir::FunctionId{0};
      Call->Arguments.push_back(std::make_unique<ir::IntegerConstant>(ir::TypeKind::I32, 42));
      auto MainReturn = std::make_unique<ir::ReturnInstruction>();
      MainReturn->ReturnValue = std::make_unique<ir::ValueOperand>(ir::TypeKind::I32, ir::ValueId{0});

      ir::Function Main;
      Main.Name = "main";
      Main.ResultType = ir::TypeKind::I32;
      ir::BasicBlock MainEntry;
      MainEntry.Name = "entry";
      MainEntry.Instructions.push_back(std::move(Call));
      MainEntry.Instructions.push_back(std::move(MainReturn));
      Main.Blocks.push_back(std::move(MainEntry));
      Result.Functions.push_back(std::move(Main));
      return Result;
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

      EXPECT_TRUE(Registry.registerSymbol("capture", CaptureAddress));
      EXPECT_TRUE(Registry.registerSymbol("capture", CaptureAddress));
      EXPECT_EQ(Registry.findAddress("capture"), CaptureAddress);
      ASSERT_TRUE(Registry.findName(CaptureAddress).has_value());
      EXPECT_EQ(*Registry.findName(CaptureAddress), "capture");
#if defined(_WIN32)
      EXPECT_TRUE(Registry.resolveAndRegister("WriteFile"));
      EXPECT_NE(Registry.findAddress("WriteFile"), nullptr);
#elif defined(__linux__)
      EXPECT_TRUE(Registry.resolveAndRegister("write"));
      EXPECT_NE(Registry.findAddress("write"), nullptr);
#endif
    }

    // Verifies that Hello World crosses the extern boundary through libffi while preserving the byte constant and ptrsize arguments.
    TEST(ExecutionEngineTest, ExecutesExternHelloWorldThroughLibffi)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule();
      ExecutionEngine Engine(Context.Execution, ModuleValue);
      CapturedOutput.clear();
      const NativeFunctionAddress CaptureAddress = reinterpret_cast<NativeFunctionAddress>(&captureOutput);
      ASSERT_TRUE(Engine.nativeSymbols().registerSymbol("ink_rt_write_stdout", CaptureAddress));

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Initialization.succeeded());
      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->type(), ir::TypeKind::Void);
      EXPECT_EQ(CapturedOutput, "Hello, world!\n");
      EXPECT_EQ(Engine.nativeSymbols().findAddress("ink_rt_write_stdout"), CaptureAddress);
    }

    // Verifies that the default runtime extern writes Hello World through the platform stdout interface and returns the written byte count.
    TEST(ExecutionEngineTest, ExecutesHelloWorldThroughDefaultPlatformRuntime)
    {
      TestContext Context;
      ir::Module ModuleValue = makeHelloWorldModule();
      ir::Function &Main = ModuleValue.Functions[1];
      Main.ResultType = ir::TypeKind::I32;
      auto &Return = static_cast<ir::ReturnInstruction &>(*Main.Blocks[0].Instructions[1]);
      Return.ReturnValue = std::make_unique<ir::ValueOperand>(ir::TypeKind::I32, ir::ValueId{0});
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->type(), ir::TypeKind::I32);
      EXPECT_EQ(Result.returnValue()->integer(), 14u);
      EXPECT_NE(Engine.nativeSymbols().findAddress("ink_rt_write_stdout"), nullptr);
    }

    // Verifies that an internal call creates a callee frame, maps parameter %0, and returns the caller's SSA result.
    TEST(ExecutionEngineTest, ExecutesInternalCallAndSsaReturn)
    {
      TestContext Context;
      ir::Module ModuleValue = makeInternalCallModule();
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.returnValue().has_value());
      EXPECT_EQ(Result.returnValue()->type(), ir::TypeKind::I32);
      EXPECT_EQ(Result.returnValue()->integer(), 42u);
    }

    // Verifies that unresolved externs stop initialization and broadcast a structured execution diagnostic.
    TEST(ExecutionEngineTest, ReportsUnresolvedExternalThroughExecutionContext)
    {
      TestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      ir::Module ModuleValue = makeHelloWorldModule("ink_test_symbol_that_does_not_exist");
      ExecutionEngine Engine(Context.Execution, ModuleValue);

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ExecutionFailed);
      EXPECT_EQ(Consumer.diagnostics(), Result.diagnostics());
      EXPECT_NE(core::DiagnosticFormatter().format(Result.diagnostics()[0]).Message.find("could not resolve external function"), std::string::npos);
    }
  } // namespace
} // namespace ink::execution
