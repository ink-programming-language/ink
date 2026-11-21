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

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
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

        std::string buildHarness(std::string_view TargetFunction, std::span<const std::uint64_t> RawWordArguments)
        {
            const std::uint64_t CallBytes = (1 + RawWordArguments.size()) * 8;
            std::ostringstream Stream;
            Stream << ".module \"fp64_harness\"\n";
            Stream << ".declare function @" << TargetFunction << "\n";
            Stream << ".section .data\n";
            Stream << ".global test_result, 8, 8 external\n";
            Stream << "    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00\n";
            Stream << ".endglobal\n";
            Stream << ".section .text\n";
            Stream << ".function main external\n";
            Stream << "entry:\n";
            Stream << "    add64rri sp, sp, " << CallBytes << "\n";
            for (std::size_t Index = 0; Index < RawWordArguments.size(); ++Index)
            {
                const std::uint64_t OffsetFromSP = (RawWordArguments.size() - Index) * 8;
                Stream << "    mov64ri r0, 0x" << std::hex << std::setfill('0') << std::setw(16) << RawWordArguments[Index] << std::dec << "\n";
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

        std::uint64_t executeCompiledBits(std::string_view Assembly, std::string_view TargetFunction, std::span<const std::uint64_t> RawWordArguments)
        {
            auto CompiledModule = vm::readMachineModuleText(Assembly);
            EXPECT_TRUE(CompiledModule.has_value()) << Assembly;
            if (!CompiledModule.has_value())
            {
                return 0;
            }

            const std::string HarnessText = buildHarness(TargetFunction, RawWordArguments);
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
            auto Linked = vm::ModuleLinker::link(Modules, Interpreter.getDiagnostics(), core::NamePool::instance().intern("linked_fp64"));
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
            std::uint64_t ResultBits = 0;
            if (ResultAddress != nullptr)
            {
                std::memcpy(&ResultBits, ResultAddress, sizeof(ResultBits));
            }
            return ResultBits;
        }

        double executeCompiledDouble(std::string_view Assembly, std::string_view TargetFunction, std::span<const double> DoubleArguments)
        {
            std::array<std::uint64_t, 8> WordArgs{};
            EXPECT_LE(DoubleArguments.size(), WordArgs.size());
            for (std::size_t Index = 0; Index < DoubleArguments.size(); ++Index)
            {
                WordArgs[Index] = std::bit_cast<std::uint64_t>(DoubleArguments[Index]);
            }
            const std::uint64_t Bits = executeCompiledBits(Assembly, TargetFunction, std::span<const std::uint64_t>(WordArgs.data(), DoubleArguments.size()));
            return std::bit_cast<double>(Bits);
        }

        std::string formatIRDoubleHex(double Value)
        {
            std::ostringstream Stream;
            Stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << std::bit_cast<std::uint64_t>(Value);
            return Stream.str();
        }
    } // namespace

    // Verifies FP64 arithmetic (fadd/fsub/fmul/fdiv), fsqrt, fneg, expanded fabs, FP constants, and i64<->f64 bitcast.
    TEST(InkVMFP64CodegenTest, CompilesAndExecutesFP64ArithmeticSqrtNegAndConstants)
    {
        constexpr std::string_view IRText = R"(declare double @llvm.sqrt.f64(double)
declare double @llvm.fabs.f64(double)

define double @fp_compute(double %a, double %b) {
entry:
  %diff = fsub double %a, %b
  %sum = fadd double %a, %b
  %d2 = fmul double %diff, %diff
  %s2 = fmul double %sum, %sum
  %rad = fadd double %d2, %s2
  %root = call double @llvm.sqrt.f64(double %rad)
  %quot = fdiv double %root, %b
  %abs_c = call double @llvm.fabs.f64(double -3.25)
  %added = fadd double %quot, %abs_c
  %out = fneg double %added
  ret double %out
}

define i64 @double_to_bits(double %x) {
entry:
  %bits = bitcast double %x to i64
  ret i64 %bits
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "fp_arith_mod");
        EXPECT_NE(Assembly.find("fadd64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fsub64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fmul64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fdiv64rrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fsqrt64rr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fneg64rr"), std::string::npos) << Assembly;

        // a=5, b=2 -> diff=3, sum=7 -> rad=58 -> -(sqrt(58)/2 + 3.25)
        const std::array<double, 2> Args{5.0, 2.0};
        const double Expected = -(std::sqrt(58.0) / 2.0 + 3.25);
        EXPECT_DOUBLE_EQ(executeCompiledDouble(Assembly, "fp_compute", Args), Expected);

        // Verify -0.0 bitcast preserves sign bit 0x8000000000000000.
        const std::array<std::uint64_t, 1> NegZero{std::uint64_t{1} << 63};
        EXPECT_EQ(executeCompiledBits(Assembly, "double_to_bits", NegZero), std::uint64_t{1} << 63);
    }

    // Verifies all 14 IEEE-754 ordered and unordered FP64 comparisons including NaN and signed-zero semantics.
    TEST(InkVMFP64CodegenTest, LowersAllFourteenIEEE754FP64ComparisonsIncludingNaN)
    {
        constexpr std::string_view IRText = R"(define i64 @fcmp_mask(double %a, double %b) {
entry:
  %c0 = fcmp oeq double %a, %b
  %c1 = fcmp one double %a, %b
  %c2 = fcmp olt double %a, %b
  %c3 = fcmp ole double %a, %b
  %c4 = fcmp ogt double %a, %b
  %c5 = fcmp oge double %a, %b
  %c6 = fcmp ord double %a, %b
  %c7 = fcmp uno double %a, %b
  %c8 = fcmp ueq double %a, %b
  %c9 = fcmp une double %a, %b
  %c10 = fcmp ult double %a, %b
  %c11 = fcmp ule double %a, %b
  %c12 = fcmp ugt double %a, %b
  %c13 = fcmp uge double %a, %b
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
  %z11 = zext i1 %c11 to i64
  %z12 = zext i1 %c12 to i64
  %z13 = zext i1 %c13 to i64
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
  %s11 = shl i64 %z11, 11
  %s12 = shl i64 %z12, 12
  %s13 = shl i64 %z13, 13
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
  %r10 = or i64 %r9, %s11
  %r11 = or i64 %r10, %s12
  %r12 = or i64 %r11, %s13
  ret i64 %r12
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "fcmp_mod");
        EXPECT_NE(Assembly.find("fcmp64oeqrrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fcmp64oltrrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fcmp64ordrrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fcmp64unorrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fcmp64unerrr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fcmp64ugerrr"), std::string::npos) << Assembly;

        auto RunMask = [&](double A, double B) -> std::uint64_t
        {
            const std::array<std::uint64_t, 2> Args{std::bit_cast<std::uint64_t>(A), std::bit_cast<std::uint64_t>(B)};
            return executeCompiledBits(Assembly, "fcmp_mask", Args);
        };

        // (-2.5, 4.0): one(1), olt(2), ole(3), ord(6), une(9), ult(10), ule(11).
        EXPECT_EQ(RunMask(-2.5, 4.0), (1ULL << 1) | (1ULL << 2) | (1ULL << 3) | (1ULL << 6) | (1ULL << 9) | (1ULL << 10) | (1ULL << 11));

        // (+0.0, -0.0): oeq(0), ole(3), oge(5), ord(6), ueq(8), ule(11), uge(13).
        EXPECT_EQ(RunMask(0.0, -0.0), (1ULL << 0) | (1ULL << 3) | (1ULL << 5) | (1ULL << 6) | (1ULL << 8) | (1ULL << 11) | (1ULL << 13));

        // (NaN, 1.0): only unordered predicates hold -> uno(7), ueq(8), une(9), ult(10), ule(11), ugt(12), uge(13).
        const double QuietNaN = std::numeric_limits<double>::quiet_NaN();
        EXPECT_EQ(RunMask(QuietNaN, 1.0), (1ULL << 7) | (1ULL << 8) | (1ULL << 9) | (1ULL << 10) | (1ULL << 11) | (1ULL << 12) | (1ULL << 13));
    }

    // Verifies FP64 conditional branches, FP64/integer select lowering, and all four integer<->FP64 conversions.
    TEST(InkVMFP64CodegenTest, LowersFP64BranchesSelectsAndIntegerConversions)
    {
        constexpr std::string_view IRText = R"(define double @classify_and_convert(i64 %signed_val, i64 %unsigned_val) {
entry:
  %fs = sitofp i64 %signed_val to double
  %fu = uitofp i64 %unsigned_val to double
  %is_less = fcmp olt double %fs, %fu
  br i1 %is_less, label %take_min, label %take_max

take_min:
  %picked_min = select i1 %is_less, double %fs, double %fu
  %neg_flag = icmp slt i64 %signed_val, 0
  %adj = select i1 %neg_flag, double 10.5, double 2.5
  %r1 = fadd double %picked_min, %adj
  ret double %r1

take_max:
  %i_s = fptosi double %fs to i64
  %i_u = fptoui double %fu to i64
  %gt = fcmp ogt double %fs, 0.0
  %chosen_i = select i1 %gt, i64 %i_s, i64 %i_u
  %out = sitofp i64 %chosen_i to double
  ret double %out
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "fp_conv_cf_mod");
        EXPECT_NE(Assembly.find("sitof64rr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("uitof64rr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("f64tosi64rr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("f64toui64rr"), std::string::npos) << Assembly;
        EXPECT_EQ(Assembly.find("SELECT_CC_F64"), std::string::npos) << Assembly;

        // (-4, 3): fs=-4.0 < fu=3.0 -> take_min -> -4.0 + 10.5 = 6.5.
        const std::array<std::uint64_t, 2> Case1{static_cast<std::uint64_t>(-4LL), 3ULL};
        EXPECT_DOUBLE_EQ(std::bit_cast<double>(executeCompiledBits(Assembly, "classify_and_convert", Case1)), 6.5);

        // (12, 3): fs=12.0 >= fu=3.0 -> take_max -> fs > 0 -> i_s = 12 -> 12.0.
        const std::array<std::uint64_t, 2> Case2{12ULL, 3ULL};
        EXPECT_DOUBLE_EQ(std::bit_cast<double>(executeCompiledBits(Assembly, "classify_and_convert", Case2)), 12.0);

        // (-10, 0x8000000000000000ULL): uitofp treats 2^63 as positive 9223372036854775808.0 > -10.0 -> -10.0 + 10.5 = 0.5.
        const std::array<std::uint64_t, 2> Case3{static_cast<std::uint64_t>(-10LL), std::uint64_t{1} << 63};
        EXPECT_DOUBLE_EQ(std::bit_cast<double>(executeCompiledBits(Assembly, "classify_and_convert", Case3)), 0.5);
    }

    // Verifies FP64 stack-slot ABI arguments/returns and live FPR spilling across caller-saved calls.
    TEST(InkVMFP64CodegenTest, PassesAndReturnsFP64AcrossCallsAndSpillsLiveValues)
    {
        constexpr std::string_view IRText = R"(define double @dot2(double %ax, double %ay, double %bx, double %by) {
entry:
  %px = fmul double %ax, %bx
  %py = fmul double %ay, %by
  %s = fadd double %px, %py
  ret double %s
}

define double @combine_dots(double %x, double %y, double %z) {
entry:
  %d1 = call double @dot2(double %x, double %y, double %y, double %z)
  %d2 = call double @dot2(double %x, double %z, double %z, double %x)
  %s0 = fadd double %d1, %d2
  %s1 = fadd double %s0, %x
  %s2 = fadd double %s1, %y
  %s3 = fadd double %s2, %z
  ret double %s3
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "fp_call_mod");
        EXPECT_NE(Assembly.find("call @dot2"), std::string::npos) << Assembly;
        // Caller must allocate local spill slots on the upward stack to keep x, y, z, d1 alive across calls.
        EXPECT_NE(Assembly.find("add64rri sp, sp, "), std::string::npos) << Assembly;

        // x=1.5, y=2.0, z=4.0:
        // d1 = 1.5*2.0 + 2.0*4.0 = 11.0
        // d2 = 1.5*4.0 + 4.0*1.5 = 12.0
        // total = 11.0 + 12.0 + 1.5 + 2.0 + 4.0 = 30.5
        const std::array<double, 3> Args{1.5, 2.0, 4.0};
        EXPECT_DOUBLE_EQ(executeCompiledDouble(Assembly, "combine_dots", Args), 30.5);
    }

    // Compiles and executes the 5-body gravitational N-body simulation from LLVM IR and verifies energies against C++ reference.
    TEST(InkVMFP64CodegenTest, CompilesAndExecutesFiveBodyNBodySimulationAgainstReference)
    {
        constexpr double Pi = 3.141592653589793;
        constexpr double SolarMass = 4.0 * Pi * Pi;
        constexpr double DaysPerYear = 365.24;
        constexpr double TimeStep = 0.01;

        struct Body
        {
                double X;
                double Y;
                double Z;
                double VX;
                double VY;
                double VZ;
                double Mass;
        };

        std::array<Body, 5> Bodies{{
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, SolarMass},
            {4.84143144246472090, -1.16032004402742839, -0.103622044471123109, 0.00166007664274403694 * DaysPerYear, 0.00769901118419740425 * DaysPerYear, -0.0000690460016972063023 * DaysPerYear, 0.000954791938424326609 * SolarMass},
            {8.34336671824457987, 4.12479856412430479, -0.403523417114321381, -0.00276742510726862411 * DaysPerYear, 0.00499852801234917238 * DaysPerYear, 0.0000230417297573763929 * DaysPerYear, 0.000285885980666130812 * SolarMass},
            {12.8943695621391310, -15.1111514016986312, -0.223307578892655734, 0.00296460137564761618 * DaysPerYear, 0.00237847173959480950 * DaysPerYear, -0.0000296589568540237556 * DaysPerYear, 0.0000436624404335156298 * SolarMass},
            {15.3796971148509165, -25.9193146099879641, 0.179258772950371181, 0.00268067772490389322 * DaysPerYear, 0.00162824170038242295 * DaysPerYear, -0.0000951592254519715870 * DaysPerYear, 0.0000515138902046611451 * SolarMass},
        }};

        // Build LLVM IR module with exact IEEE-754 bit patterns for initial conditions.
        std::ostringstream IR;
        IR << "%Body = type { double, double, double, double, double, double, double }\n";
        IR << "declare double @llvm.sqrt.f64(double)\n";
        IR << "@bodies = global [5 x %Body] [\n";
        for (std::size_t Index = 0; Index < Bodies.size(); ++Index)
        {
            const Body& B = Bodies[Index];
            IR << "  %Body { double " << formatIRDoubleHex(B.X) << ", double " << formatIRDoubleHex(B.Y) << ", double " << formatIRDoubleHex(B.Z) << ", double " << formatIRDoubleHex(B.VX) << ", double " << formatIRDoubleHex(B.VY) << ", double " << formatIRDoubleHex(B.VZ) << ", double " << formatIRDoubleHex(B.Mass) << " }";
            IR << (Index + 1 < Bodies.size() ? ",\n" : "\n");
        }
        IR << "], align 8\n\n";
        IR << "@solar_mass = constant double " << formatIRDoubleHex(SolarMass) << ", align 8\n";
        IR << "@dt = constant double " << formatIRDoubleHex(TimeStep) << ", align 8\n\n";
        IR << R"(define void @offset_momentum() {
entry:
  br label %loop

loop:
  %i = phi i64 [ 0, %entry ], [ %next_i, %loop ]
  %px = phi double [ 0.0, %entry ], [ %px_next, %loop ]
  %py = phi double [ 0.0, %entry ], [ %py_next, %loop ]
  %pz = phi double [ 0.0, %entry ], [ %pz_next, %loop ]
  %pvx = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 3
  %pvy = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 4
  %pvz = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 5
  %pm  = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 6
  %vx = load double, ptr %pvx, align 8
  %vy = load double, ptr %pvy, align 8
  %vz = load double, ptr %pvz, align 8
  %m  = load double, ptr %pm, align 8
  %mx = fmul double %vx, %m
  %my = fmul double %vy, %m
  %mz = fmul double %vz, %m
  %px_next = fadd double %px, %mx
  %py_next = fadd double %py, %my
  %pz_next = fadd double %pz, %mz
  %next_i = add i64 %i, 1
  %cont = icmp ult i64 %next_i, 5
  br i1 %cont, label %loop, label %finish

finish:
  %sm = load double, ptr @solar_mass, align 8
  %npx = fneg double %px_next
  %npy = fneg double %py_next
  %npz = fneg double %pz_next
  %svx = fdiv double %npx, %sm
  %svy = fdiv double %npy, %sm
  %svz = fdiv double %npz, %sm
  %s0vx = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 0, i32 3
  %s0vy = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 0, i32 4
  %s0vz = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 0, i32 5
  store double %svx, ptr %s0vx, align 8
  store double %svy, ptr %s0vy, align 8
  store double %svz, ptr %s0vz, align 8
  ret void
}

