#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include <gtest/gtest.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

// Include VM runtime headers after LLVM headers because core_define.h defines a check() macro colliding with PassManagerInternal.h.
#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/name/name_pool.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace ink::backend::llvm_inkvm
{
    namespace
    {
        std::unique_ptr<llvm::Module> parseIR(llvm::LLVMContext& Context, std::string_view IRText, std::string_view ModuleName)
        {
            llvm::SMDiagnostic ParseError;
            auto ParsedModule = llvm::parseAssemblyString(llvm::StringRef(IRText.data(), IRText.size()), ParseError, Context);
            if (ParsedModule != nullptr)
            {
                ParsedModule->setModuleIdentifier(llvm::StringRef(ModuleName.data(), ModuleName.size()));
            }
            return ParsedModule;
        }
    } // namespace

    // Verifies direct LLVM IR -> MachineModule emission populates module name, declarations, globals, functions, blocks, and typed instructions.
    TEST(InkVMMachineModuleEmissionTest, EmitsTypedMachineModuleMetadataGlobalsAndInstructionsDirectly)
    {
        constexpr std::string_view IRText = R"(@ext_const = external constant i64
@ext_mut = external global i64
declare i64 @ext_fn(i64)

@value = global i64 42, align 8
@ro_tag = internal constant [4 x i8] c"INK\00", align 1

define internal i64 @helper(i64 %x) {
entry:
  %v = load i64, ptr @value, align 8
  %sum = add i64 %x, %v
  ret i64 %sum
}

define i64 @main_entry(i64 %arg) {
entry:
  %r = call i64 @helper(i64 %arg)
  ret i64 %r
}
)";

        llvm::LLVMContext Context;
        auto IRModule = parseIR(Context, IRText, "direct_model");
        ASSERT_NE(IRModule, nullptr);

        auto Emitted = emitInkVMMachineModule(*IRModule);
        ASSERT_TRUE(Emitted.has_value()) << Emitted.error();
        const vm::MachineModule& ModuleValue = **Emitted;

        auto& Names = core::NamePool::instance();
        EXPECT_EQ(ModuleValue.name(), Names.intern("direct_model"));

        const auto Declarations = ModuleValue.symbolDeclarations();
        ASSERT_EQ(Declarations.size(), 3u);
        EXPECT_EQ(Declarations[0].Name, Names.intern("ext_const"));
        EXPECT_EQ(Declarations[0].Kind, vm::MachineSymbolKind::GlobalVariable);
        EXPECT_TRUE(Declarations[0].IsConstant);
        EXPECT_EQ(Declarations[1].Name, Names.intern("ext_mut"));
        EXPECT_EQ(Declarations[1].Kind, vm::MachineSymbolKind::GlobalVariable);
        EXPECT_FALSE(Declarations[1].IsConstant);
        EXPECT_EQ(Declarations[2].Name, Names.intern("ext_fn"));
        EXPECT_EQ(Declarations[2].Kind, vm::MachineSymbolKind::Function);
        EXPECT_FALSE(Declarations[2].IsConstant);

        ASSERT_EQ(ModuleValue.globalVariableCount(), 2u);
        auto GlobalIt = ModuleValue.globalVariables().begin();
        EXPECT_EQ(GlobalIt->name(), Names.intern("value"));
        EXPECT_EQ(GlobalIt->byteSize(), 8u);
        EXPECT_EQ(GlobalIt->alignment(), 8u);
        EXPECT_EQ(GlobalIt->linkage(), vm::MachineSymbolLinkage::External);
        EXPECT_FALSE(GlobalIt->isConstant());
        const std::array<std::uint8_t, 1> ExpectedValueBytes{42};
        EXPECT_TRUE(std::ranges::equal(GlobalIt->initialBytes(), ExpectedValueBytes));

        ++GlobalIt;
        EXPECT_EQ(GlobalIt->name(), Names.intern("ro_tag"));
        EXPECT_EQ(GlobalIt->byteSize(), 4u);
        EXPECT_EQ(GlobalIt->alignment(), 1u);
        EXPECT_EQ(GlobalIt->linkage(), vm::MachineSymbolLinkage::Local);
        EXPECT_TRUE(GlobalIt->isConstant());
        const std::array<std::uint8_t, 3> ExpectedTagBytes{'I', 'N', 'K'};
        EXPECT_TRUE(std::ranges::equal(GlobalIt->initialBytes(), ExpectedTagBytes));

        ASSERT_EQ(ModuleValue.functionCount(), 2u);
        auto FuncIt = ModuleValue.functions().begin();
        EXPECT_EQ(FuncIt->name(), Names.intern("helper"));
        EXPECT_EQ(FuncIt->linkage(), vm::MachineSymbolLinkage::Local);
        ASSERT_EQ(FuncIt->basicBlockCount(), 1u);
        EXPECT_EQ(FuncIt->basicBlocks().begin()->name(), Names.intern("entry"));

        bool FoundSymbolLoad = false;
        for (const vm::MachineInstruction& Inst : FuncIt->basicBlocks().begin()->instructions())
        {
            if (Inst.getOpcode() == vm::Opcode::Load64rm)
            {
                const auto& Load = static_cast<const vm::MachineLoad64rmInstruction&>(Inst);
                if (Load.getSource().getSymbol() == Names.intern("value") && Load.getSource().getByteOffset() == 0)
                {
                    FoundSymbolLoad = true;
                }
            }
        }
        EXPECT_TRUE(FoundSymbolLoad);

        ++FuncIt;
        EXPECT_EQ(FuncIt->name(), Names.intern("main_entry"));
        EXPECT_EQ(FuncIt->linkage(), vm::MachineSymbolLinkage::External);
        bool FoundHelperCall = false;
        for (const vm::MachineInstruction& Inst : FuncIt->basicBlocks().begin()->instructions())
        {
            if (Inst.getOpcode() == vm::Opcode::Calli)
            {
                const auto& Call = static_cast<const vm::MachineCalliInstruction&>(Inst);
                if (Call.getTarget().getSymbol() == Names.intern("helper"))
                {
                    FoundHelperCall = true;
                }
            }
        }
        EXPECT_TRUE(FoundHelperCall);
    }

    // Verifies non-ASCII UTF-8 names, control characters, spaces, and .G0/.F0 label collisions work both directly and through canonical text round-trip.
    TEST(InkVMMachineModuleEmissionTest, HandlesNonASCIIControlCharsAndLabelCollisionsDirectlyAndViaTextRoundTrip)
    {
        constexpr std::string_view IRText = R"(@"space global" = global i64 11, align 8
@.G0 = global i64 29, align 8
@"utf8_\E4\B8\AD\E6\96\87" = constant i64 2, align 8

define internal i64 @"space helper"() {
  %a = load i64, ptr @"space global", align 8
  %b = load i64, ptr @.G0, align 8
  %c = load i64, ptr @"utf8_\E4\B8\AD\E6\96\87", align 8
  %s0 = add i64 %a, %b
  %s1 = add i64 %s0, %c
  ret i64 %s1
}

define internal i64 @.F0() {
  %r = call i64 @"space helper"()
  ret i64 %r
}

define void @main() {
entry:
  %v = call i64 @.F0()
  store i64 %v, ptr @"space global", align 8
  ret void
}
)";

        const std::string SpecialModuleName = std::string("模块\n\t\"path\\") + std::string(1, '\x01');
        llvm::LLVMContext Context;
        auto IRModule = parseIR(Context, IRText, SpecialModuleName);
        ASSERT_NE(IRModule, nullptr);

        auto DirectModule = emitInkVMMachineModule(*IRModule);
        ASSERT_TRUE(DirectModule.has_value()) << DirectModule.error();
        EXPECT_EQ(core::NamePool::instance().text((*DirectModule)->name()), SpecialModuleName);

        // Canonical assembly must escape control/UTF-8 bytes with \HH and avoid .G0/.F0 collisions so readMachineModuleText accepts it.
        std::string Assembly;
        std::string ErrorMessage;
        ASSERT_TRUE(emitInkVMAssembly(*IRModule, Assembly, ErrorMessage)) << ErrorMessage;
        auto Reparsed = vm::readMachineModuleText(Assembly);
        ASSERT_TRUE(Reparsed.has_value()) << Assembly;
        const auto Rewritten = vm::writeMachineModuleText(**Reparsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, Assembly);

        // Execute the directly emitted MachineModule in Interpreter and verify the updated global holds 11 + 29 + 2 = 42.
        core::CollectingDiagnosticConsumer DiagnosticsConsumer;
        vm::Interpreter Interpreter;
        Interpreter.getDiagnostics().addConsumer(DiagnosticsConsumer);

        auto& Names = core::NamePool::instance();
        const core::Name MainName = Names.intern("main");
        const core::Name ResultName = Names.intern("space global");
        const std::array ExportNames{MainName, ResultName};

        vm::ExecutableEmitter Emitter(Interpreter.getDiagnostics());
        vm::ExecutableFile Executable = Emitter.emit(**DirectModule, MainName, ExportNames);
        const auto Handle = Interpreter.loadLibrary(std::move(Executable));
        ASSERT_NE(Handle, vm::Interpreter::InvalidLibraryHandle);

        const void* EntryAddress = Interpreter.resolveSymbolAddress(MainName);
        ASSERT_NE(EntryAddress, nullptr);
        EXPECT_TRUE(Interpreter.execute(EntryAddress));
        EXPECT_TRUE(DiagnosticsConsumer.takeDiagnostics().empty());

        const void* ResultAddress = Interpreter.resolveSymbolAddress(ResultName);
        ASSERT_NE(ResultAddress, nullptr);
        std::int64_t Result = 0;
        std::memcpy(&Result, ResultAddress, sizeof(Result));
        EXPECT_EQ(Result, 42);
    }

    // Verifies direct MachineModule serialization, emitInkVMAssembly, and LLVM TargetMachine AssemblyFile emit byte-identical canonical text.
    TEST(InkVMMachineModuleEmissionTest, MatchesBetweenDirectEmissionAssemblyHelperAndLLVMTargetMachineAsmFile)
    {
        constexpr std::string_view IRText = R"(@acc = global i64 7, align 8

define i64 @add_acc(i64 %x) {
entry:
  %cur = load i64, ptr @acc, align 8
  %sum = add i64 %cur, %x
  store i64 %sum, ptr @acc, align 8
  ret i64 %sum
}
)";

        llvm::LLVMContext ContextA;
        auto ModA = parseIR(ContextA, IRText, "unified_output");
        ASSERT_NE(ModA, nullptr);
        auto DirectModule = emitInkVMMachineModule(*ModA);
        ASSERT_TRUE(DirectModule.has_value()) << DirectModule.error();
        const auto DirectText = vm::writeMachineModuleText(**DirectModule);
        ASSERT_TRUE(DirectText.has_value());

        llvm::LLVMContext ContextB;
        auto ModB = parseIR(ContextB, IRText, "unified_output");
        ASSERT_NE(ModB, nullptr);
        std::string HelperAssembly;
        std::string HelperError;
        ASSERT_TRUE(emitInkVMAssembly(*ModB, HelperAssembly, HelperError)) << HelperError;
        EXPECT_EQ(HelperAssembly, *DirectText);

        llvm::LLVMContext ContextC;
        auto ModC = parseIR(ContextC, IRText, "unified_output");
        ASSERT_NE(ModC, nullptr);
        std::string TMError;
        const auto TM = createInkVMTargetMachine(llvm::CodeGenOptLevel::Default, TMError);
        ASSERT_NE(TM, nullptr) << TMError;
        ModC->setDataLayout(TM->createDataLayout());
        ModC->setTargetTriple(TM->getTargetTriple());
        llvm::SmallString<512> AsmBuffer;
        llvm::raw_svector_ostream AsmStream(AsmBuffer);
        llvm::legacy::PassManager PM;
        ASSERT_FALSE(TM->addPassesToEmitFile(PM, AsmStream, nullptr, llvm::CodeGenFileType::AssemblyFile));
        PM.run(*ModC);
        EXPECT_EQ(std::string_view(AsmBuffer.data(), AsmBuffer.size()), *DirectText);
    }

    // Verifies unsupported .data symbol relocation initializers fail cleanly via explicit errors instead of aborting the process.
    TEST(InkVMMachineModuleEmissionTest, ReportsExplicitErrorForUnsupportedGlobalRelocationInitializersWithoutAborting)
    {
        constexpr std::string_view IRText = R"(@target = global i64 1, align 8
@ptr_slot = global ptr @target, align 8

define void @main() {
entry:
  ret void
}
)";

        llvm::LLVMContext ContextA;
        auto ModA = parseIR(ContextA, IRText, "reloc_data_err");
        ASSERT_NE(ModA, nullptr);
        const auto DirectResult = emitInkVMMachineModule(*ModA);
        ASSERT_FALSE(DirectResult.has_value());
        EXPECT_FALSE(DirectResult.error().empty());

        llvm::LLVMContext ContextB;
        auto ModB = parseIR(ContextB, IRText, "reloc_data_err");
        ASSERT_NE(ModB, nullptr);
        std::string Assembly = "stale";
        std::string ErrorMessage;
        EXPECT_FALSE(emitInkVMAssembly(*ModB, Assembly, ErrorMessage));
        EXPECT_TRUE(Assembly.empty());
        EXPECT_FALSE(ErrorMessage.empty());
    }

    // Verifies empty and globals-only LLVM modules emit valid MachineModules with default "inkvm" naming when unnamed.
    TEST(InkVMMachineModuleEmissionTest, EmitsEmptyAndGlobalsOnlyModulesCleanly)
    {
        llvm::LLVMContext EmptyContext;
        llvm::Module EmptyMod("", EmptyContext);
        auto EmptyEmitted = emitInkVMMachineModule(EmptyMod);
        ASSERT_TRUE(EmptyEmitted.has_value()) << EmptyEmitted.error();
        EXPECT_EQ(core::NamePool::instance().text((*EmptyEmitted)->name()), "inkvm");
        EXPECT_EQ((*EmptyEmitted)->globalVariableCount(), 0u);
        EXPECT_EQ((*EmptyEmitted)->functionCount(), 0u);
        EXPECT_TRUE((*EmptyEmitted)->symbolDeclarations().empty());

        llvm::LLVMContext DataContext;
        auto DataMod = parseIR(DataContext, "@only_data = constant i32 305419896, align 4\n", "data_only");
        ASSERT_NE(DataMod, nullptr);
        auto DataEmitted = emitInkVMMachineModule(*DataMod);
        ASSERT_TRUE(DataEmitted.has_value()) << DataEmitted.error();
        EXPECT_EQ((*DataEmitted)->functionCount(), 0u);
        ASSERT_EQ((*DataEmitted)->globalVariableCount(), 1u);
        const std::array<std::uint8_t, 4> ExpectedBytes{0x78, 0x56, 0x34, 0x12};
        EXPECT_TRUE(std::ranges::equal((*DataEmitted)->globalVariables().begin()->initialBytes(), ExpectedBytes));
    }
} // namespace ink::backend::llvm_inkvm
