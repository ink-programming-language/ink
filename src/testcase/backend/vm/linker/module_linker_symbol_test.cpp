#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <variant>

namespace ink::vm::test
{
    namespace
    {
        core::Name symbolName(std::string_view Text)
        {
            return core::NamePool::instance().intern(Text);
        }

        const MachineInstruction& symbolInstructionAt(const MachineBasicBlock& Block, std::size_t Index)
        {
            return *std::next(Block.instructions().begin(), static_cast<std::ptrdiff_t>(Index));
        }
    } // namespace

    // The same unresolved name cannot identify a function in one input and data in another.
    TEST(VMModuleLinkerSymbolTest, RejectsSameUnresolvedNameAcrossDifferentKinds)
    {
        const auto Alias = symbolName("symbol.native.alias");
        MachineModule First;
        MachineModule Second;
        First.declareSymbol(Alias, MachineSymbolKind::Function);
        Second.declareSymbol(Alias, MachineSymbolKind::GlobalVariable);
        const std::array<const MachineModule*, 2> Inputs{&First, &Second};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        const auto Result = ModuleLinker::link(Inputs, Diagnostics);
        ASSERT_EQ(Result, nullptr);
        ASSERT_EQ(Collector.diagnostics().size(), 1U);
        EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
        EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
    }