define double @energy() {
entry:
  br label %outer

outer:
  %i = phi i64 [ 0, %entry ], [ %next_i, %outer_latch ]
  %e = phi double [ 0.0, %entry ], [ %e_after_inner, %outer_latch ]
  %pxi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 0
  %pyi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 1
  %pzi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 2
  %pvxi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 3
  %pvyi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 4
  %pvzi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 5
  %pmi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 6
  %xi = load double, ptr %pxi, align 8
  %yi = load double, ptr %pyi, align 8
  %zi = load double, ptr %pzi, align 8
  %vxi = load double, ptr %pvxi, align 8
  %vyi = load double, ptr %pvyi, align 8
  %vzi = load double, ptr %pvzi, align 8
  %mi = load double, ptr %pmi, align 8
  %vx2 = fmul double %vxi, %vxi
  %vy2 = fmul double %vyi, %vyi
  %vz2 = fmul double %vzi, %vzi
  %v2a = fadd double %vx2, %vy2
  %v2 = fadd double %v2a, %vz2
  %hm = fmul double 0.5, %mi
  %ke = fmul double %hm, %v2
  %e_kin = fadd double %e, %ke
  %j0 = add i64 %i, 1
  %has_inner = icmp ult i64 %j0, 5
  br i1 %has_inner, label %inner, label %outer_latch

inner:
  %j = phi i64 [ %j0, %outer ], [ %next_j, %inner ]
  %e_in = phi double [ %e_kin, %outer ], [ %e_sub, %inner ]
  %pxj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 0
  %pyj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 1
  %pzj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 2
  %pmj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 6
  %xj = load double, ptr %pxj, align 8
  %yj = load double, ptr %pyj, align 8
  %zj = load double, ptr %pzj, align 8
  %mj = load double, ptr %pmj, align 8
  %dx = fsub double %xi, %xj
  %dy = fsub double %yi, %yj
  %dz = fsub double %zi, %zj
  %dx2 = fmul double %dx, %dx
  %dy2 = fmul double %dy, %dy
  %dz2 = fmul double %dz, %dz
  %d2a = fadd double %dx2, %dy2
  %d2 = fadd double %d2a, %dz2
  %dist = call double @llvm.sqrt.f64(double %d2)
  %mm = fmul double %mi, %mj
  %pot = fdiv double %mm, %dist
  %e_sub = fsub double %e_in, %pot
  %next_j = add i64 %j, 1
  %cont_j = icmp ult i64 %next_j, 5
  br i1 %cont_j, label %inner, label %outer_latch

outer_latch:
  %e_after_inner = phi double [ %e_kin, %outer ], [ %e_sub, %inner ]
  %next_i = add i64 %i, 1
  %cont_i = icmp ult i64 %next_i, 5
  br i1 %cont_i, label %outer, label %done

done:
  ret double %e_after_inner
}

