#include "ink/backend/llvm_inkvm/inkvm_target.h"
#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
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
#include <limits>
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

        // Builds a minimal Ink VM caller module that prepares the ABI call area, calls TargetFunction, and stores the i64 result into @test_result.
        std::string buildHarnessAssembly(std::string_view TargetFunction, std::span<const std::int64_t> Arguments)
        {
            const std::uint64_t CallBytes = (1 + Arguments.size()) * 8;
            std::ostringstream Stream;
            Stream << ".module \"harness\"\n";
            Stream << ".declare function @" << TargetFunction << "\n";
            Stream << ".section .data\n";
            Stream << ".global test_result, 8, 8 external\n";
            Stream << "    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00\n";
            Stream << ".endglobal\n";
            Stream << ".section .text\n";
            Stream << ".function main external\n";
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

        std::int64_t executeCompiledFunction(const vm::MachineModule& CompiledModule, std::string_view TargetFunction, std::span<const std::int64_t> Arguments)
        {
            const std::string HarnessText = buildHarnessAssembly(TargetFunction, Arguments);
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
            auto Linked = vm::ModuleLinker::link(Modules, Interpreter.getDiagnostics(), core::NamePool::instance().intern("linked"));
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

    // Verifies 64-bit integer addition, subtraction, negation via ZERO, multiplication, immediate folding, and large 64-bit constants.
    TEST(InkVMIntegerALUCodegenTest, CompilesAndExecutesAddSubMulAndConstants)
    {
        constexpr std::string_view IRText = R"(define i64 @arith(i64 %a, i64 %b) {
entry:
  %sum = add i64 %a, %b
  %inc = add i64 %sum, 10
  %prod = mul i64 %inc, 3
  %diff = sub i64 %prod, %b
  ret i64 %diff
}

define i64 @negate(i64 %x) {
entry:
  %neg = sub i64 0, %x
  ret i64 %neg
}

define i64 @big_const() {
entry:
  ret i64 1311768467463790320
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "arith_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find("add64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("add64rri"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("mul64rri"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("sub64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("zero"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("mov64ri"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 2> ArithArgs{7, 5};
        // ((7 + 5) + 10) * 3 - 5 = 61
        EXPECT_EQ(executeCompiledFunction(*Compiled, "arith", ArithArgs), 61);

        const std::array<std::int64_t, 1> NegArgs{42};
        EXPECT_EQ(executeCompiledFunction(*Compiled, "negate", NegArgs), -42);

        const std::span<const std::int64_t> EmptyArgs;
        EXPECT_EQ(executeCompiledFunction(*Compiled, "big_const", EmptyArgs), 0x123456789ABCDEF0LL);
    }

    // Verifies signed and unsigned 64-bit division and remainder, including simultaneous div+rem (SDIVREM/UDIVREM expansion).
    TEST(InkVMIntegerALUCodegenTest, CompilesAndExecutesSignedAndUnsignedDivRem)
    {
        constexpr std::string_view IRText = R"(define i64 @sdiv_srem_sum(i64 %a, i64 %b) {
entry:
  %q = sdiv i64 %a, %b
  %r = srem i64 %a, %b
  %res = add i64 %q, %r
  ret i64 %res
}

define i64 @udiv_urem_sum(i64 %a, i64 %b) {
entry:
  %q = udiv i64 %a, %b
  %r = urem i64 %a, %b
  %res = add i64 %q, %r
  ret i64 %res
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "divrem_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find("sdiv64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("srem64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("udiv64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("urem64rrr"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 2> SignedArgs{-100, 7};
        // -100 / 7 = -14, -100 % 7 = -2 -> -16
        EXPECT_EQ(executeCompiledFunction(*Compiled, "sdiv_srem_sum", SignedArgs), -16);

        const std::array<std::int64_t, 2> UnsignedArgs{-16, 10};
        const std::uint64_t UDividend = static_cast<std::uint64_t>(UnsignedArgs[0]);
        const std::int64_t ExpectedUnsigned = static_cast<std::int64_t>((UDividend / 10ULL) + (UDividend % 10ULL));
        EXPECT_EQ(executeCompiledFunction(*Compiled, "udiv_urem_sum", UnsignedArgs), ExpectedUnsigned);
    }

    // Verifies 64-bit bitwise operations (and/or/xor) and logical vs arithmetic shifts in both register and immediate forms.
    TEST(InkVMIntegerALUCodegenTest, CompilesAndExecutesBitwiseAndShifts)
    {
        constexpr std::string_view IRText = R"(define i64 @bitwise_and_shifts(i64 %a, i64 %b, i64 %shift) {
entry:
  %and_r = and i64 %a, %b
  %or_i = or i64 %and_r, 240
  %xor_i = xor i64 %or_i, 15
  %shl_r = shl i64 %xor_i, %shift
  %lshr_i = lshr i64 %shl_r, 2
  ret i64 %lshr_i
}

define i64 @ashr_negative(i64 %x) {
entry:
  %res = ashr i64 %x, 4
  ret i64 %res
}

define i64 @lshr_negative(i64 %x) {
entry:
  %res = lshr i64 %x, 4
  ret i64 %res
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "bitwise_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        EXPECT_NE(Assembly.find("and64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("or64rri"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("xor64rri"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("shl64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("lshr64rri"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("ashr64rri"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 3> BitArgs{0xFF0F, 0x0FFF, 3};
        const std::uint64_t ExpectedBits = ((((0xFF0FULL & 0x0FFFULL) | 240ULL) ^ 15ULL) << 3ULL) >> 2ULL;
        EXPECT_EQ(executeCompiledFunction(*Compiled, "bitwise_and_shifts", BitArgs), static_cast<std::int64_t>(ExpectedBits));

        const std::array<std::int64_t, 1> NegValue{-64};
        EXPECT_EQ(executeCompiledFunction(*Compiled, "ashr_negative", NegValue), -4);
        EXPECT_EQ(executeCompiledFunction(*Compiled, "lshr_negative", NegValue), static_cast<std::int64_t>(static_cast<std::uint64_t>(-64) >> 4ULL));
    }

    // Verifies LLVM automatic type promotion of i32 arithmetic and sign extension into native 64-bit InkVM instructions.
    TEST(InkVMIntegerALUCodegenTest, PromotesNarrowIntegerArithmeticAndSignExtension)
    {
        constexpr std::string_view IRText = R"(define i64 @narrow_wrap(i64 %a, i64 %b) {
entry:
  %a32 = trunc i64 %a to i32
  %b32 = trunc i64 %b to i32
  %sum32 = add i32 %a32, %b32
  %ext = sext i32 %sum32 to i64
  ret i64 %ext
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "promote_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::array<std::int64_t, 2> Args{0x7FFFFFFF, 1};
        // (int32_t)(0x7FFFFFFF + 1) == INT32_MIN == -2147483648
        EXPECT_EQ(executeCompiledFunction(*Compiled, "narrow_wrap", Args), -2147483648LL);
    }

    // Verifies register spilling, upward stack prologue allocation, and BP-relative frame index elimination when live values exceed R0-R9.
    TEST(InkVMIntegerALUCodegenTest, SpillsRegistersToUpwardStackFrameWhenExceedingTenGPRs)
    {
        // Volatile chain barriers force 12 distinct non-rematerializable values to stay simultaneously live across R0-R9.
        constexpr std::string_view IRText = R"(define i64 @high_pressure(i64 %a, i64 %b) {
entry:
  %slot = alloca i64, align 8
  %x0 = add i64 %a, 1
  store volatile i64 %x0, ptr %slot
  %v0 = load volatile i64, ptr %slot
  %x1 = add i64 %b, 2
  store volatile i64 %x1, ptr %slot
  %v1 = load volatile i64, ptr %slot
  %x2 = add i64 %a, 3
  store volatile i64 %x2, ptr %slot
  %v2 = load volatile i64, ptr %slot
  %x3 = add i64 %b, 4
  store volatile i64 %x3, ptr %slot
  %v3 = load volatile i64, ptr %slot
  %x4 = add i64 %a, 5
  store volatile i64 %x4, ptr %slot
  %v4 = load volatile i64, ptr %slot
  %x5 = add i64 %b, 6
  store volatile i64 %x5, ptr %slot
  %v5 = load volatile i64, ptr %slot
  %x6 = add i64 %a, 7
  store volatile i64 %x6, ptr %slot
  %v6 = load volatile i64, ptr %slot
  %x7 = add i64 %b, 8
  store volatile i64 %x7, ptr %slot
  %v7 = load volatile i64, ptr %slot
  %x8 = add i64 %a, 9
  store volatile i64 %x8, ptr %slot
  %v8 = load volatile i64, ptr %slot
  %x9 = add i64 %b, 10
  store volatile i64 %x9, ptr %slot
  %v9 = load volatile i64, ptr %slot
  %x10 = add i64 %a, 11
  store volatile i64 %x10, ptr %slot
  %v10 = load volatile i64, ptr %slot
  %x11 = add i64 %b, 12
  store volatile i64 %x11, ptr %slot
  %v11 = load volatile i64, ptr %slot
  store volatile i64 %v0, ptr %slot
  store volatile i64 %v1, ptr %slot
  store volatile i64 %v2, ptr %slot
  store volatile i64 %v3, ptr %slot
  store volatile i64 %v4, ptr %slot
  store volatile i64 %v5, ptr %slot
  store volatile i64 %v6, ptr %slot
  store volatile i64 %v7, ptr %slot
  store volatile i64 %v8, ptr %slot
  store volatile i64 %v9, ptr %slot
  store volatile i64 %v10, ptr %slot
  store volatile i64 %v11, ptr %slot
  %s0 = add i64 %v0, %v1
  %s1 = add i64 %s0, %v2
  %s2 = add i64 %s1, %v3
  %s3 = add i64 %s2, %v4
  %s4 = add i64 %s3, %v5
  %s5 = add i64 %s4, %v6
  %s6 = add i64 %s5, %v7
  %s7 = add i64 %s6, %v8
  %s8 = add i64 %s7, %v9
  %s9 = add i64 %s8, %v10
  %s10 = add i64 %s9, %v11
  ret i64 %s10
}
)";

        const auto Compiled = compileIRToMachineModule(IRText, "spill_mod");
        ASSERT_NE(Compiled, nullptr);
        const std::string Assembly = formatModuleText(*Compiled);
        // Verify local alloca and spill slots were allocated on the upward stack above [bp + 16].
        EXPECT_NE(Assembly.find("add64rri sp, sp, "), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("[bp + 16]"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("[bp + 24]"), std::string::npos) << Assembly;

        const std::int64_t A = 100;
        const std::int64_t B = 200;
        // 6*(A) + 6*(B) + (1 + 2 + ... + 12) = 600 + 1200 + 78 = 1878
        const std::int64_t Expected = 6 * A + 6 * B + 78;

        const std::array<std::int64_t, 2> Args{A, B};
        EXPECT_EQ(executeCompiledFunction(*Compiled, "high_pressure", Args), Expected);
    }
} // namespace ink::backend::llvm_inkvm
