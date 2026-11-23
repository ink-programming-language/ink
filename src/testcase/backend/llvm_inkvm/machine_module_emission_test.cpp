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

        // Canonical assembly must escape control/UTF-8 bytes with \HH and preserve direct @.G0/@.F0 symbol names so readMachineModuleText accepts it.
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

    // Verifies unsupported .data constant initializers (blockaddress, symbol differences, and unaligned packed pointers) fail cleanly via explicit errors without aborting.
    TEST(InkVMMachineModuleEmissionTest, ReportsExplicitErrorForUnsupportedGlobalInitializersWithoutAborting)
    {
        constexpr std::array<std::string_view, 3> UnsupportedIRCases{
            R"(@ba = global ptr blockaddress(@main, %target), align 8

define void @main() {
entry:
  br label %target
target:
  ret void
}
)",
            R"(@a = global i64 1, align 8
@b = global i64 2, align 8
@diff = global i64 sub (i64 ptrtoint (ptr @a to i64), i64 ptrtoint (ptr @b to i64)), align 8

define void @main() {
entry:
  ret void
}
)",
            R"(@target = global i64 1, align 8
@packed_ptr = global <{ i8, ptr }> <{ i8 1, ptr @target }>, align 8

define void @main() {
entry:
  ret void
}
)",
        };

        for (const std::string_view IRText : UnsupportedIRCases)
        {
            SCOPED_TRACE(IRText);
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
    }

    // Verifies LLVM IR .data symbol relocations (direct pointers, constant GEPs, ptrtoint+add, and cyclic structs) emit, round-trip in text, and execute cleanly.
    TEST(InkVMMachineModuleEmissionTest, EmitsRoundTripsAndExecutesGlobalDataSymbolRelocations)
    {
        constexpr std::string_view IRText = R"(%Node = type { i64, ptr }

@table = constant [4 x i64] [i64 10, i64 20, i64 30, i64 40], align 8
@elem_ptr = constant ptr getelementptr ([4 x i64], ptr @table, i64 0, i64 2), align 8
@addr_plus = constant i64 add (i64 ptrtoint (ptr @table to i64), i64 24), align 8
@n1 = global %Node { i64 100, ptr @n2 }, align 8
@n2 = global %Node { i64 200, ptr @n1 }, align 8
@out = global i64 0, align 8

define void @main() {
entry:
  %p2 = load ptr, ptr @elem_ptr, align 8
  %v2 = load i64, ptr %p2, align 8
  %a3 = load i64, ptr @addr_plus, align 8
  %p3 = inttoptr i64 %a3 to ptr
  %v3 = load i64, ptr %p3, align 8
  %n1_next_slot = getelementptr %Node, ptr @n1, i64 0, i32 1
  %n2_ptr = load ptr, ptr %n1_next_slot, align 8
  %n2_val = load i64, ptr %n2_ptr, align 8
  %n2_next_slot = getelementptr %Node, ptr %n2_ptr, i64 0, i32 1
  %back_to_n1 = load ptr, ptr %n2_next_slot, align 8
  %n1_val = load i64, ptr %back_to_n1, align 8
  %s0 = add i64 %v2, %v3
  %s1 = add i64 %s0, %n2_val
  %s2 = add i64 %s1, %n1_val
  store i64 %s2, ptr @out, align 8
  ret void
}
)";

        llvm::LLVMContext Context;
        auto IRModule = parseIR(Context, IRText, "data_reloc_mod");
        ASSERT_NE(IRModule, nullptr);

        auto DirectModule = emitInkVMMachineModule(*IRModule);
        ASSERT_TRUE(DirectModule.has_value()) << DirectModule.error();

        auto& Names = core::NamePool::instance();
        const auto FindGlobal = [&](std::string_view Name) -> const vm::MachineGlobalVariable*
        {
            const core::Name Target = Names.intern(Name);
            for (const auto& Global : (*DirectModule)->globalVariables())
            {
                if (Global.name() == Target)
                {
                    return &Global;
                }
            }
            return nullptr;
        };

        const auto* ElemPtr = FindGlobal("elem_ptr");
        ASSERT_NE(ElemPtr, nullptr);
        ASSERT_EQ(ElemPtr->relocationCount(), 1u);
        EXPECT_EQ(ElemPtr->relocations()[0], (vm::MachineGlobalRelocation{0, Names.intern("table"), 16}));

        const auto* AddrPlus = FindGlobal("addr_plus");
        ASSERT_NE(AddrPlus, nullptr);
        ASSERT_EQ(AddrPlus->relocationCount(), 1u);
        EXPECT_EQ(AddrPlus->relocations()[0], (vm::MachineGlobalRelocation{0, Names.intern("table"), 24}));

        const auto* Node1 = FindGlobal("n1");
        const auto* Node2 = FindGlobal("n2");
        ASSERT_NE(Node1, nullptr);
        ASSERT_NE(Node2, nullptr);
        ASSERT_EQ(Node1->relocationCount(), 1u);
        EXPECT_EQ(Node1->relocations()[0], (vm::MachineGlobalRelocation{8, Names.intern("n2"), 0}));
        ASSERT_EQ(Node2->relocationCount(), 1u);
        EXPECT_EQ(Node2->relocations()[0], (vm::MachineGlobalRelocation{8, Names.intern("n1"), 0}));

        // Verify canonical assembly emits .reloc lines and round-trips losslessly.
        const auto Text = vm::writeMachineModuleText(**DirectModule);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find("    .reloc 0, @table + 16\n"), std::string::npos) << *Text;
        EXPECT_NE(Text->find("    .reloc 0, @table + 24\n"), std::string::npos) << *Text;
        EXPECT_NE(Text->find("    .reloc 8, @n2\n"), std::string::npos) << *Text;
        EXPECT_NE(Text->find("    .reloc 8, @n1\n"), std::string::npos) << *Text;
        const auto Reparsed = vm::readMachineModuleText(*Text);
        ASSERT_TRUE(Reparsed.has_value()) << *Text;
        const auto Rewritten = vm::writeMachineModuleText(**Reparsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);

        // Execute the emitted MachineModule in Interpreter: 30 + 40 + 200 + 100 = 370.
        core::CollectingDiagnosticConsumer DiagnosticsConsumer;
        vm::Interpreter Interpreter;
        Interpreter.getDiagnostics().addConsumer(DiagnosticsConsumer);
        const core::Name MainName = Names.intern("main");
        const core::Name OutName = Names.intern("out");
        const std::array ExportNames{MainName, OutName};
        vm::ExecutableEmitter Emitter(Interpreter.getDiagnostics());
        vm::ExecutableFile Executable = Emitter.emit(**DirectModule, MainName, ExportNames);
        ASSERT_NE(Interpreter.loadLibrary(std::move(Executable)), vm::Interpreter::InvalidLibraryHandle);

        const void* EntryAddress = Interpreter.resolveSymbolAddress(MainName);
        ASSERT_NE(EntryAddress, nullptr);
        ASSERT_TRUE(Interpreter.execute(EntryAddress));
        EXPECT_TRUE(DiagnosticsConsumer.takeDiagnostics().empty());

        const void* OutAddress = Interpreter.resolveSymbolAddress(OutName);
        ASSERT_NE(OutAddress, nullptr);
        std::int64_t OutValue = 0;
        std::memcpy(&OutValue, OutAddress, sizeof(OutValue));
        EXPECT_EQ(OutValue, 370);
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

    // Verifies constant lowering handles global aliases, negative relocation offsets, inttoptr constants, FP64 and multi-byte arrays, while skipping llvm metadata globals.
    TEST(InkVMMachineModuleEmissionTest, LowersConstantAliasesNegativeAddendsIntToPtrFPAndSkipsMetadataGlobals)
    {
        constexpr std::string_view IRText = R"(@llvm.metadata_ignored = global i64 99, section "llvm.metadata", align 8
@arr = constant [4 x i64] [i64 1, i64 2, i64 3, i64 4], align 8
@arr_elem1 = alias i64, getelementptr ([4 x i64], ptr @arr, i64 0, i64 1)
@via_alias = constant ptr @arr_elem1, align 8
@neg_reloc = constant i64 sub (i64 ptrtoint (ptr @arr to i64), i64 8), align 8
@raw_ptr64 = constant ptr inttoptr (i64 4660 to ptr), align 8
@raw_ptr32 = constant ptr inttoptr (i32 305419896 to ptr), align 8
@fp_val = constant double 1.5, align 8
@u16_seq = constant [2 x i16] [i16 258, i16 772], align 2
)";

        llvm::LLVMContext Context;
        auto IRModule = parseIR(Context, IRText, "const_lowering_edges");
        ASSERT_NE(IRModule, nullptr);

        auto DirectModule = emitInkVMMachineModule(*IRModule);
        ASSERT_TRUE(DirectModule.has_value()) << DirectModule.error();

        auto& Names = core::NamePool::instance();
        const auto FindGlobal = [&](std::string_view Name) -> const vm::MachineGlobalVariable*
        {
            const core::Name Target = Names.intern(Name);
            for (const auto& Global : (*DirectModule)->globalVariables())
            {
                if (Global.name() == Target)
                {
                    return &Global;
                }
            }
            return nullptr;
        };

        EXPECT_EQ(FindGlobal("llvm.metadata_ignored"), nullptr);

        const auto* ViaAlias = FindGlobal("via_alias");
        ASSERT_NE(ViaAlias, nullptr);
        ASSERT_EQ(ViaAlias->relocationCount(), 1u);
        EXPECT_EQ(ViaAlias->relocations()[0], (vm::MachineGlobalRelocation{0, Names.intern("arr"), 8}));

        const auto* NegReloc = FindGlobal("neg_reloc");
        ASSERT_NE(NegReloc, nullptr);
        ASSERT_EQ(NegReloc->relocationCount(), 1u);
        EXPECT_EQ(NegReloc->relocations()[0], (vm::MachineGlobalRelocation{0, Names.intern("arr"), -8}));

        const auto* RawPtr64 = FindGlobal("raw_ptr64");
        ASSERT_NE(RawPtr64, nullptr);
        EXPECT_EQ(RawPtr64->relocationCount(), 0u);
        const std::array<std::uint8_t, 2> ExpectedPtr64Bytes{0x34, 0x12};
        EXPECT_TRUE(std::ranges::equal(RawPtr64->initialBytes(), ExpectedPtr64Bytes));

        const auto* RawPtr32 = FindGlobal("raw_ptr32");
        ASSERT_NE(RawPtr32, nullptr);
        EXPECT_EQ(RawPtr32->relocationCount(), 0u);
        const std::array<std::uint8_t, 4> ExpectedPtr32Bytes{0x78, 0x56, 0x34, 0x12};
        EXPECT_TRUE(std::ranges::equal(RawPtr32->initialBytes(), ExpectedPtr32Bytes));

        const auto* FPVal = FindGlobal("fp_val");
        ASSERT_NE(FPVal, nullptr);
        const std::array<std::uint8_t, 8> ExpectedFPBytes{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x3F};
        EXPECT_TRUE(std::ranges::equal(FPVal->initialBytes(), ExpectedFPBytes));

        const auto* U16Seq = FindGlobal("u16_seq");
        ASSERT_NE(U16Seq, nullptr);
        const std::array<std::uint8_t, 4> ExpectedU16Bytes{0x02, 0x01, 0x04, 0x03};
        EXPECT_TRUE(std::ranges::equal(U16Seq->initialBytes(), ExpectedU16Bytes));
    }

    // Verifies invalid constant expression relocations (adding two symbols, subtracting a symbol, signed addend overflow, and metadata refs) fail with explicit errors.
    TEST(InkVMMachineModuleEmissionTest, ReportsExplicitErrorForInvalidConstantExprRelocations)
    {
        struct InvalidCase
        {
                std::string_view IRText;
                std::string_view ExpectedErrorSubstring;
        };

        constexpr std::array<InvalidCase, 4> InvalidIRCases{
            InvalidCase{
                R"(@a = global i64 1, align 8
@b = global i64 2, align 8
@sum_syms = global i64 add (i64 ptrtoint (ptr @a to i64), i64 ptrtoint (ptr @b to i64)), align 8
)",
                "cannot add two symbol addresses",
            },
            InvalidCase{
                R"(@a = global i64 1, align 8
@neg_sym = global i64 sub (i64 16, i64 ptrtoint (ptr @a to i64)), align 8
)",
                "does not support subtracting a symbol address",
            },
            InvalidCase{
                R"(@a = global i64 1, align 8
@overflow_reloc = global i64 add (i64 ptrtoint (ptr getelementptr (i8, ptr @a, i64 9223372036854775807) to i64), i64 1), align 8
)",
                "overflows signed 64-bit range",
            },
            InvalidCase{
                R"(@meta = global i64 1, section "llvm.metadata", align 8
@ref_meta = global ptr @meta, align 8
)",
                "LLVM metadata global",
            },
        };

        for (const InvalidCase& Case : InvalidIRCases)
        {
            SCOPED_TRACE(Case.IRText);
            llvm::LLVMContext Context;
            auto Mod = parseIR(Context, Case.IRText, "const_reloc_err");
            ASSERT_NE(Mod, nullptr);
            const auto Result = emitInkVMMachineModule(*Mod);
            ASSERT_FALSE(Result.has_value());
            EXPECT_NE(Result.error().find(Case.ExpectedErrorSubstring), std::string::npos) << Result.error();
        }
    }
} // namespace ink::backend::llvm_inkvm
