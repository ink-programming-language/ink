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
            Stream << ".module \"mem_harness\"\n";
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
            auto Linked = vm::ModuleLinker::link(Modules, Interpreter.getDiagnostics(), core::NamePool::instance().intern("linked_mem"));
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

    // Verifies writable and readonly global scalars fold into single-instruction symbol-relative loads and stores ([@sym]).
    TEST(InkVMMemoryGlobalCodegenTest, EmitsDataSectionAndFoldsDirectGlobalScalarAccess)
    {
        constexpr std::string_view IRText = R"(@counter = global i64 40, align 8
@step = constant i64 2, align 8

define i64 @bump_and_read(i64 %delta) {
entry:
  %cur = load i64, ptr @counter, align 8
  %s = load i64, ptr @step, align 8
  %next = add i64 %cur, %s
  %updated = add i64 %next, %delta
  store i64 %updated, ptr @counter, align 8
  %reloaded = load i64, ptr @counter, align 8
  ret i64 %reloaded
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "global_scalar_mod");
        EXPECT_NE(Assembly.find(".section .data"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".global counter, 8, 8 external"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(".global step, 8, 8 readonly external"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("load64rm "), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("[@counter]"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("store64mr [@counter], "), std::string::npos) << Assembly;

        const std::array<std::int64_t, 1> Args{5};
        // 40 + 2 + 5 = 47
        EXPECT_EQ(executeCompiled(Assembly, "bump_and_read", Args), 47);
    }

    // Verifies global arrays and strings support both constant-offset folding ([@table + 16]) and dynamic indexing.
    TEST(InkVMMemoryGlobalCodegenTest, SupportsGlobalArraysStringsAndConstantOffsetFolding)
    {
        constexpr std::string_view IRText = R"(@table = internal constant [4 x i64] [i64 10, i64 20, i64 30, i64 40], align 8
@.str = private unnamed_addr constant [5 x i8] c"Ink!\00", align 1

define i64 @read_table_and_str(i64 %idx) {
entry:
  %fixed_ptr = getelementptr [4 x i64], ptr @table, i64 0, i64 2
  %fixed_val = load i64, ptr %fixed_ptr, align 8
  %dyn_ptr = getelementptr [4 x i64], ptr @table, i64 0, i64 %idx
  %dyn_val = load i64, ptr %dyn_ptr, align 8
  %ch_ptr = getelementptr [5 x i8], ptr @.str, i64 0, i64 1
  %ch = load i8, ptr %ch_ptr, align 1
  %ch_ext = zext i8 %ch to i64
  %s0 = add i64 %fixed_val, %dyn_val
  %s1 = add i64 %s0, %ch_ext
  ret i64 %s1
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "global_array_mod");
        EXPECT_NE(Assembly.find("[@table + 16]"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("[@.str + 1]"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("mov64ri "), std::string::npos) << Assembly;

        const std::array<std::int64_t, 1> Args{3};
        // table[2] (30) + table[3] (40) + 'n' (110) = 180
        EXPECT_EQ(executeCompiled(Assembly, "read_table_and_str", Args), 180);
    }

    // Verifies global struct layout padding and mixed-width signed/unsigned narrow loads (i8, i16, i32, i64).
    TEST(InkVMMemoryGlobalCodegenTest, SerializesStructPaddingAndExecutesMixedWidthGlobalLoads)
    {
        constexpr std::string_view IRText = R"(%Record = type { i8, i16, i32, i64 }
@rec = constant %Record { i8 -5, i16 -300, i32 1000, i64 100000 }, align 8

define i64 @sum_record() {
entry:
  %p0 = getelementptr %Record, ptr @rec, i64 0, i32 0
  %p1 = getelementptr %Record, ptr @rec, i64 0, i32 1
  %p2 = getelementptr %Record, ptr @rec, i64 0, i32 2
  %p3 = getelementptr %Record, ptr @rec, i64 0, i32 3
  %b0 = load i8, ptr %p0, align 1
  %b1 = load i16, ptr %p1, align 2
  %b2 = load i32, ptr %p2, align 4
  %b3 = load i64, ptr %p3, align 8
  %e0 = sext i8 %b0 to i64
  %e1 = sext i16 %b1 to i64
  %e2 = zext i32 %b2 to i64
  %s0 = add i64 %e0, %e1
  %s1 = add i64 %s0, %e2
  %s2 = add i64 %s1, %b3
  ret i64 %s2
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "global_struct_mod");
        // Struct layout: i8 at +0, pad +1, i16 at +2, i32 at +4, i64 at +8 -> total 16 bytes.
        EXPECT_NE(Assembly.find(".global rec, 16, 8 readonly external"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("load8srm "), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("load16srm "), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("load32urm "), std::string::npos) << Assembly;

        const std::span<const std::int64_t> EmptyArgs;
        // -5 + (-300) + 1000 + 100000 = 100695
        EXPECT_EQ(executeCompiled(Assembly, "sum_record", EmptyArgs), 100695);
    }

    // Verifies local alloca arrays on the upward stack, dynamic GEP indexing, and passing a local pointer across a call.
    TEST(InkVMMemoryGlobalCodegenTest, SupportsLocalAllocaArraysAndPassingStackPointersToCalls)
    {
        constexpr std::string_view IRText = R"(define void @write_offset(ptr %base, i64 %index, i64 %value) {
entry:
  %elem = getelementptr i64, ptr %base, i64 %index
  store i64 %value, ptr %elem, align 8
  ret void
}

define i64 @use_local_array(i64 %idx, i64 %val) {
entry:
  %arr = alloca [4 x i64], align 8
  %p0 = getelementptr [4 x i64], ptr %arr, i64 0, i64 0
  %p1 = getelementptr [4 x i64], ptr %arr, i64 0, i64 1
  %p2 = getelementptr [4 x i64], ptr %arr, i64 0, i64 2
  %p3 = getelementptr [4 x i64], ptr %arr, i64 0, i64 3
  store i64 11, ptr %p0, align 8
  store i64 22, ptr %p1, align 8
  store i64 33, ptr %p2, align 8
  store i64 44, ptr %p3, align 8
  call void @write_offset(ptr %arr, i64 %idx, i64 %val)
  %dyn = getelementptr [4 x i64], ptr %arr, i64 0, i64 %idx
  %out = load i64, ptr %dyn, align 8
  %first = load i64, ptr %p0, align 8
  %sum = add i64 %out, %first
  ret i64 %sum
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "local_alloca_mod");
        // Local [4 x i64] (32 bytes at [bp + 16..48)) plus any cross-call spill slot allocated via prologue add64rri sp, sp, N.
        EXPECT_NE(Assembly.find("add64rri sp, sp, "), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find("[bp + 16]"), std::string::npos) << Assembly;
        EXPECT_NE(Assembly.find(", bp, 16"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 2> Args{2, 999};
        // arr[2] = 999, arr[0] = 11 -> 1010
        EXPECT_EQ(executeCompiled(Assembly, "use_local_array", Args), 1010);
    }

    // Verifies zeroinitializer globals omit trailing .byte lines while still being zero-filled in the emitted VM image.
    TEST(InkVMMemoryGlobalCodegenTest, EmitsCompactZeroInitializedGlobalsAndReadsZeroAtRuntime)
    {
        constexpr std::string_view IRText = R"(@zero_buf = global [16 x i64] zeroinitializer, align 8

define i64 @check_zero_buf(i64 %val) {
entry:
  %p5 = getelementptr [16 x i64], ptr @zero_buf, i64 0, i64 5
  %before = load i64, ptr %p5, align 8
  store i64 %val, ptr %p5, align 8
  %after = load i64, ptr %p5, align 8
  %sum = add i64 %before, %after
  ret i64 %sum
}
)";

        const std::string Assembly = compileIRToAssembly(IRText, "zero_global_mod");
        EXPECT_NE(Assembly.find(".global zero_buf, 128, 8 external\n.endglobal"), std::string::npos) << Assembly;

        const std::array<std::int64_t, 1> Args{77};
        EXPECT_EQ(executeCompiled(Assembly, "check_zero_buf", Args), 77);
    }
} // namespace ink::backend::llvm_inkvm