    // A generated private name cannot make another module's originally unresolved reference resolve successfully.
    TEST(VMModuleLinkerSymbolTest, DoesNotCaptureUnresolvedReferencesWithGeneratedLocalNames)
    {
        MachineModule Provider;
        MachineModule Consumer;
        ASSERT_TRUE(Provider.createGlobalVariable(0, 1, {}, symbolName("symbol.private.data"), MachineSymbolLinkage::Local, true).has_value());
        auto& Block = Consumer.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(symbolName(".__ink_link_local_0")));
        const std::array<const MachineModule*, 2> Inputs{&Provider, &Consumer};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        const auto Result = ModuleLinker::link(Inputs, Diagnostics);
        ASSERT_EQ(Result, nullptr);
        ASSERT_EQ(Collector.diagnostics().size(), 1U);
        EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
        EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
    }

    // Undefined symbols and invalid registers compete in semantic operand order instead of separate validation passes.
    TEST(VMModuleLinkerSymbolTest, ReportsMixedOperandFailuresInSemanticOrder)
    {
        const auto Missing = symbolName("symbol.mixed.failure.missing");
        for (const bool SymbolFirst : {false, true})
        {
            SCOPED_TRACE(SymbolFirst);
            MachineModule Input;
            auto& Block = Input.createFunction().createBasicBlock();
            const AddressOperand Symbol(Missing);
            const AddressOperand Register(static_cast<RegisterId>(255));
            const AddressOperand Destination = SymbolFirst ? Symbol : Register;
            const AddressOperand Source = SymbolFirst ? Register : Symbol;
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Block);
            Builder.create<Opcode::Add64mmi>(Destination, Source, ImmediateOperand(1));
            const std::array<const MachineModule*, 1> Inputs{&Input};
            core::CollectingDiagnosticConsumer Collector;
            core::DiagnosticEngine Diagnostics;
            Diagnostics.addConsumer(Collector);
            EXPECT_EQ(ModuleLinker::link(Inputs, Diagnostics), nullptr);
            ASSERT_EQ(Collector.diagnostics().size(), 1U);
            const auto& Diagnostic = Collector.diagnostics().front();
            EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::VMModuleLinkFailed);
            const auto Description = std::ranges::find(Diagnostic.Arguments, core::DiagnosticArgumentName::Description, &core::DiagnosticArgument::Name);
            ASSERT_NE(Description, Diagnostic.Arguments.end());
            const auto& Text = std::get<std::string>(Description->Value);
            if (SymbolFirst)
            {
                EXPECT_NE(Text.find(core::NamePool::instance().text(Missing)), std::string::npos);
                EXPECT_EQ(Text.find("invalid base register"), std::string::npos);
            }
            else
            {
                EXPECT_NE(Text.find("invalid base register"), std::string::npos);
                EXPECT_EQ(Text.find(core::NamePool::instance().text(Missing)), std::string::npos);
            }
            ASSERT_EQ(Block.instructionCount(), 1U);
            const auto& Original = static_cast<const MachineAdd64mmiInstruction&>(symbolInstructionAt(Block, 0));
            EXPECT_EQ(Original.getDestination().getSymbol(), Destination.getSymbol());
            EXPECT_EQ(Original.getDestination().getRegister(), Destination.getRegister());
            EXPECT_EQ(Original.getLHS().getSymbol(), Source.getSymbol());
            EXPECT_EQ(Original.getLHS().getRegister(), Source.getRegister());
        }
    }

    // Duplicate definitions are diagnosed before invalid operands even when malformed code occurs in an earlier input.
    TEST(VMModuleLinkerSymbolTest, ReportsDefinitionConflictsBeforeMalformedCode)
    {
        const auto Name = symbolName("symbol.phase.failure.duplicate");
        MachineModule First;
        MachineModule Second;
        auto& Block = First.createFunction(Name, MachineSymbolLinkage::External).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(static_cast<RegisterId>(255)));
        Second.createFunction(Name, MachineSymbolLinkage::External);
        const std::array<const MachineModule*, 2> Inputs{&First, &Second};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        EXPECT_EQ(ModuleLinker::link(Inputs, Diagnostics), nullptr);
        ASSERT_EQ(Collector.diagnostics().size(), 1U);
        const auto& Diagnostic = Collector.diagnostics().front();
        EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::VMModuleLinkFailed);
        const auto Description = std::ranges::find(Diagnostic.Arguments, core::DiagnosticArgumentName::Description, &core::DiagnosticArgument::Name);
        ASSERT_NE(Description, Diagnostic.Arguments.end());
        const auto& Text = std::get<std::string>(Description->Value);
        EXPECT_NE(Text.find("duplicate External definition"), std::string::npos);
        EXPECT_NE(Text.find(core::NamePool::instance().text(Name)), std::string::npos);
        EXPECT_EQ(Text.find("invalid base register"), std::string::npos);
    }

    // Multiple inputs preserve node order, anonymous nodes, per-input Local naming and first-seen unresolved declarations.
    TEST(VMModuleLinkerSymbolTest, PreservesInputNodeAndCanonicalImportOrder)
    {
        const std::array<core::Name, 2> GlobalNames{symbolName("symbol.order.first.global"), symbolName("symbol.order.second.global")};
        const std::array<core::Name, 2> FunctionNames{symbolName("symbol.order.first.function"), symbolName("symbol.order.second.function")};
        const auto SharedFunctionName = symbolName(".__ink_link_local_0");
        const auto SharedDataName = symbolName("symbol.order.native.data");
        const std::array<core::Name, 2> UniqueFunctionAliases{symbolName("symbol.order.first.unique.function"), symbolName("symbol.order.second.unique.function")};
        MachineModule First;
        MachineModule Second;
        const std::array<MachineModule*, 2> Modules{&First, &Second};
        for (std::size_t Index = 0; Index < Modules.size(); ++Index)
        {
            auto& Module = *Modules[Index];
            const std::size_t Size = (Index + 1) * 8;
            ASSERT_TRUE(Module.createGlobalVariable(Size, 8, {}, GlobalNames[Index]).has_value());
            ASSERT_TRUE(Module.createGlobalVariable(Size + 1, 1).has_value());
            auto& Block = Module.createFunction(FunctionNames[Index]).createBasicBlock();
            Module.createFunction();
            Module.declareSymbol(SharedFunctionName, MachineSymbolKind::Function);
            Module.declareSymbol(UniqueFunctionAliases[Index], MachineSymbolKind::Function);
            Module.declareSymbol(SharedDataName, MachineSymbolKind::GlobalVariable);
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Block);
            Builder.create<Opcode::Store64i>(AddressOperand(SharedDataName), ImmediateOperand(SharedFunctionName));
        }
        const std::array<const MachineModule*, 2> Inputs{&First, &Second};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        const auto Result = ModuleLinker::link(Inputs, Diagnostics);
        EXPECT_TRUE(Collector.diagnostics().empty());
        ASSERT_NE(Result, nullptr);
        ASSERT_EQ(Result->globalVariableCount(), 4U);
        ASSERT_EQ(Result->functionCount(), 4U);
        const std::array<core::Name, 4> ExpectedGlobalNames{symbolName(".__ink_link_local_1"), core::NAME_None, symbolName(".__ink_link_local_3"), core::NAME_None};
        const std::array<std::size_t, 4> ExpectedSizes{8, 9, 16, 17};
        std::size_t Index = 0;
        for (const auto& Global : Result->globalVariables())
        {
            EXPECT_EQ(Global.name(), ExpectedGlobalNames[Index]);
            EXPECT_EQ(Global.byteSize(), ExpectedSizes[Index]);
            ++Index;
        }
        const std::array<core::Name, 4> ExpectedFunctionNames{symbolName(".__ink_link_local_2"), core::NAME_None, symbolName(".__ink_link_local_4"), core::NAME_None};
        Index = 0;
        for (const auto& Function : Result->functions())
        {
            EXPECT_EQ(Function.name(), ExpectedFunctionNames[Index]);
            if (Index % 2 == 0)
            {
                ASSERT_EQ(Function.basicBlockCount(), 1U);
                const auto& Instruction = static_cast<const MachineStore64iInstruction&>(symbolInstructionAt(*Function.basicBlocks().begin(), 0));
                EXPECT_EQ(Instruction.getDestination().getSymbol(), SharedDataName);
                EXPECT_EQ(Instruction.getSource().getSymbol(), SharedFunctionName);
            }
            else
            {
                EXPECT_EQ(Function.basicBlockCount(), 0U);
            }
            ++Index;
        }
        ASSERT_EQ(Result->symbolDeclarations().size(), 4U);
        EXPECT_EQ(Result->symbolDeclarations()[0].Name, SharedFunctionName);
        EXPECT_EQ(Result->symbolDeclarations()[0].Kind, MachineSymbolKind::Function);
        EXPECT_EQ(Result->symbolDeclarations()[1].Name, UniqueFunctionAliases[0]);
        EXPECT_EQ(Result->symbolDeclarations()[2].Name, SharedDataName);
        EXPECT_EQ(Result->symbolDeclarations()[2].Kind, MachineSymbolKind::GlobalVariable);
        EXPECT_EQ(Result->symbolDeclarations()[3].Name, UniqueFunctionAliases[1]);
    }

    // Relinking preserves an unresolved import beside another input's Local definition and independently renames private bindings.
    TEST(VMModuleLinkerSymbolTest, RelinksOutputWithoutCapturingLocalOrNativeBindings)
    {
        const auto OriginalLocal = symbolName("symbol.relink.private");
        const auto NativeAlias = symbolName(".__ink_link_local_0");
        MachineModule First;
        ASSERT_TRUE(First.createGlobalVariable(0, 1, {}, OriginalLocal, MachineSymbolLinkage::Local, true).has_value());
        First.declareSymbol(NativeAlias, MachineSymbolKind::GlobalVariable);
        auto& FirstBlock = First.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(FirstBlock);
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(OriginalLocal, 3));
        Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(NativeAlias, 5));
        const std::array<const MachineModule*, 1> FirstInputs{&First};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        const auto FirstResult = ModuleLinker::link(FirstInputs, Diagnostics);
        EXPECT_TRUE(Collector.diagnostics().empty());
        ASSERT_NE(FirstResult, nullptr);
        const auto LinkedLocal = FirstResult->globalVariables().begin()->name();

        MachineModule Additional;
        ASSERT_TRUE(Additional.createGlobalVariable(0, 1, {}, LinkedLocal, MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Additional.createGlobalVariable(8, 8, {}, NativeAlias).has_value());
        Additional.declareSymbol(NativeAlias, MachineSymbolKind::GlobalVariable);
        auto& AdditionalBlock = Additional.createFunction().createBasicBlock();
        Builder.setInsertPoint(AdditionalBlock);
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(LinkedLocal, 7));
        Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(NativeAlias, 11));
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(NativeAlias, 13));
        const std::array<const MachineModule*, 2> Inputs{FirstResult.get(), &Additional};
        const auto Result = ModuleLinker::link(Inputs, Diagnostics);
        EXPECT_TRUE(Collector.diagnostics().empty());
        ASSERT_NE(Result, nullptr);
        ASSERT_EQ(Result->globalVariableCount(), 3U);
        EXPECT_TRUE(Result->globalVariables().begin()->isConstant());
        EXPECT_TRUE(std::next(Result->globalVariables().begin())->isConstant());
        EXPECT_FALSE(std::next(Result->globalVariables().begin(), 2)->isConstant());
        ASSERT_EQ(Result->symbolDeclarations().size(), 1U);
        const auto FirstLocalName = Result->globalVariables().begin()->name();
        const auto AdditionalLocalName = std::next(Result->globalVariables().begin())->name();
        const auto AdditionalGlobalName = std::next(Result->globalVariables().begin(), 2)->name();
        const auto OutputNativeAlias = Result->symbolDeclarations()[0].Name;
        EXPECT_NE(FirstLocalName, AdditionalLocalName);
        EXPECT_NE(AdditionalGlobalName, OutputNativeAlias);

        const auto& OutputFirstBlock = *Result->functions().begin()->basicBlocks().begin();
        const auto& FirstLocalReference = static_cast<const MachineStore64iInstruction&>(symbolInstructionAt(OutputFirstBlock, 0));
        const auto& FirstNativeReference = static_cast<const MachineLoad64Instruction&>(symbolInstructionAt(OutputFirstBlock, 1));
        EXPECT_EQ(FirstLocalReference.getSource().getSymbol(), FirstLocalName);
        EXPECT_EQ(FirstLocalReference.getSource().getByteOffset(), 3);
        EXPECT_EQ(FirstNativeReference.getSource().getSymbol(), OutputNativeAlias);
        EXPECT_EQ(FirstNativeReference.getSource().getByteOffset(), 5);

        const auto& OutputAdditionalBlock = *std::next(Result->functions().begin())->basicBlocks().begin();
        const auto& AdditionalLocalReference = static_cast<const MachineStore64iInstruction&>(symbolInstructionAt(OutputAdditionalBlock, 0));
        const auto& AdditionalNativeReference = static_cast<const MachineLoad64Instruction&>(symbolInstructionAt(OutputAdditionalBlock, 1));
        const auto& AdditionalGlobalReference = static_cast<const MachineStore64iInstruction&>(symbolInstructionAt(OutputAdditionalBlock, 2));
        EXPECT_EQ(AdditionalLocalReference.getSource().getSymbol(), AdditionalLocalName);
        EXPECT_EQ(AdditionalLocalReference.getSource().getByteOffset(), 7);
        EXPECT_EQ(AdditionalNativeReference.getSource().getSymbol(), AdditionalGlobalName);
        EXPECT_EQ(AdditionalNativeReference.getSource().getByteOffset(), 11);
        EXPECT_EQ(AdditionalGlobalReference.getSource().getSymbol(), AdditionalGlobalName);
        EXPECT_EQ(AdditionalGlobalReference.getSource().getByteOffset(), 13);
    }

    // Global data relocations bind per-module Local definitions, cross-module External symbols, and declared imports while preserving addends.
    TEST(VMModuleLinkerSymbolTest, LinksAndRemapsGlobalDataRelocationsAcrossLocalExternalAndImportSymbols)
    {
        const auto SharedLocalData = symbolName("reloc.link.local.data");
        const auto SharedLocalFn = symbolName("reloc.link.local.fn");
        const auto FirstTable = symbolName("reloc.link.first.table");
        const auto SecondExternalData = symbolName("reloc.link.second.external");
        const auto SecondPointer = symbolName("reloc.link.second.ptr");
        const auto ExternalImport = symbolName("reloc.link.external.import");

        MachineModule First;
        First.declareSymbol(ExternalImport, MachineSymbolKind::GlobalVariable, true);
        ASSERT_TRUE(First.createGlobalVariable(16, 8, {}, SharedLocalData).has_value());
        const std::array<MachineGlobalRelocation, 5> FirstRelocs{{
            {0, SharedLocalData, 8},
            {8, SharedLocalFn, 0},
            {16, FirstTable, 0},
            {24, SecondExternalData, -8},
            {32, ExternalImport, 16},
        }};
        ASSERT_TRUE(First.createGlobalVariable(40, 8, {}, FirstTable, MachineSymbolLinkage::Local, true, FirstRelocs).has_value());
        First.createFunction(SharedLocalFn, MachineSymbolLinkage::Local);

        MachineModule Second;
        ASSERT_TRUE(Second.createGlobalVariable(8, 8, {}, SharedLocalData).has_value());
        ASSERT_TRUE(Second.createGlobalVariable(8, 8, {}, SecondExternalData, MachineSymbolLinkage::External).has_value());
        const std::array<MachineGlobalRelocation, 2> SecondRelocs{{
            {0, SharedLocalData, 0},
            {8, SecondExternalData, 4},
        }};
        ASSERT_TRUE(Second.createGlobalVariable(16, 8, {}, SecondPointer, MachineSymbolLinkage::External, false, SecondRelocs).has_value());
        Second.createFunction(SharedLocalFn, MachineSymbolLinkage::Local);

        const std::array<const MachineModule*, 2> Inputs{&First, &Second};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        const auto Linked = ModuleLinker::link(Inputs, Diagnostics);
        EXPECT_TRUE(Collector.diagnostics().empty());
        ASSERT_NE(Linked, nullptr);
        ASSERT_EQ(Linked->globalVariableCount(), 5U);
        ASSERT_EQ(Linked->functionCount(), 2U);
        ASSERT_EQ(Linked->symbolDeclarations().size(), 1U);
        EXPECT_EQ(Linked->symbolDeclarations()[0].Name, ExternalImport);
        EXPECT_TRUE(Linked->symbolDeclarations()[0].IsConstant);

        auto GlobalIt = Linked->globalVariables().begin();
        const auto& OutFirstLocalData = *GlobalIt++;
        const auto& OutFirstTable = *GlobalIt++;
        const auto& OutSecondLocalData = *GlobalIt++;
        const auto& OutSecondExternalData = *GlobalIt++;
        const auto& OutSecondPointer = *GlobalIt;

        const auto& OutFirstLocalFn = *Linked->functions().begin();
        const auto& OutSecondLocalFn = *std::next(Linked->functions().begin());

        EXPECT_NE(OutFirstLocalData.name(), OutSecondLocalData.name());
        EXPECT_NE(OutFirstLocalFn.name(), OutSecondLocalFn.name());
        EXPECT_EQ(OutSecondExternalData.name(), SecondExternalData);
        EXPECT_EQ(OutSecondPointer.name(), SecondPointer);

        ASSERT_EQ(OutFirstTable.relocationCount(), 5U);
        EXPECT_EQ(OutFirstTable.relocations()[0], (MachineGlobalRelocation{0, OutFirstLocalData.name(), 8}));
        EXPECT_EQ(OutFirstTable.relocations()[1], (MachineGlobalRelocation{8, OutFirstLocalFn.name(), 0}));
        EXPECT_EQ(OutFirstTable.relocations()[2], (MachineGlobalRelocation{16, OutFirstTable.name(), 0}));
        EXPECT_EQ(OutFirstTable.relocations()[3], (MachineGlobalRelocation{24, SecondExternalData, -8}));
        EXPECT_EQ(OutFirstTable.relocations()[4], (MachineGlobalRelocation{32, ExternalImport, 16}));

        ASSERT_EQ(OutSecondPointer.relocationCount(), 2U);
        EXPECT_EQ(OutSecondPointer.relocations()[0], (MachineGlobalRelocation{0, OutSecondLocalData.name(), 0}));
        EXPECT_EQ(OutSecondPointer.relocations()[1], (MachineGlobalRelocation{8, SecondExternalData, 4}));

        // Source inputs remain unchanged after deep cloning.
        const auto& SourceFirstTable = *std::next(First.globalVariables().begin());
        EXPECT_EQ(SourceFirstTable.relocations()[0].Symbol, SharedLocalData);
        EXPECT_EQ(SourceFirstTable.relocations()[2].Symbol, FirstTable);
    }

    // Global data relocations reject undeclared symbols and cannot be captured by generated Local names from another input.
    TEST(VMModuleLinkerSymbolTest, RejectsUndefinedSymbolReferencesInGlobalRelocations)
    {
        MachineModule Provider;
        ASSERT_TRUE(Provider.createGlobalVariable(8, 8, {}, symbolName("reloc.link.provider.private"), MachineSymbolLinkage::Local).has_value());
        MachineModule Consumer;
        const std::array<MachineGlobalRelocation, 1> MissingReloc{{{0, symbolName(".__ink_link_local_0"), 0}}};
        ASSERT_TRUE(Consumer.createGlobalVariable(8, 8, {}, symbolName("reloc.link.consumer.bad"), MachineSymbolLinkage::External, false, MissingReloc).has_value());

        const std::array<const MachineModule*, 2> Inputs{&Provider, &Consumer};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        EXPECT_EQ(ModuleLinker::link(Inputs, Diagnostics), nullptr);
        ASSERT_EQ(Collector.diagnostics().size(), 1U);
        EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
        EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
    }

    // Narrow-width (< 64-bit) instructions cannot carry symbolic immediates during module linking.
    TEST(VMModuleLinkerSymbolTest, RejectsNarrowSymbolicImmediateInstructions)
    {
        const auto Target = symbolName("symbol.narrow.imm.target");
        MachineModule Input;
        ASSERT_TRUE(Input.createGlobalVariable(8, 8, {}, Target, MachineSymbolLinkage::External).has_value());
        auto& Block = Input.createFunction(symbolName("symbol.narrow.imm.fn"), MachineSymbolLinkage::External).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(Target));
        const std::array<const MachineModule*, 1> Inputs{&Input};
        core::CollectingDiagnosticConsumer Collector;
        core::DiagnosticEngine Diagnostics;
        Diagnostics.addConsumer(Collector);
        EXPECT_EQ(ModuleLinker::link(Inputs, Diagnostics), nullptr);
        ASSERT_EQ(Collector.diagnostics().size(), 1U);
        EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
        EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
        const auto Description = std::ranges::find(Collector.diagnostics()[0].Arguments, core::DiagnosticArgumentName::Description, &core::DiagnosticArgument::Name);
        ASSERT_NE(Description, Collector.diagnostics()[0].Arguments.end());
        EXPECT_NE(std::get<std::string>(Description->Value).find("64-bit"), std::string::npos);
    }
} // namespace ink::vm::test
