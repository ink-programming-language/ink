#include "ink/backend/vm/executable/executable_file_validator.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emission_context.h"
#include "ink/backend/vm/executable_emitter/instruction_emitter.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        std::uint64_t readLittleEndian(const ExecutableFile& Executable, std::size_t Offset, std::size_t Count = 8)
        {
            std::uint64_t Bits = 0;
            for (std::size_t Index = 0; Index < Count; ++Index)
            {
                Bits |= static_cast<std::uint64_t>(Executable.getImage()[Offset + Index]) << (Index * 8);
            }
            return Bits;
        }
    } // namespace

    static_assert(std::is_same_v<decltype(std::declval<ExecutableEmitter&>().getExecutable()), const ExecutableFile&>);
    static_assert(std::is_same_v<decltype(std::declval<const ExecutableEmitter&>().getExecutable()), const ExecutableFile&>);

    // Only declared names without VM definitions become imports, including unused functions and read-only data.
    TEST(VMExecutableEmitterTest, DerivesImportsBySubtractingFunctionsAndGlobalsFromDeclarations)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto FunctionName = Names.intern("emitter-declared-vm-function");
        const auto GlobalName = Names.intern("emitter-declared-vm-global");
        const auto ImportFunction = Names.intern("emitter-declared-import-function");
        const auto ImportData = Names.intern("emitter-declared-import-data");
        const auto ImportConstant = Names.intern("emitter-declared-import-constant");
        const auto UndeclaredName = Names.intern("emitter-undeclared-symbol");
        MachineModule Module;
        Module.declareSymbol(FunctionName, MachineSymbolKind::Function);
        Module.declareSymbol(GlobalName, MachineSymbolKind::GlobalVariable);
        Module.declareSymbol(ImportFunction, MachineSymbolKind::Function);
        Module.declareSymbol(ImportFunction, MachineSymbolKind::Function);
        Module.declareSymbol(ImportData, MachineSymbolKind::GlobalVariable);
        Module.declareSymbol(ImportConstant, MachineSymbolKind::GlobalVariable, true);
        Module.createFunction(FunctionName);
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, GlobalName).has_value());
        ExecutableEmitter Emitter(Diagnostics);
        Emitter.scanUnResolvedSymbol(Module);

        EXPECT_FALSE(Emitter.getContext().isImportSymbol(FunctionName));
        EXPECT_FALSE(Emitter.getContext().isImportSymbol(GlobalName));
        EXPECT_FALSE(Emitter.getContext().isImportSymbol(UndeclaredName));
        EXPECT_TRUE(Emitter.getContext().isImportSymbol(ImportFunction));
        EXPECT_TRUE(Emitter.getContext().isImportSymbol(ImportData));
        EXPECT_TRUE(Emitter.getContext().isImportSymbol(ImportConstant));
        EXPECT_TRUE(Emitter.getContext().hasUnResolvedSymbol(FunctionName));
        EXPECT_TRUE(Emitter.getContext().hasUnResolvedSymbol(GlobalName));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(ImportFunction));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(ImportData));
        EXPECT_TRUE(Emitter.getExecutable().getDynamicImportSymbols().empty());
    }

    // Function entries use their actual image positions after an existing prefix and aligned global storage, and leave the unresolved map.
    TEST(VMExecutableEmitterTest, ResolvesFunctionEntriesAfterPrefixAndGlobalStorage)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto GlobalName = Names.intern("emitter-function-layout-global");
        const auto FirstName = Names.intern("emitter-function-layout-first");
        const auto SecondName = Names.intern("emitter-function-layout-second");
        const std::array<std::uint8_t, 2> InitialBytes{0xA1, 0xB2};
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(4, 8, InitialBytes, GlobalName).has_value());
        MachineBasicBlock& First = Module.createFunction(FirstName).createBasicBlock();
        MachineBasicBlock& Second = Module.createFunction(SecondName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Second);
        Builder.create<Opcode::Ret>();
        ExecutableEmitter Emitter(Diagnostics);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(3);
        Emitter.padGlobals(Module);
        ASSERT_TRUE(Emitter.getContext().hasUnResolvedSymbol(FirstName));
        ASSERT_TRUE(Emitter.getContext().hasUnResolvedSymbol(SecondName));
        Emitter.padFunction(Module);

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 32U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(GlobalName), 8U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 16U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 24U);
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(FirstName));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(SecondName));
        EXPECT_EQ(Executable.getImage()[8], 0xA1U);
        EXPECT_EQ(Executable.getImage()[9], 0xB2U);
        EXPECT_EQ(readLittleEndian(Executable, 10, 2), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 16, 2), 0x003AU);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x003AU);
    }

    // Full module emission binds ordinary forward, self, and backward function-address immediates to their laid-out entries.
    TEST(VMExecutableEmitterTest, EmitsFunctionAddressRelocationsAcrossFunctionBodies)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto GlobalName = Names.intern("emitter-function-address-slots");
        const auto FirstName = Names.intern("emitter-function-address-first");
        const auto SecondName = Names.intern("emitter-function-address-second");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(24, 8, {}, GlobalName).has_value());
        MachineBasicBlock& First = Module.createFunction(FirstName).createBasicBlock();
        MachineBasicBlock& Second = Module.createFunction(SecondName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Store64i>(AddressOperand(GlobalName), ImmediateOperand(SecondName));
        Builder.create<Opcode::Store64i>(AddressOperand(GlobalName, 8), ImmediateOperand(FirstName));
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Second);
        Builder.create<Opcode::Store64i>(AddressOperand(GlobalName, 16), ImmediateOperand(FirstName));
        Builder.create<Opcode::Ret>();
        ExecutableEmitter Emitter(Diagnostics);
        const ExecutableFile Executable = Emitter.emit(Module, FirstName, {});

        ASSERT_EQ(Executable.byteSize(), 112U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 24U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 80U);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x000CU);
        EXPECT_EQ(readLittleEndian(Executable, 32), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 56), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 80, 2), 0x000CU);
        EXPECT_EQ(readLittleEndian(Executable, 88), 0U);
        const auto& Relocations = Executable.getImageBaseAddressRelocations();
        ASSERT_EQ(Relocations.size(), 6U);
        constexpr std::array<ImageBaseAddressRelocation, 6> ExpectedRelocations{{
            {32, 0, 0},
            {40, 80, 0},
            {56, 0, 8},
            {64, 24, 0},
            {88, 0, 16},
            {96, 24, 0},
        }};
        for (const auto& [ImageOffset, TargetOffset, Addend] : ExpectedRelocations)
        {
            const auto Position = std::ranges::find(Relocations, ImageOffset, &ImageBaseAddressRelocation::ImageOffset);
            ASSERT_NE(Position, Relocations.end());
            EXPECT_EQ(Position->TargetOffset, TargetOffset);
            EXPECT_EQ(Position->Addend, Addend);
            EXPECT_EQ(readLittleEndian(Executable, ImageOffset), 0U);
        }
        EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Direct calls reserve full addresses for forward, self, and backward VM targets after global storage.
    TEST(VMExecutableEmitterTest, EncodesDirectVMCallsAcrossFunctionBodies)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto FirstName = Names.intern("emitter-direct-call-first");
        const auto SecondName = Names.intern("emitter-direct-call-second");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(5, 1, {}, Names.intern("emitter-direct-call-prefix")).has_value());
        MachineBasicBlock& First = Module.createFunction(FirstName).createBasicBlock();
        MachineBasicBlock& Second = Module.createFunction(SecondName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Calli>(ImmediateOperand(SecondName));
        Builder.create<Opcode::Calli>(ImmediateOperand(FirstName));
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Second);
        Builder.create<Opcode::Calli>(ImmediateOperand(FirstName));
        Builder.create<Opcode::Ret>();
        ExecutableEmitter Emitter(Diagnostics);
        const ExecutableFile Executable = Emitter.emit(Module, FirstName, {});

        ASSERT_EQ(Executable.byteSize(), 72U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 8U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 48U);
        EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 32), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 40, 2), 0x003AU);
        EXPECT_EQ(readLittleEndian(Executable, 48, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 56), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 64, 2), 0x003AU);
        const auto& Relocations = Executable.getImageBaseAddressRelocations();
        ASSERT_EQ(Relocations.size(), 3U);
        constexpr std::array<ImageBaseAddressRelocation, 3> ExpectedRelocations{{
            {16, 48, 0},
            {32, 8, 0},
            {56, 8, 0},
        }};
        for (const auto& Expected : ExpectedRelocations)
        {
            const auto Position = std::ranges::find(Relocations, Expected.ImageOffset, &ImageBaseAddressRelocation::ImageOffset);
            ASSERT_NE(Position, Relocations.end());
            EXPECT_EQ(Position->TargetOffset, Expected.TargetOffset);
            EXPECT_EQ(Position->Addend, 0);
        }
        EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Comparison source addresses use image-base and import bindings while the final block target retains its PC-relative displacement.
    TEST(VMExecutableEmitterTest, BindsComparisonSourceSymbolsAndFinalBranchTargets)
    {
        const core::DiagnosticEngine Diagnostics;
        auto& Names = core::NamePool::instance();
        const auto VMSymbol = Names.intern("emitter-comparison-vm-source");
        const auto ImportSymbol = Names.intern("emitter-comparison-import-source");
        const auto EntryName = Names.intern("emitter-comparison-entry");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, VMSymbol).has_value());
        Module.declareSymbol(ImportSymbol, MachineSymbolKind::GlobalVariable);
        auto& Function = Module.createFunction(EntryName);
        auto& Entry = Function.createBasicBlock();
        auto& Target = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Entry);
        Builder.create<Opcode::Je64mmb>(AddressOperand(VMSymbol, 1), AddressOperand(ImportSymbol, -3), BasicBlockOperand(&Target));
        Builder.create<Opcode::Jltu8mib>(AddressOperand(VMSymbol, 2), ImmediateOperand(0xFF), BasicBlockOperand(&Target));
        Builder.setInsertPoint(Target);
        Builder.create<Opcode::Ret>();
        ExecutableEmitter Emitter(Diagnostics);
        const auto Executable = Emitter.emit(Module, EntryName, {});

        ASSERT_EQ(Executable.byteSize(), 80U);
        EXPECT_EQ(Executable.getEntryOffset(), 8U);
        EXPECT_EQ(Executable.getImage()[10], static_cast<std::uint8_t>(RegisterId::ZERO));
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(Executable.getImage()[11], static_cast<std::uint8_t>(RegisterId::ZERO));
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        EXPECT_EQ(Executable.getImage()[12], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Executable, 32), 64U);
        EXPECT_EQ(readLittleEndian(Executable, 48), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 56), 0xFFU);
        EXPECT_EQ(readLittleEndian(Executable, 64), 32U);
        ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, ImportSymbol);
        ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 24U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex, 0U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].Addend, -3);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 2U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 16U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 0U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, 1);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].ImageOffset, 48U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].TargetOffset, 0U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].Addend, 2);
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Immediate jumps encode forward, self and backward function entries relative to their own opcode without address relocations.
    TEST(VMExecutableEmitterTest, EncodesPCRelativeImmediateJumpsAcrossFunctionBodies)
    {
        const core::DiagnosticEngine Diagnostics;
        auto& Names = core::NamePool::instance();
        const auto FirstName = Names.intern("emitter-direct-jump-first");
        const auto SecondName = Names.intern("emitter-direct-jump-second");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(5, 1, {}, Names.intern("emitter-direct-jump-prefix")).has_value());
        auto& First = Module.createFunction(FirstName).createBasicBlock();
        auto& Second = Module.createFunction(SecondName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Jmpi>(ImmediateOperand(SecondName));
        Builder.create<Opcode::Jmpi>(ImmediateOperand(FirstName));
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Second);
        Builder.create<Opcode::Jmpi>(ImmediateOperand(FirstName));
        Builder.create<Opcode::Ret>();
        ExecutableEmitter Emitter(Diagnostics);
        const auto Executable = Emitter.emit(Module, FirstName, {});

        ASSERT_EQ(Executable.byteSize(), 72U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 8U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 48U);
        EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x003FU);
        EXPECT_EQ(readLittleEndian(Executable, 16), 40U);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x003FU);
        EXPECT_EQ(readLittleEndian(Executable, 32), static_cast<std::uint64_t>(-16));
        EXPECT_EQ(readLittleEndian(Executable, 48, 2), 0x003FU);
        EXPECT_EQ(readLittleEndian(Executable, 56), static_cast<std::uint64_t>(-40));
        EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Register jumps keep their value operands, while memory jump slots use image-base or import address relocations.
    TEST(VMExecutableEmitterTest, EncodesRegisterJumpsAndRelocatesMemoryJumpSlots)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto VMSlot = core::NamePool::instance().intern("emitter-jump-vm-slot");
        const auto ImportSlot = core::NamePool::instance().intern("emitter-jump-import-slot");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, VMSlot).has_value());
        Module.declareSymbol(ImportSlot, MachineSymbolKind::GlobalVariable);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(5);
        Instructions.padInstruction(MachineJmprInstruction(RegisterOperand(RegisterId::PC, -8)));
        Instructions.padInstruction(MachineJmpmInstruction(AddressOperand(VMSlot, 4)));
        Instructions.padInstruction(MachineJmpmInstruction(AddressOperand(ImportSlot, -8)));
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 32), 0U);
        Emitter.padGlobals(Module);
        Emitter.resolveStaticRelocations();

        const auto& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 64U);
        EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x0040U);
        EXPECT_EQ(Executable.getImage()[10], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Executable, 16), static_cast<std::uint64_t>(-8));
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x0041U);
        EXPECT_EQ(Executable.getImage()[26], static_cast<std::uint8_t>(RegisterId::ZERO));
        EXPECT_EQ(readLittleEndian(Executable, 32), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 40, 2), 0x0041U);
        EXPECT_EQ(Executable.getImage()[42], static_cast<std::uint8_t>(RegisterId::ZERO));
        EXPECT_EQ(readLittleEndian(Executable, 48), 0U);
        ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, ImportSlot);
        ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 48U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex, 0U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].Addend, -8);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 1U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 56U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, 4);
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // An unresolved direct call reserves eight address bytes and gains an image-base relocation after module layout.
    TEST(VMExecutableEmitterTest, DefersForwardDirectVMCallUntilModuleFinalization)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto TargetName = core::NamePool::instance().intern("emitter-forward-direct-call-target");
        MachineModule Module;
        MachineBasicBlock& Target = Module.createFunction(TargetName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Target);
        Builder.create<Opcode::Ret>();
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(3);
        Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(TargetName)));

        ASSERT_EQ(Emitter.getExecutable().byteSize(), 24U);
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 8, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 16), 0U);
        Emitter.padFunction(Module);
        ASSERT_EQ(Emitter.getExecutable().byteSize(), 32U);
        EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(TargetName), 24U);
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 16), 0U);
        Emitter.resolveStaticRelocations();

        const ExecutableFile& Executable = Emitter.getExecutable();
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x003AU);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 1U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 16U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 24U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, 0);
        EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    }

    // External direct calls reuse address imports by symbol name and give every call target its own address relocation.
    TEST(VMExecutableEmitterTest, SharesImportsAcrossDirectCallsAndAddressFields)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto SharedSymbol = Names.intern("emitter-import-call-shared");
        const auto OtherSymbol = Names.intern("emitter-import-call-other");
        MachineModule Module;
        Module.declareSymbol(SharedSymbol, MachineSymbolKind::Function);
        Module.declareSymbol(SharedSymbol, MachineSymbolKind::Function);
        Module.declareSymbol(OtherSymbol, MachineSymbolKind::Function);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Instructions.padOperand(ImmediateOperand(SharedSymbol), 64);
        Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(SharedSymbol)));
        Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(OtherSymbol)));
        Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(SharedSymbol)));

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 56U);
        EXPECT_EQ(readLittleEndian(Executable, 0), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 32), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 40, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 48), 0U);
        ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 2U);
        EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, SharedSymbol);
        EXPECT_EQ(Executable.getDynamicImportSymbols()[1].Name, OtherSymbol);
        const auto& Relocations = Executable.getDynamicImportSymbolRelocations();
        ASSERT_EQ(Relocations.size(), 4U);
        constexpr std::array<DynamicImportSymbolRelocation, 4> ExpectedRelocations{{
            {0, 0, 0},
            {16, 0, 0},
            {32, 1, 0},
            {48, 0, 0},
        }};
        for (std::size_t Index = 0; Index < ExpectedRelocations.size(); ++Index)
        {
            EXPECT_EQ(Relocations[Index].ImageOffset, ExpectedRelocations[Index].ImageOffset);
            EXPECT_EQ(Relocations[Index].SymbolIndex, ExpectedRelocations[Index].SymbolIndex);
            EXPECT_EQ(Relocations[Index].Addend, 0);
        }
        EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Numeric calls preserve complete 64-bit address patterns while immediate jumps retain signed displacement bits.
    TEST(VMExecutableEmitterTest, PreservesNumericDirectCallBoundaryValues)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(static_cast<std::uint64_t>(-16))));
        Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(std::numeric_limits<std::uint64_t>::max())));
        Instructions.padInstruction(MachineJmpiInstruction(ImmediateOperand(std::uint64_t{1} << 63)));

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 48U);
        EXPECT_EQ(readLittleEndian(Executable, 0, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 8), static_cast<std::uint64_t>(-16));
        EXPECT_EQ(readLittleEndian(Executable, 16, 2), 0x003BU);
        EXPECT_EQ(readLittleEndian(Executable, 24), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(readLittleEndian(Executable, 32, 2), 0x003FU);
        EXPECT_EQ(readLittleEndian(Executable, 40), std::uint64_t{1} << 63);
        EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    }

    // An add instruction packs register IDs into its header, emits ordered payload words, and precedes an eight-byte return header.
    TEST(VMExecutableEmitterTest, EncodesOpcodeBeforeOrderedRegisterAndImmediateOperands)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Instructions.padInstruction(MachineAdd64rriInstruction(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::BP, -8), ImmediateOperand(16)));
        Instructions.padInstruction(MachineRetInstruction());

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 40U);
        EXPECT_EQ(readLittleEndian(Executable, 0), 0x000000030001001BULL);
        EXPECT_EQ(readLittleEndian(Executable, 0, 2), 0x001BU);
        EXPECT_EQ(Executable.getImage()[2], static_cast<std::uint8_t>(RegisterId::SP));
        EXPECT_EQ(readLittleEndian(Executable, 8), 0U);
        EXPECT_EQ(Executable.getImage()[3], static_cast<std::uint8_t>(RegisterId::BP));
        EXPECT_EQ(readLittleEndian(Executable, 16), static_cast<std::uint64_t>(-8));
        EXPECT_EQ(readLittleEndian(Executable, 24), 16U);
        EXPECT_EQ(readLittleEndian(Executable, 32, 2), 0x003AU);
        EXPECT_EQ(readLittleEndian(Executable, 32), 0x000000030303003AULL);
    }

    // A 16-bit store preserves its low two immediate bytes and clears the remaining bytes of its aligned payload word.
    TEST(VMExecutableEmitterTest, ZeroExtendsNarrowInstructionImmediateInAlignedWord)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Instructions.padInstruction(MachineStore16iInstruction(AddressOperand(RegisterId::SP, -8), ImmediateOperand(0x1122334455667788)));

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 24U);
        EXPECT_EQ(readLittleEndian(Executable, 0, 2), 0x0008U);
        EXPECT_EQ(Executable.getImage()[2], static_cast<std::uint8_t>(RegisterId::SP));
        EXPECT_EQ(readLittleEndian(Executable, 8), static_cast<std::uint64_t>(-8));
        EXPECT_EQ(readLittleEndian(Executable, 16), 0x7788U);
    }

    // Every forward symbol address in one instruction receives its own image-base relocation and signed addend.
    TEST(VMExecutableEmitterTest, RelocatesEveryForwardMemoryAddressOperand)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto Symbol = core::NamePool::instance().intern("emitter-instruction-forward-global");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(8, 16, {}, Symbol).has_value());
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(5);
        Instructions.padInstruction(MachineLoad64Instruction(AddressOperand(Symbol, 1), AddressOperand(Symbol, 4)));
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 16), 0U);
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 24), 0U);
        Emitter.padGlobals(Module);
        Emitter.resolveStaticRelocations();

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 40U);
        EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x0004U);
        EXPECT_EQ(Executable.getImage()[10], static_cast<std::uint8_t>(RegisterId::ZERO));
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(Executable.getImage()[11], static_cast<std::uint8_t>(RegisterId::ZERO));
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 2U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 16U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, 1);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].ImageOffset, 24U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].TargetOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].Addend, 4);
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    }

    // Numeric immediates retain their low bits at every supported width without creating address relocations.
    TEST(VMExecutableEmitterTest, PreservesNumericImmediateWidths)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        constexpr std::uint64_t Bits = 0x1122334455667788;
        Instructions.padOperand(ImmediateOperand(Bits), 8);
        Instructions.padOperand(ImmediateOperand(Bits), 16);
        Instructions.padOperand(ImmediateOperand(Bits), 32);
        Instructions.padOperand(ImmediateOperand(Bits), 64);

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 32U);
        EXPECT_EQ(readLittleEndian(Executable, 0), 0x88U);
        EXPECT_EQ(readLittleEndian(Executable, 8), 0x7788U);
        EXPECT_EQ(readLittleEndian(Executable, 16), 0x55667788U);
        EXPECT_EQ(readLittleEndian(Executable, 24), Bits);
        EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    }

    // ZERO register values and memory addresses use header register slots and preserve every signed offset bit in their payload words.
    TEST(VMExecutableEmitterTest, EncodesZeroRelativeRegisterAndAddressOperands)
    {
        const core::DiagnosticEngine Diagnostics;
        const std::array<std::int64_t, 5> Offsets{0, 17, -17, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()};
        for (const auto Offset : Offsets)
        {
            SCOPED_TRACE(Offset);
            ExecutableEmitter Emitter(Diagnostics);
            FunctionEmissionContext FunctionContext;
            InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
            Instructions.padInstruction(MachineAdd64mrrInstruction(AddressOperand(RegisterId::ZERO, Offset), RegisterOperand(RegisterId::ZERO, Offset), RegisterOperand(RegisterId::ZERO)));

            const ExecutableFile& Executable = Emitter.getExecutable();
            ASSERT_EQ(Executable.byteSize(), 32U);
            EXPECT_EQ(Executable.getImage()[2], 0x03);
            EXPECT_EQ(Executable.getImage()[3], 0x03);
            EXPECT_EQ(readLittleEndian(Executable, 8), static_cast<std::uint64_t>(Offset));
            EXPECT_EQ(readLittleEndian(Executable, 16), static_cast<std::uint64_t>(Offset));
            EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
            EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        }
    }

    // Already laid-out VM symbols still require image-base binding and preserve signed addends outside the placeholder.
    TEST(VMExecutableEmitterTest, EmitsBaseRelocationsForLaidOutVMSymbols)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto Symbol = core::NamePool::instance().intern("emitter-resolved-global");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(16, 8, {}, Symbol).has_value());
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(3);
        Emitter.padGlobals(Module);
        Instructions.padOperand(ImmediateOperand(Symbol, -3), 64);

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 32U);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 1U);
        const auto& Relocation = Executable.getImageBaseAddressRelocations()[0];
        EXPECT_EQ(Relocation.ImageOffset, 24U);
        EXPECT_EQ(Relocation.TargetOffset, 8U);
        EXPECT_EQ(Relocation.Addend, -3);
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Forward VM references remain pending through global layout and are processed together by explicit finalization.
    TEST(VMExecutableEmitterTest, ResolvesForwardVMAddressReferencesAfterLayoutFinalization)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto Symbol = core::NamePool::instance().intern("emitter-forward-global");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(16, 16, {}, Symbol).has_value());
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(1);
        Instructions.padOperand(ImmediateOperand(Symbol, -1), 64);
        Instructions.padOperand(ImmediateOperand(Symbol, 8), 64);
        Instructions.padOperand(AddressOperand(Symbol, 4), 24);
        EXPECT_TRUE(Emitter.getExecutable().getImageBaseAddressRelocations().empty());
        Emitter.padGlobals(Module);
        EXPECT_TRUE(Emitter.getExecutable().getImageBaseAddressRelocations().empty());
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 24), 0U);
        Emitter.resolveStaticRelocations();

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 48U);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 3U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 8U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, -1);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].ImageOffset, 16U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].TargetOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].Addend, 8);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[2].ImageOffset, 24U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[2].TargetOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[2].Addend, 4);
        EXPECT_EQ(readLittleEndian(Executable, 8), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        Emitter.resolveStaticRelocations();
        EXPECT_EQ(Executable.getImageBaseAddressRelocations().size(), 3U);
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // VM memory addresses produce image-base relocations both before and after their target is laid out.
    TEST(VMExecutableEmitterTest, RelocatesMemoryAddressesBeforeAndAfterLayout)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto Symbol = core::NamePool::instance().intern("emitter-pc-global");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(8, 16, {}, Symbol).has_value());
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(3);
        Instructions.padOperand(AddressOperand(Symbol, -2), 8);
        Emitter.padGlobals(Module);
        Instructions.padOperand(AddressOperand(Symbol, 4), 24);
        EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 8), 0U);
        Emitter.resolveStaticRelocations();

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 32U);
        EXPECT_EQ(readLittleEndian(Executable, 8), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        const auto& Relocations = Executable.getImageBaseAddressRelocations();
        ASSERT_EQ(Relocations.size(), 2U);
        EXPECT_EQ(Relocations[0].ImageOffset, 24U);
        EXPECT_EQ(Relocations[0].TargetOffset, 16U);
        EXPECT_EQ(Relocations[0].Addend, 4);
        EXPECT_EQ(Relocations[1].ImageOffset, 8U);
        EXPECT_EQ(Relocations[1].TargetOffset, 16U);
        EXPECT_EQ(Relocations[1].Addend, -2);
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Image-local and imported pointers use separate loading tables for both immediates and memory-address fields.
    TEST(VMExecutableEmitterTest, KeepsVMAndImportAddressFieldsInSeparateLoadingTables)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto VMSymbol = Names.intern("emitter-mixed-vm");
        const auto ImportSymbol = Names.intern("emitter-mixed-import");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(16, 1, {}, VMSymbol).has_value());
        Module.declareSymbol(ImportSymbol, MachineSymbolKind::GlobalVariable);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.padGlobals(Module);
        Instructions.padOperand(ImmediateOperand(VMSymbol, 8), 64);
        Instructions.padOperand(ImmediateOperand(ImportSymbol, -8), 64);
        Instructions.padOperand(AddressOperand(VMSymbol, 8), 32);
        Instructions.padOperand(AddressOperand(ImportSymbol, 12), 40);

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 48U);
        ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 2U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 16U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 0U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, 8);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].ImageOffset, 32U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].TargetOffset, 0U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].Addend, 8);
        ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, ImportSymbol);
        ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 2U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 24U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex, 0U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].Addend, -8);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].ImageOffset, 40U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].SymbolIndex, 0U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].Addend, 12);
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 32), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 40), 0U);
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Repeated external declarations share one import across ZERO-relative memory addresses and ordinary address immediates.
    TEST(VMExecutableEmitterTest, SharesImportImportsAcrossImmediateAndMemoryAddressFields)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto ImportSymbol = Names.intern("emitter-shared-import");
        MachineModule Module;
        Module.declareSymbol(ImportSymbol, MachineSymbolKind::GlobalVariable);
        Module.declareSymbol(ImportSymbol, MachineSymbolKind::GlobalVariable);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.getContext().padZero(1);
        Instructions.padOperand(AddressOperand(ImportSymbol, 12), 0);
        Instructions.padOperand(ImmediateOperand(ImportSymbol, -4), 64);

        const ExecutableFile& Executable = Emitter.getExecutable();
        ASSERT_EQ(Executable.byteSize(), 24U);
        EXPECT_EQ(readLittleEndian(Executable, 8), 0U);
        EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
        ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, ImportSymbol);
        ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 2U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 8U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex, 0U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].Addend, 12);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].ImageOffset, 16U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].SymbolIndex, 0U);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].Addend, -4);
        EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Starting another module emission discards both loading tables and the previous image instead of leaking old pointer fields.
    TEST(VMExecutableEmitterTest, ClearsAddressRelocationsBeforeEmittingAnotherModule)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto VMSymbol = Names.intern("emitter-reset-vm");
        const auto ImportSymbol = Names.intern("emitter-reset-import");
        MachineModule FirstModule;
        ASSERT_TRUE(FirstModule.createGlobalVariable(8, 1, {}, VMSymbol).has_value());
        FirstModule.declareSymbol(ImportSymbol, MachineSymbolKind::GlobalVariable);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(FirstModule);
        Emitter.padGlobals(FirstModule);
        Instructions.padOperand(ImmediateOperand(VMSymbol), 64);
        Instructions.padOperand(ImmediateOperand(ImportSymbol), 64);
        ASSERT_EQ(Emitter.getExecutable().getImageBaseAddressRelocations().size(), 1U);
        ASSERT_EQ(Emitter.getExecutable().getDynamicImportSymbolRelocations().size(), 1U);

        MachineModule SecondModule;
        const std::array<std::uint8_t, 2> InitialBytes{0xAB, 0xCD};
        ASSERT_TRUE(SecondModule.createGlobalVariable(2, 1, InitialBytes, Names.intern("emitter-reset-next")).has_value());
        const auto EntryName = Names.intern("emitter-reset-next-entry");
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(SecondModule.createFunction(EntryName).createBasicBlock());
        Builder.create<Opcode::Ret>();
        const ExecutableFile Executable = Emitter.emit(SecondModule, EntryName, {});
        ASSERT_EQ(Executable.byteSize(), 16U);
        EXPECT_EQ(Executable.getEntryOffset(), 8U);
        EXPECT_FALSE(Emitter.getContext().isImportSymbol(ImportSymbol));
        EXPECT_EQ(Executable.getImage()[0], 0xABU);
        EXPECT_EQ(Executable.getImage()[1], 0xCDU);
        EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x003AU);
        EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
        EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // Explicit entry selection can skip the first function, and listed local code and data exports preserve caller order regardless of linkage.
    TEST(VMExecutableEmitterTest, SelectsExplicitEntryAndOrderedMixedExports)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto GlobalName = Names.intern("emitter-selected-export-global");
        const auto FirstName = Names.intern("emitter-unlisted-external-function");
        const auto EntryName = Names.intern("emitter-selected-local-entry");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, Names.intern("emitter-selected-entry-prefix")).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, GlobalName).has_value());
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction(FirstName, MachineSymbolLinkage::External).createBasicBlock());
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Module.createFunction(EntryName, MachineSymbolLinkage::Local).createBasicBlock());
        Builder.create<Opcode::Ret>();
        const std::array<core::Name, 2> ExportNames{EntryName, GlobalName};
        ExecutableEmitter Emitter(Diagnostics);
        const ExecutableFile Executable = Emitter.emit(Module, EntryName, ExportNames);

        ASSERT_EQ(Executable.byteSize(), 32U);
        EXPECT_EQ(Executable.getEntryOffset(), 24U);
        EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x003AU);
        ASSERT_EQ(Executable.getExportSymbols().size(), 2U);
        EXPECT_EQ(Executable.getExportSymbols()[0].Name, EntryName);
        EXPECT_EQ(Executable.getExportSymbols()[0].Offset, 24U);
        EXPECT_EQ(Executable.getExportSymbols()[1].Name, GlobalName);
        EXPECT_EQ(Executable.getExportSymbols()[1].Offset, 8U);
        ExecutableFileValidator::validate(Executable, Diagnostics);
    }

    // A reused emitter replaces a nonzero entry and populated exports with an explicitly selected zero-offset entry and an empty export list.
    TEST(VMExecutableEmitterTest, ReplacesEntryAndExportsWhenEmittingAnotherModule)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto GlobalName = Names.intern("emitter-entry-reset-global");
        const auto FirstEntryName = Names.intern("emitter-entry-reset-first");
        const auto SecondEntryName = Names.intern("emitter-entry-reset-second");
        MachineModule FirstModule;
        ASSERT_TRUE(FirstModule.createGlobalVariable(4, 1, {}, GlobalName).has_value());
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(FirstModule.createFunction(FirstEntryName).createBasicBlock());
        Builder.create<Opcode::Ret>();
        const std::array<core::Name, 2> FirstExports{FirstEntryName, GlobalName};
        ExecutableEmitter Emitter(Diagnostics);
        const ExecutableFile First = Emitter.emit(FirstModule, FirstEntryName, FirstExports);
        ASSERT_EQ(First.getEntryOffset(), 8U);
        ASSERT_EQ(First.getExportSymbols().size(), 2U);

        MachineModule SecondModule;
        Builder.setInsertPoint(SecondModule.createFunction(SecondEntryName).createBasicBlock());
        Builder.create<Opcode::Ret>();
        const ExecutableFile Second = Emitter.emit(SecondModule, SecondEntryName, {});

        ASSERT_EQ(Second.byteSize(), 8U);
        EXPECT_EQ(Second.getEntryOffset(), 0U);
        EXPECT_TRUE(Second.getExportSymbols().empty());
        EXPECT_EQ(readLittleEndian(Second, 0, 2), 0x003AU);
        EXPECT_EQ(First.getEntryOffset(), 8U);
        EXPECT_EQ(First.getExportSymbols().size(), 2U);
        ExecutableFileValidator::validate(Second, Diagnostics);
    }

    // An entry must be a registered VM function with instructions, even when an empty function shares its offset with later code.
    TEST(VMExecutableEmitterDeathTest, RejectsInvalidEntrySelections)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto GlobalName = Names.intern("emitter-invalid-entry-global");
        const auto EmptyName = Names.intern("emitter-invalid-entry-empty-function");
        const auto ValidName = Names.intern("emitter-invalid-entry-valid-function");
        const auto ImportName = Names.intern("emitter-invalid-entry-import");
        const auto UnknownName = Names.intern("emitter-invalid-entry-unknown");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, GlobalName).has_value());
        Module.createFunction(EmptyName);
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction(ValidName).createBasicBlock());
        Builder.create<Opcode::Ret>();
        Module.declareSymbol(ImportName, MachineSymbolKind::Function);
        ExecutableEmitter Emitter(Diagnostics);

        for (const auto EntryName : {UnknownName, GlobalName, ImportName})
        {
            SCOPED_TRACE(EntryName.index());
            EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, {})), "INK-V0002.*entry must name a defined VM function");
        }
        EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, core::NAME_None, {})), "INK-V0002.*entry function name must be registered and nonempty");
        EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EmptyName, {})), "INK-V0002.*entry function must contain at least one instruction");
    }

    // Exports reject duplicate or invalid names, missing VM offsets, and a trailing empty function whose offset equals the image size.
    TEST(VMExecutableEmitterDeathTest, RejectsInvalidExportSelections)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto GlobalName = Names.intern("emitter-invalid-export-global");
        const auto EntryName = Names.intern("emitter-invalid-export-entry");
        const auto EmptyName = Names.intern("emitter-invalid-export-trailing-empty-function");
        const auto ImportName = Names.intern("emitter-invalid-export-import");
        const auto UnknownName = Names.intern("emitter-invalid-export-unknown");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, GlobalName).has_value());
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction(EntryName).createBasicBlock());
        Builder.create<Opcode::Ret>();
        Module.createFunction(EmptyName);
        Module.declareSymbol(ImportName, MachineSymbolKind::Function);
        ExecutableEmitter Emitter(Diagnostics);
        const std::array<core::Name, 2> Duplicated{GlobalName, GlobalName};
        EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, Duplicated)), "INK-V0002.*export symbol name is duplicated");
        for (const auto Name : {UnknownName, ImportName})
        {
            SCOPED_TRACE(Name.index());
            const std::array<core::Name, 1> ExportNames{Name};
            EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, ExportNames)), "INK-V0002.*export symbol has no VM image offset");
        }
        const std::array<core::Name, 1> InvalidNames{core::NAME_None};
        EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, InvalidNames)), "INK-V0002.*export symbol name must be registered and nonempty");
        const std::array<core::Name, 1> TrailingEmptyFunction{EmptyName};
        EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, TrailingEmptyFunction)), "INK-V0002.*export symbol offset is outside the image");
    }

    // Direct calls reject byte offsets on function symbols before creating an address relocation.
    TEST(VMExecutableEmitterDeathTest, RejectsNonzeroDirectCallSymbolOffsets)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto Symbol = core::NamePool::instance().intern("emitter-offset-call-target");
        MachineModule Module;
        Module.declareSymbol(Symbol, MachineSymbolKind::Function);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        for (const std::int64_t Offset : {-1, 1})
        {
            SCOPED_TRACE(Offset);
            EXPECT_DEATH(Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(Symbol, Offset))), "INK-V0002.*zero symbol byte offset");
        }
    }

    // Finalization rejects a referenced function whose entry was never assigned an image offset.
    TEST(VMExecutableEmitterDeathTest, RejectsUnlaidOutFunctionTargetsDuringFinalization)
    {
        const core::DiagnosticEngine Diagnostics;
        const auto Symbol = core::NamePool::instance().intern("emitter-unlaid-function");
        MachineModule Module;
        Module.createFunction(Symbol);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Instructions.padOperand(ImmediateOperand(Symbol), 64);
        Emitter.padGlobals(Module);
        EXPECT_DEATH(Emitter.resolveStaticRelocations(), "INK-V0002.*VM symbol has no image offset after layout");
    }

    // Ordinary image-local and imported symbol addresses reject narrow fields even when their image offsets would fit those fields.
    TEST(VMExecutableEmitterDeathTest, RejectsNarrowSymbolAddressImmediateFields)
    {
        const core::DiagnosticEngine Diagnostics;
        core::NamePool& Names = core::NamePool::instance();
        const auto VMSymbol = Names.intern("emitter-narrow-vm");
        const auto ImportSymbol = Names.intern("emitter-narrow-import");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(8, 1, {}, VMSymbol).has_value());
        Module.declareSymbol(ImportSymbol, MachineSymbolKind::GlobalVariable);
        ExecutableEmitter Emitter(Diagnostics);
        FunctionEmissionContext FunctionContext;
        InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
        Emitter.scanUnResolvedSymbol(Module);
        Emitter.padGlobals(Module);
        for (const std::uint32_t Width : {8U, 16U, 32U})
        {
            SCOPED_TRACE(Width);
            EXPECT_DEATH(Instructions.padOperand(ImmediateOperand(VMSymbol), Width), "INK-V0002.*64-bit immediate");
            EXPECT_DEATH(Instructions.padOperand(ImmediateOperand(ImportSymbol), Width), "INK-V0002.*64-bit immediate");
        }
    }
} // namespace ink::vm::test
