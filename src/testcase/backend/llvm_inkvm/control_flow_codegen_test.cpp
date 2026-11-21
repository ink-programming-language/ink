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
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace ink::backend::llvm_inkvm
{
    namespace
    {
        std::string compileIRToAssembly(std::string_view IRText, std::string_view ModuleName)
        {
            llvm::LLVMContext Context;
            llvm::SMDiagnostic ParseError;
            auto IRModule = llvm::parseAssemblyString(llvm::StringRef(IRText.data(), IRText.size()), ParseError, Context);
            EXPECT_NE(IRModule, nullptr);
            if (IRModule == nullptr)
            {
                return {};
            }
            IRModule->setModuleIdentifier(llvm::StringRef(ModuleName.data(), ModuleName.size()));

            std::string Assembly;
            std::string ErrorMessage;
            EXPECT_TRUE(emitInkVMAssembly(*IRModule, Assembly, ErrorMessage)) << ErrorMessage;
            return Assembly;
        }

        std::string buildHarness(std::string_view TargetFunction, std::span<const std::int64_t> Arguments)
        {
            const std::uint64_t CallBytes = (1 + Arguments.size()) * 8;
            std::ostringstream Stream;
            Stream << ".module \"cf_harness\"\n";
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

        std::int64_t executeCompiled(std::string_view Assembly, std::string_view TargetFunction, std::span<const std::int64_t> Arguments)
        {
            auto CompiledModule = vm::readMachineModuleText(Assembly);
            EXPECT_TRUE(CompiledModule.has_value()) << Assembly;
            if (!CompiledModule.has_value())
            {
                return 0;
            }

            const std::string HarnessText = buildHarness(TargetFunction, Arguments);
            auto HarnessModule = vm::readMachineModuleText(HarnessText);
            EXPECT_TRUE(HarnessModule.has_value()) << HarnessText;
            if (!HarnessModule.has_value())
            {
                return 0;
            }

            core::CollectingDiagnosticConsumer DiagnosticsConsumer;
            vm::Interpreter Interpreter;
            Interpreter.getDiagnostics().addConsumer(DiagnosticsConsumer);

            const std::array<const vm::MachineModule*, 2> Modules{HarnessModule->get(), CompiledModule->get()};
            auto Linked = vm::ModuleLinker::link(Modules, Interpreter.getDiagnostics(), core::NamePool::instance().intern("linked_cf"));
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

    // Verifies all 10 signed/unsigned icmp predicates lower to InkVM cmp*64rrr/rri instructions with 0/1 boolean results.
    TEST(InkVMControlFlowCodegenTest, LowersAllTenIntegerComparisonsToCmpInstructions)
    {
        constexpr std::string_view IRText = R"(define i64 @cmp_mask(i64 %a, i64 %b) {
entry:
  %c0 = icmp eq i64 %a, %b
  %c1 = icmp ne i64 %a, %b
  %c2 = icmp slt i64 %a, %b
  %c3 = icmp sle i64 %a, %b
  %c4 = icmp sgt i64 %a, %b
  %c5 = icmp sge i64 %a, %b
  %c6 = icmp ult i64 %a, %b
  %c7 = icmp ule i64 %a, %b
  %c8 = icmp ugt i64 %a, %b
  %c9 = icmp uge i64 %a, %b
  %c10 = icmp slt i64 %a, 42
  %z0 = zext i1 %c0 to i64
  %z1 = zext i1 %c1 to i64
  %z2 = zext i1 %c2 to i64
  %z3 = zext i1 %c3 to i64
  %z4 = zext i1 %c4 to i64
  %z5 = zext i1 %c5 to i64
  %z6 = zext i1 %c6 to i64
  %z7 = zext i1 %c7 to i64
  %z8 = zext i1 %c8 to i64
  %z9 = zext i1 %c9 to i64
  %z10 = zext i1 %c10 to i64
  %s1 = shl i64 %z1, 1
  %s2 = shl i64 %z2, 2
  %s3 = shl i64 %z3, 3
  %s4 = shl i64 %z4, 4
  %s5 = shl i64 %z5, 5
  %s6 = shl i64 %z6, 6
  %s7 = shl i64 %z7, 7
  %s8 = shl i64 %z8, 8
  %s9 = shl i64 %z9, 9
  %s10 = shl i64 %z10, 10
  %r0 = or i64 %z0, %s1
  %r1 = or i64 %r0, %s2
  %r2 = or i64 %r1, %s3
  %r3 = or i64 %r2, %s4
  %r4 = or i64 %r3, %s5
  %r5 = or i64 %r4, %s6
  %r6 = or i64 %r5, %s7
  %r7 = or i64 %r6, %s8
  %r8 = or i64 %r7, %s9
  %r9 = or i64 %r8, %s10
  ret i64 %r9
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "cmp_mod");
        EXPECT_NE(Assembly.find("cmpeq64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("cmplt64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("cmpltu64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("cmplt64rri"), std::string::npos) << Assembly;

        // (-5, 3): ne(1), slt(2), sle(3), ugt(8), uge(9), a<42(10) -> bits 1,2,3,8,9,10 = 0x70E = 1806.
        const std::array<std::int64_t, 2> NegPos{-5, 3};
        EXPECT_EQ(executeCompiled(Assembly, "cmp_mask", NegPos), (1 << 1) | (1 << 2) | (1 << 3) | (1 << 8) | (1 << 9) | (1 << 10));

        // (7, 7): eq(0), sle(3), sge(5), ule(7), uge(9), a<42(10)
        const std::array<std::int64_t, 2> EqualPair{7, 7};
        EXPECT_EQ(executeCompiled(Assembly, "cmp_mask", EqualPair), (1 << 0) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 10));

        // (100, 2): ne(1), sgt(4), sge(5), ugt(8), uge(9), !(100<42)
        const std::array<std::int64_t, 2> GreaterPair{100, 2};
        EXPECT_EQ(executeCompiled(Assembly, "cmp_mask", GreaterPair), (1 << 1) | (1 << 4) | (1 << 5) | (1 << 8) | (1 << 9));
    }

    // Verifies select and compound boolean select lower via SELECT_CC_I64 custom inserter into a diamond CFG and PHI.
    TEST(InkVMControlFlowCodegenTest, LowersSelectViaCustomInserterDiamondCFG)
    {
        constexpr std::string_view IRText = R"(define i64 @clamp_and_choose(i64 %x, i64 %lo, i64 %hi) {
entry:
  %below = icmp slt i64 %x, %lo
  %t0 = select i1 %below, i64 %lo, i64 %x
  %above = icmp sgt i64 %t0, %hi
  %clamped = select i1 %above, i64 %hi, i64 %t0
  %is_neg = icmp slt i64 %clamped, 0
  %neg = sub i64 0, %clamped
  %abs_val = select i1 %is_neg, i64 %neg, i64 %clamped
  %in_range = and i1 %below, %above
  %res = select i1 %in_range, i64 -999, i64 %abs_val
  ret i64 %res
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "select_mod");
        EXPECT_EQ(Assembly.find("SELECT_CC_I64"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 3> BelowArgs{-50, -10, 20};
        EXPECT_EQ(executeCompiled(Assembly, "clamp_and_choose", BelowArgs), 10);

        const std::array<std::int64_t, 3> MidArgs{-7, -10, 20};
        EXPECT_EQ(executeCompiled(Assembly, "clamp_and_choose", MidArgs), 7);

        const std::array<std::int64_t, 3> AboveArgs{99, -10, 20};
        EXPECT_EQ(executeCompiled(Assembly, "clamp_and_choose", AboveArgs), 20);
    }

    // Verifies loops with PHI nodes emit direct compare-and-branch instructions and execute accurately in the VM.
    TEST(InkVMControlFlowCodegenTest, ExecutesLoopsWithPhiNodesAndCompareBranches)
    {
        constexpr std::string_view IRText = R"(define i64 @sum_to_n(i64 %n) {
entry:
  %zero_check = icmp sle i64 %n, 0
  br i1 %zero_check, label %done, label %loop

loop:
  %i = phi i64 [ 1, %entry ], [ %next_i, %loop ]
  %acc = phi i64 [ 0, %entry ], [ %next_acc, %loop ]
  %next_acc = add i64 %acc, %i
  %next_i = add i64 %i, 1
  %cont = icmp sle i64 %next_i, %n
  br i1 %cont, label %loop, label %done

done:
  %result = phi i64 [ 0, %entry ], [ %next_acc, %loop ]
  ret i64 %result
}

define i64 @gcd(i64 %a, i64 %b) {
entry:
  br label %header

header:
  %x = phi i64 [ %a, %entry ], [ %y, %body ]
  %y = phi i64 [ %b, %entry ], [ %rem, %body ]
  %done_cond = icmp eq i64 %y, 0
  br i1 %done_cond, label %exit, label %body

body:
  %rem = srem i64 %x, %y
  br label %header

exit:
  ret i64 %x
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "loops_mod");
        EXPECT_NE(Assembly.find("64rrb "), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("64rib "), std::string::npos) << Assembly;

        const std::array<std::int64_t, 1> Sum10{10};
        EXPECT_EQ(executeCompiled(Assembly, "sum_to_n", Sum10), 55);

        const std::array<std::int64_t, 1> SumZero{0};
        EXPECT_EQ(executeCompiled(Assembly, "sum_to_n", SumZero), 0);

        const std::array<std::int64_t, 2> GcdArgs{1071, 462};
        EXPECT_EQ(executeCompiled(Assembly, "gcd", GcdArgs), 21);

        const std::array<std::int64_t, 2> GcdCoprime{35, 64};
        EXPECT_EQ(executeCompiled(Assembly, "gcd", GcdCoprime), 1);
    }

    // Verifies multi-case LLVM switch lowers via expanded BR_JT into InkVM compare-and-branch trees.
    TEST(InkVMControlFlowCodegenTest, ExpandsSwitchToCompareBranchTree)
    {
        constexpr std::string_view IRText = R"(define i64 @dispatch_case(i64 %k) {
entry:
  switch i64 %k, label %default [
    i64 0, label %c0
    i64 1, label %c1
    i64 2, label %c2
    i64 3, label %c3
    i64 4, label %c4
    i64 5, label %c5
  ]

c0:
  ret i64 100
c1:
  ret i64 202
c2:
  ret i64 305
c3:
  ret i64 409
c4:
  ret i64 514
c5:
  ret i64 620
default:
  ret i64 -1
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "switch_mod");
        EXPECT_EQ(Assembly.find("jmpr"), std::string::npos) << Assembly;

        const std::array<std::pair<std::int64_t, std::int64_t>, 8> Cases{
            std::pair{0, 100},
            std::pair{1, 202},
            std::pair{2, 305},
            std::pair{3, 409},
            std::pair{4, 514},
            std::pair{5, 620},
            std::pair{6, -1},
            std::pair{-3, -1},
        };
        for (const auto& [Key, Expected] : Cases)
        {
            const std::array<std::int64_t, 1> Args{Key};
            EXPECT_EQ(executeCompiled(Assembly, "dispatch_case", Args), Expected) << "key=" << Key;
        }
    }

    // Verifies end-to-end compilation and execution of recursive Fibonacci combining branches, calls, and stack spilling.
    TEST(InkVMControlFlowCodegenTest, CompilesAndExecutesRecursiveFibonacciEndToEnd)
    {
        constexpr std::string_view IRText = R"(define i64 @fib(i64 %n) {
entry:
  %base = icmp sle i64 %n, 1
  br i1 %base, label %return_n, label %recurse

return_n:
  ret i64 %n

recurse:
  %n1 = sub i64 %n, 1
  %f1 = call i64 @fib(i64 %n1)
  %n2 = sub i64 %n, 2
  %f2 = call i64 @fib(i64 %n2)
  %sum = add i64 %f1, %f2
  ret i64 %sum
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "fib_mod");
        EXPECT_NE(Assembly.find("call @fib"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("ret"), std::string::npos) << Assembly;

        const std::array<std::pair<std::int64_t, std::int64_t>, 6> Cases{
            std::pair{0, 0},
            std::pair{1, 1},
            std::pair{2, 1},
            std::pair{7, 13},
            std::pair{10, 55},
            std::pair{15, 610},
        };
        for (const auto& [Input, Expected] : Cases)
        {
            const std::array<std::int64_t, 1> Args{Input};
            EXPECT_EQ(executeCompiled(Assembly, "fib", Args), Expected) << "fib(" << Input << ")";
        }
    }
} // namespace ink::backend::llvm_inkvm