define void @advance() {
entry:
  %dtv = load double, ptr @dt, align 8
  br label %outer

outer:
  %i = phi i64 [ 0, %entry ], [ %next_i, %outer_latch ]
  %j0 = add i64 %i, 1
  %has_inner = icmp ult i64 %j0, 5
  br i1 %has_inner, label %inner, label %outer_latch

inner:
  %j = phi i64 [ %j0, %outer ], [ %next_j, %inner ]
  %pxi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 0
  %pyi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 1
  %pzi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 2
  %pvxi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 3
  %pvyi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 4
  %pvzi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 5
  %pmi = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %i, i32 6
  %pxj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 0
  %pyj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 1
  %pzj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 2
  %pvxj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 3
  %pvyj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 4
  %pvzj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 5
  %pmj = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %j, i32 6
  %xi = load double, ptr %pxi, align 8
  %yi = load double, ptr %pyi, align 8
  %zi = load double, ptr %pzi, align 8
  %mi = load double, ptr %pmi, align 8
  %xj = load double, ptr %pxj, align 8
  %yj = load double, ptr %pyj, align 8
  %zj = load double, ptr %pzj, align 8
  %mj = load double, ptr %pmj, align 8
  %dx = fsub double %xi, %xj
  %dy = fsub double %yi, %yj
  %dz = fsub double %zi, %zj
  %dx2 = fmul double %dx, %dx
  %dy2 = fmul double %dy, %dy
  %dz2 = fmul double %dz, %dz
  %d2a = fadd double %dx2, %dy2
  %d2 = fadd double %d2a, %dz2
  %dist = call double @llvm.sqrt.f64(double %d2)
  %d3 = fmul double %d2, %dist
  %mag = fdiv double %dtv, %d3
  %jm = fmul double %mj, %mag
  %im = fmul double %mi, %mag
  %vxi = load double, ptr %pvxi, align 8
  %vyi = load double, ptr %pvyi, align 8
  %vzi = load double, ptr %pvzi, align 8
  %dvxi = fmul double %dx, %jm
  %dvyi = fmul double %dy, %jm
  %dvzi = fmul double %dz, %jm
  %nvxi = fsub double %vxi, %dvxi
  %nvyi = fsub double %vyi, %dvyi
  %nvzi = fsub double %vzi, %dvzi
  store double %nvxi, ptr %pvxi, align 8
  store double %nvyi, ptr %pvyi, align 8
  store double %nvzi, ptr %pvzi, align 8
  %vxj = load double, ptr %pvxj, align 8
  %vyj = load double, ptr %pvyj, align 8
  %vzj = load double, ptr %pvzj, align 8
  %dvxj = fmul double %dx, %im
  %dvyj = fmul double %dy, %im
  %dvzj = fmul double %dz, %im
  %nvxj = fadd double %vxj, %dvxj
  %nvyj = fadd double %vyj, %dvyj
  %nvzj = fadd double %vzj, %dvzj
  store double %nvxj, ptr %pvxj, align 8
  store double %nvyj, ptr %pvyj, align 8
  store double %nvzj, ptr %pvzj, align 8
  %next_j = add i64 %j, 1
  %cont_j = icmp ult i64 %next_j, 5
  br i1 %cont_j, label %inner, label %outer_latch

outer_latch:
  %next_i = add i64 %i, 1
  %cont_i = icmp ult i64 %next_i, 5
  br i1 %cont_i, label %outer, label %pos_loop

pos_loop:
  %k = phi i64 [ 0, %outer_latch ], [ %next_k, %pos_loop ]
  %pkx = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %k, i32 0
  %pky = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %k, i32 1
  %pkz = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %k, i32 2
  %pkvx = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %k, i32 3
  %pkvy = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %k, i32 4
  %pkvz = getelementptr [5 x %Body], ptr @bodies, i64 0, i64 %k, i32 5
  %kx = load double, ptr %pkx, align 8
  %ky = load double, ptr %pky, align 8
  %kz = load double, ptr %pkz, align 8
  %kvx = load double, ptr %pkvx, align 8
  %kvy = load double, ptr %pkvy, align 8
  %kvz = load double, ptr %pkvz, align 8
  %dxk = fmul double %dtv, %kvx
  %dyk = fmul double %dtv, %kvy
  %dzk = fmul double %dtv, %kvz
  %nkx = fadd double %kx, %dxk
  %nky = fadd double %ky, %dyk
  %nkz = fadd double %kz, %dzk
  store double %nkx, ptr %pkx, align 8
  store double %nky, ptr %pky, align 8
  store double %nkz, ptr %pkz, align 8
  %next_k = add i64 %k, 1
  %cont_k = icmp ult i64 %next_k, 5
  br i1 %cont_k, label %pos_loop, label %ret_block

ret_block:
  ret void
}

