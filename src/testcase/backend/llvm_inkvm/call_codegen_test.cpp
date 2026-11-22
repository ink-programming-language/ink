#include "ink/backend/llvm_inkvm/inkvm_target.h"
#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace ink::backend::llvm_inkvm
{
    namespace
    {
        std::unique_ptr<vm::MachineModule> compileIRToMachineModule(std::string_view IRText, std::string_view ModuleName)
        {
            llvm::LLVMContext Context;
            llvm::SMDiagnostic ParseError;
            auto IRModule = llvm::parseAssemblyString(llvm::StringRef(IRText.data(), IRText.size()), ParseError, Context);
            EXPECT_NE(IRModule, nullptr);
            if (IRModule == nullptr)
            {
                return nullptr;
            }
            IRModule->setModuleIdentifier(llvm::StringRef(ModuleName.data(), ModuleName.size()));

            auto Compiled = emitInkVMMachineModule(*IRModule);
            EXPECT_TRUE(Compiled.has_value()) << (Compiled.has_value() ? std::string() : Compiled.error());
            return Compiled.has_value() ? std::move(*Compiled) : nullptr;
        }

        std::string formatModuleText(const vm::MachineModule& ModuleValue)
        {
            const auto Text = vm::writeMachineModuleText(ModuleValue);
            EXPECT_TRUE(Text.has_value());
            return Text.has_value() ? *Text : std::string();
        }

        bool hasStackAdjustment(const vm::MachineModule& ModuleValue, std::int64_t Delta)
        {
            const std::uint64_t ExpectedBits = static_cast<std::uint64_t>(Delta);
            for (const vm::MachineFunction& Function : ModuleValue.functions())
            {
                for (const vm::MachineBasicBlock& Block : Function.basicBlocks())
                {
                    for (const vm::MachineInstruction& Instruction : Block.instructions())
                    {
                        if (Instruction.getOpcode() != vm::Opcode::Add64rri)
                        {
                            continue;
                        }
                        const auto& Add = static_cast<const vm::MachineAdd64rriInstruction&>(Instruction);
                        if (Add.getDestination().getRegister() == vm::RegisterId::SP && Add.getLHS().getRegister() == vm::RegisterId::SP && Add.getRHS().bits() == ExpectedBits)
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        std::string buildCallHarness(std::string_view TargetFunction, std::span<const std::int64_t> Arguments)
        {
            const std::uint64_t CallBytes = (1 + Arguments.size()) * 8;
            std::ostringstream Stream;
            Stream << ".module \"call_harness\"\n";
            Stream << ".declare function @" << TargetFunction << "\n";
            Stream << ".section .data\n";
            Stream << ".global @test_result size 8 align 8 external\n";
            Stream << ".endglobal\n";
            Stream << ".section .text\n";
            Stream << ".function @main external\n";
            Stream << "entry:\n";
            Stream << "    add64rri sp, sp, " << CallBytes << "\n";
            for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
            {
                const std::uint64_t OffsetFromSP = (Arguments.size() - Index) * 8;
                Stream << "    mov64ri r0, " << Arguments[Index] << "\n";
                Stream << "    store64mr [sp - " << OffsetFromSP << "], r0\n";
            }
            Stream << "    call @" << TargetFunction << "\n";
            Stream << "    load64rm r0, [sp - " << CallBytes << "]\n";
            Stream << "    store64mr [@test_result], r0\n";
            Stream << "    add64rri sp, sp, -" << CallBytes << "\n";
            Stream << "    ret\n";
            Stream << ".endfunction\n";
            return Stream.str();
        }

        std::int64_t executeCompiledCallTarget(const vm::MachineModule& CompiledModule, std::string_view TargetFunction, std::span<const std::int64_t> Arguments)
        {
            const std::string HarnessText = buildCallHarness(TargetFunction, Arguments);
            auto HarnessModule = vm::readMachineModuleText(HarnessText);
            EXPECT_TRUE(HarnessModule.has_value()) << HarnessText;
            if (!HarnessModule.has_value())
            {
                return 0;
            }

            core::CollectingDiagnosticConsumer DiagnosticsConsumer;
            vm::Interpreter Interpreter;
            Interpreter.getDiagnostics().addConsumer(DiagnosticsConsumer);

            const std::array<const vm::MachineModule*, 2> Modules{HarnessModule->get(), &CompiledModule};
            auto Linked = vm::ModuleLinker::link(Modules, Interpreter.getDiagnostics(), core::NamePool::instance().intern("linked_call"));
            EXPECT_NE(Linked, nullptr);
            if (Linked == nullptr)
            {
                return 0;
            }

            const core::Name MainName = core::NamePool::instance().intern("main");
            const core::Name ResultName = core::NamePool::instance().intern("test_result");
            const std::array ExportNames{MainName, ResultName};

            vm::ExecutableEmitter Emitter(Interpreter.getDiagnostics());
            vm::ExecutableFile Executable = Emitter.emit(*Linked, MainName, ExportNames);
            const auto Handle = Interpreter.loadLibrary(std::move(Executable));
            EXPECT_NE(Handle, vm::Interpreter::InvalidLibraryHandle);

            const void* EntryAddress = Interpreter.resolveSymbolAddress(MainName);
            EXPECT_NE(EntryAddress, nullptr);
            EXPECT_TRUE(Interpreter.execute(EntryAddress));
            EXPECT_TRUE(DiagnosticsConsumer.takeDiagnostics().empty());

            const void* ResultAddress = Interpreter.resolveSymbolAddress(ResultName);
            EXPECT_NE(ResultAddress, nullptr);
            std::int64_t Result = 0;
            if (ResultAddress != nullptr)
            {
                std::memcpy(&Result, ResultAddress, sizeof(Result));
            }
            return Result;
        }
    } // namespace

    // Verifies direct calls, upward-stack call area setup/teardown, and automatic spilling of live values across caller-saved R0-R9 calls.
    TEST(InkVMCallCodegenTest, DirectCallsPreserveLiveValuesAcrossCallerSavedCalls)
    {
        constexpr std::string_view IRText = R"(define i64 @add_mul(i64 %x, i64 %y) {
entry:
  %sum = add i64 %x, %y
  %prod = mul i64 %sum, 3
  ret i64 %prod
}

define i64 @caller(i64 %a, i64 %b) {
entry:
  %saved = add i64 %a, 100
  %first = call i64 @add_mul(i64 %a, i64 %b)
  %second = call i64 @add_mul(i64 %first, i64 %b)
  %total = add i64 %second, %saved
  ret i64 %total
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "direct_call_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find("call @add_mul"), std::string::npos) << Assembly;
        EXPECT_TRUE(hasStackAdjustment(*Compiled, 24)) << Assembly;
        EXPECT_TRUE(hasStackAdjustment(*Compiled, -24)) << Assembly;
        // %saved and/or %b must survive across calls via [bp + 16] because all R0-R9 are caller-saved.
        EXPECT_NE(Assembly.find("[bp + 16]"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 2> Args{4, 5};
        // saved = 104, first = (4 + 5) * 3 = 27, second = (27 + 5) * 3 = 96, total = 200
        EXPECT_EQ(executeCompiledCallTarget(*Compiled, "caller", Args), 200);
    }

    // Verifies zero-byte void calls omit redundant SP adjustments and multi-argument calls preserve exact parameter order.
    TEST(InkVMCallCodegenTest, HandlesVoidZeroArgCallsAndFourArgumentOrdering)
    {
        constexpr std::string_view IRText = R"(define void @noop() {
entry:
  ret void
}

define i64 @weighted(i64 %a, i64 %b, i64 %c, i64 %d) {
entry:
  %t0 = mul i64 %a, 1000
  %t1 = mul i64 %b, 100
  %t2 = mul i64 %c, 10
  %s0 = add i64 %t0, %t1
  %s1 = add i64 %s0, %t2
  %s2 = add i64 %s1, %d
  ret i64 %s2
}

define i64 @driver(i64 %a, i64 %b, i64 %c, i64 %d) {
entry:
  call void @noop()
  %res = call i64 @weighted(i64 %a, i64 %b, i64 %c, i64 %d)
  ret i64 %res
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "arg_order_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find("call @noop"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("call @weighted"), std::string::npos) << Assembly;
        EXPECT_FALSE(hasStackAdjustment(*Compiled, 0)) << Assembly;
        // Return slot (8) + 4 args (32) = 40 bytes.
        EXPECT_TRUE(hasStackAdjustment(*Compiled, 40)) << Assembly;
        EXPECT_TRUE(hasStackAdjustment(*Compiled, -40)) << Assembly;

        const std::array<std::int64_t, 4> Args{1, 2, 3, 4};
        EXPECT_EQ(executeCompiledCallTarget(*Compiled, "driver", Args), 1234);
    }

    // Verifies function symbol materialization (mov64ri dst, @fn) and indirect register calls (call rN).
    TEST(InkVMCallCodegenTest, CompilesAndExecutesIndirectFunctionPointerCall)
    {
        constexpr std::string_view IRText = R"(define i64 @triple(i64 %x) {
entry:
  %res = mul i64 %x, 3
  ret i64 %res
}

define i64 @apply(ptr %fn, i64 %x) {
entry:
  %res = call i64 %fn(i64 %x)
  ret i64 %res
}

define i64 @invoke_indirect(i64 %x) {
entry:
  %res = call i64 @apply(ptr @triple, i64 %x)
  ret i64 %res
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "indirect_call_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find("@triple"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("call r"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 1> Args{14};
        EXPECT_EQ(executeCompiledCallTarget(*Compiled, "invoke_indirect", Args), 42);
    }

    // Verifies calling Ink VM's built-in native runtime stub @identity(i64) -> i64 directly from compiled LLVM IR.
    TEST(InkVMCallCodegenTest, CallsBuiltInNativeRuntimeStubAcrossVMBoundary)
    {
        constexpr std::string_view IRText = R"(declare i64 @identity(i64)

define i64 @round_trip_native(i64 %x) {
entry:
  %a = call i64 @identity(i64 %x)
  %b = add i64 %a, 5
  %c = call i64 @identity(i64 %b)
  %sum = add i64 %a, %c
  ret i64 %sum
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "native_stub_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find(".declare function @identity"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("call @identity"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 1> Args{100};
        // a = 100, b = 105, c = 105 -> sum = 205
        EXPECT_EQ(executeCompiledCallTarget(*Compiled, "round_trip_native", Args), 205);
    }
} // namespace ink::backend::llvm_inkvm