define double @run_nbody(i64 %steps) {
entry:
  call void @offset_momentum()
  %check = icmp eq i64 %steps, 0
  br i1 %check, label %done, label %step_loop

step_loop:
  %s = phi i64 [ 0, %entry ], [ %next_s, %step_loop ]
  call void @advance()
  %next_s = add i64 %s, 1
  %more = icmp ult i64 %next_s, %steps
  br i1 %more, label %step_loop, label %done

done:
  %final_e = call double @energy()
  ret double %final_e
}
)";

        // Compute C++ reference final energy for 10 steps.
        auto OffsetMomentumRef = [&]()
        {
            double PX = 0.0;
            double PY = 0.0;
            double PZ = 0.0;
            for (const Body& B : Bodies)
            {
                PX += B.VX * B.Mass;
                PY += B.VY * B.Mass;
                PZ += B.VZ * B.Mass;
            }
            Bodies[0].VX = -PX / SolarMass;
            Bodies[0].VY = -PY / SolarMass;
            Bodies[0].VZ = -PZ / SolarMass;
        };
        auto AdvanceRef = [&]()
        {
            for (std::size_t I = 0; I < Bodies.size(); ++I)
            {
                for (std::size_t J = I + 1; J < Bodies.size(); ++J)
                {
                    const double DX = Bodies[I].X - Bodies[J].X;
                    const double DY = Bodies[I].Y - Bodies[J].Y;
                    const double DZ = Bodies[I].Z - Bodies[J].Z;
                    const double Dist = std::sqrt(DX * DX + DY * DY + DZ * DZ);
                    const double Mag = TimeStep / (Dist * Dist * Dist);
                    Bodies[I].VX -= DX * Bodies[J].Mass * Mag;
                    Bodies[I].VY -= DY * Bodies[J].Mass * Mag;
                    Bodies[I].VZ -= DZ * Bodies[J].Mass * Mag;
                    Bodies[J].VX += DX * Bodies[I].Mass * Mag;
                    Bodies[J].VY += DY * Bodies[I].Mass * Mag;
                    Bodies[J].VZ += DZ * Bodies[I].Mass * Mag;
                }
            }
            for (Body& B : Bodies)
            {
                B.X += TimeStep * B.VX;
                B.Y += TimeStep * B.VY;
                B.Z += TimeStep * B.VZ;
            }
        };
        auto EnergyRef = [&]()
        {
            double E = 0.0;
            for (std::size_t I = 0; I < Bodies.size(); ++I)
            {
                E += 0.5 * Bodies[I].Mass * (Bodies[I].VX * Bodies[I].VX + Bodies[I].VY * Bodies[I].VY + Bodies[I].VZ * Bodies[I].VZ);
                for (std::size_t J = I + 1; J < Bodies.size(); ++J)
                {
                    const double DX = Bodies[I].X - Bodies[J].X;
                    const double DY = Bodies[I].Y - Bodies[J].Y;
                    const double DZ = Bodies[I].Z - Bodies[J].Z;
                    E -= Bodies[I].Mass * Bodies[J].Mass / std::sqrt(DX * DX + DY * DY + DZ * DZ);
                }
            }
            return E;
        };

        OffsetMomentumRef();
        const double ExpectedInitialEnergy = EnergyRef();
        for (int Step = 0; Step < 10; ++Step)
        {
            AdvanceRef();
        }
        const double ExpectedFinalEnergy = EnergyRef();

        const std::string Assembly = compileIRToAssembly(IR.str(), "nbody_mod");
        EXPECT_NE(Assembly.find("fsqrt64rr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("fneg64rr"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("call @advance"), std::string::npos) << Assembly;

        const std::array<std::uint64_t, 1> ZeroSteps{0};
        const double ActualInitialEnergy = std::bit_cast<double>(executeCompiledBits(Assembly, "run_nbody", ZeroSteps));
        EXPECT_LE(std::abs(ActualInitialEnergy - ExpectedInitialEnergy), 1.0e-12 * std::max(1.0, std::abs(ExpectedInitialEnergy)));

        const std::array<std::uint64_t, 1> TenSteps{10};
        const double ActualFinalEnergy = std::bit_cast<double>(executeCompiledBits(Assembly, "run_nbody", TenSteps));
        EXPECT_LE(std::abs(ActualFinalEnergy - ExpectedFinalEnergy), 1.0e-12 * std::max(1.0, std::abs(ExpectedFinalEnergy)));
    }
} // namespace ink::backend::llvm_inkvm
