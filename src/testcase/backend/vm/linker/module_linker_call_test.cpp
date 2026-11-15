#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace ink::vm::test
{
    namespace
    {
        core::Name callName(std::string_view Text)
        {
            return core::NamePool::instance().intern(Text);
        }

        std::unique_ptr<MachineModule> linkCallModules(std::initializer_list<const MachineModule*> Inputs)
        {
            core::CollectingDiagnosticConsumer Collector;
            core::DiagnosticEngine Diagnostics;
            Diagnostics.addConsumer(Collector);
            auto Output = ModuleLinker::link(std::span<const MachineModule* const>(Inputs.begin(), Inputs.size()), Diagnostics);
            EXPECT_TRUE(Collector.diagnostics().empty());
            return Output;
        }

        void expectCallLinkFailure(std::initializer_list<const MachineModule*> Inputs)
        {
            core::CollectingDiagnosticConsumer Collector;
            core::DiagnosticEngine Diagnostics;
            Diagnostics.addConsumer(Collector);
            EXPECT_EQ(ModuleLinker::link(std::span<const MachineModule* const>(Inputs.begin(), Inputs.size()), Diagnostics), nullptr);
            ASSERT_EQ(Collector.diagnostics().size(), 1U);
            EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
            EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
        }

        const MachineInstruction& callInstructionAt(const MachineFunction& Function, std::size_t Index = 0)
        {
            return *std::next(Function.basicBlocks().begin()->instructions().begin(), static_cast<std::ptrdiff_t>(Index));
        }

    } // namespace

    // Calls resolve forward External definitions across modules while recursive calls retain their target identity.
    TEST(VMModuleLinkerCallTest, ResolvesCrossModuleAndRecursiveVMCalls)
    {
        const auto CallerName = callName("call.cross.caller");
        const auto TargetName = callName("call.cross.target");
        MachineModule Caller;
        MachineModule Callee;
        auto& CallerBlock = Caller.createFunction(CallerName, MachineSymbolLinkage::External).createBasicBlock();
        auto& CalleeBlock = Callee.createFunction(TargetName, MachineSymbolLinkage::External).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(CallerBlock);
        const auto& Original = Builder.create<Opcode::Calli>(ImmediateOperand(TargetName));
        Builder.setInsertPoint(CalleeBlock);
        Builder.create<Opcode::Calli>(ImmediateOperand(TargetName));
        const auto Output = linkCallModules({&Caller, &Callee});
        ASSERT_NE(Output, nullptr);
        ASSERT_EQ(Output->functionCount(), 2U);
        const auto& LinkedCaller = *Output->functions().begin();
        const auto& LinkedCallee = *std::next(Output->functions().begin());
        const auto& LinkedCall = static_cast<const MachineCalliInstruction&>(callInstructionAt(LinkedCaller));
        EXPECT_EQ(LinkedCaller.name(), CallerName);
        EXPECT_EQ(LinkedCallee.name(), TargetName);
        EXPECT_EQ(LinkedCall.getTarget().getSymbol(), LinkedCallee.name());
        EXPECT_NE(&LinkedCall, &Original);
        EXPECT_EQ(LinkedCall.parent(), &*LinkedCaller.basicBlocks().begin());
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(LinkedCallee)).getTarget().getSymbol(), LinkedCallee.name());
        EXPECT_EQ(Original.getTarget().getSymbol(), TargetName);
    }

    // Equal Local function names are renamed independently and each call follows its own module's definition.
    TEST(VMModuleLinkerCallTest, RewritesCallsToRenamedLocalFunctions)
    {
        const auto TargetName = callName("call.local.target");
        MachineModule First;
        MachineModule Second;
        MachineInstructionBuilder Builder;
        for (auto* Input : {&First, &Second})
        {
            auto& Block = Input->createFunction(TargetName).createBasicBlock();
            Builder.setInsertPoint(Block);
            Builder.create<Opcode::Calli>(ImmediateOperand(TargetName));
        }
        const auto Output = linkCallModules({&First, &Second});
        ASSERT_NE(Output, nullptr);
        ASSERT_EQ(Output->functionCount(), 2U);
        EXPECT_NE(Output->functions().begin()->name(), std::next(Output->functions().begin())->name());
        for (const auto& Function : Output->functions())
        {
            EXPECT_NE(Function.name(), TargetName);
            EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(Function)).getTarget().getSymbol(), Function.name());
        }
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(*First.functions().begin())).getTarget().getSymbol(), TargetName);
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(*Second.functions().begin())).getTarget().getSymbol(), TargetName);
    }

    // Repeated unresolved function declarations coalesce and cloned imported calls outlive both source modules.
    TEST(VMModuleLinkerCallTest, CanonicalizesImportedCallsWithIndependentOwnership)
    {
        const auto Name = callName("call.import.shared");
        std::unique_ptr<MachineModule> Output;
        {
            MachineModule First;
            MachineModule Second;
            const std::array<MachineModule*, 2> Inputs{&First, &Second};
            MachineInstructionBuilder Builder;
            for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
            {
                auto& Input = *Inputs[Index];
                Input.declareSymbol(Name, MachineSymbolKind::Function);
                auto& Block = Input.createFunction().createBasicBlock();
                Builder.setInsertPoint(Block);
                Builder.create<Opcode::Calli>(ImmediateOperand(Name));
            }
            Output = linkCallModules({&First, &Second});
            ASSERT_NE(Output, nullptr);
            for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
            {
                const auto& SourceFunction = *Inputs[Index]->functions().begin();
                const auto& LinkedFunction = *std::next(Output->functions().begin(), static_cast<std::ptrdiff_t>(Index));
                EXPECT_NE(&callInstructionAt(SourceFunction), &callInstructionAt(LinkedFunction));
                EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(SourceFunction)).getTarget().getSymbol(), Name);
            }
        }
        ASSERT_NE(Output, nullptr);
        ASSERT_EQ(Output->symbolDeclarations().size(), 1U);
        EXPECT_EQ(Output->symbolDeclarations()[0].Name, Name);
        for (const auto& Function : Output->functions())
        {
            const auto& Call = static_cast<const MachineCalliInstruction&>(callInstructionAt(Function));
            EXPECT_EQ(Call.getTarget().getSymbol(), Name);
            EXPECT_EQ(Call.parent(), &*Function.basicBlocks().begin());
        }
    }

    // Direct calls reject data definitions and declarations while both function sources remain valid.
    TEST(VMModuleLinkerCallTest, RejectsDataCallTargets)
    {
        const auto Target = callName("call.invalid.kind.target");
        for (const bool Defined : {false, true})
        {
            MachineModule Input;
            if (Defined)
            {
                ASSERT_TRUE(Input.createGlobalVariable(8, 8, {}, Target).has_value());
            }
            else
            {
                Input.declareSymbol(Target, MachineSymbolKind::GlobalVariable);
            }
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Input.createFunction().createBasicBlock());
            Builder.create<Opcode::Calli>(ImmediateOperand(Target));
            expectCallLinkFailure({&Input});
        }
    }

    // External fallback resolution rejects a global definition used as a function target.
    TEST(VMModuleLinkerCallTest, RejectsExternalDataCallTargets)
    {
        const auto Target = callName("call.external.invalid.target");
        MachineModule Provider;
        MachineModule Caller;
        ASSERT_TRUE(Provider.createGlobalVariable(8, 8, {}, Target, MachineSymbolLinkage::External).has_value());
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Caller.createFunction().createBasicBlock());
        Builder.create<Opcode::Calli>(ImmediateOperand(Target));
        expectCallLinkFailure({&Provider, &Caller});
    }

    // Direct calls reject absent names and undefined symbols that have no declaration.
    TEST(VMModuleLinkerCallTest, RejectsInvalidAndUndefinedFunctionSymbols)
    {
        for (const auto Target : {core::NAME_None, callName("call.undefined.target")})
        {
            MachineModule Input;
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Input.createFunction().createBasicBlock());
            Builder.create<Opcode::Calli>(ImmediateOperand(Target));
            expectCallLinkFailure({&Input});
        }
    }

    // Unresolved declarations in another input do not authorize calls until the caller declares the same name.
    TEST(VMModuleLinkerCallTest, RequiresImportDeclarationsInReferencingModules)
    {
        const auto Alias = callName("call.import.scoped.name");
        MachineModule Provider;
        MachineModule Caller;
        Provider.declareSymbol(Alias, MachineSymbolKind::Function);
        auto& Block = Caller.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Calli>(ImmediateOperand(Alias));
        expectCallLinkFailure({&Provider, &Caller});
        Caller.declareSymbol(Alias, MachineSymbolKind::Function);
        EXPECT_NE(linkCallModules({&Provider, &Caller}), nullptr);
    }

    // Numeric calls retain full-width address bits while numeric jumps preserve their signed displacements.
    TEST(VMModuleLinkerCallTest, ClonesNumericCallTargetsWithoutPrematureAddressValidation)
    {
        MachineModule Input;
        auto& Block = Input.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Calli>(ImmediateOperand(0));
        Builder.create<Opcode::Calli>(ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
        Builder.create<Opcode::Calli>(ImmediateOperand(0));
        Builder.create<Opcode::Calli>(ImmediateOperand(std::uint64_t{1} << 32));
        Builder.create<Opcode::Jmpi>(ImmediateOperand(0));
        Builder.create<Opcode::Jmpi>(ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
        const auto Output = linkCallModules({&Input});
        ASSERT_NE(Output, nullptr);
        const auto& Function = *Output->functions().begin();
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(Function, 0)).getTarget().bits(), 0U);
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(Function, 1)).getTarget().bits(), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(Function, 2)).getTarget().bits(), 0U);
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(callInstructionAt(Function, 3)).getTarget().bits(), std::uint64_t{1} << 32);
        EXPECT_EQ(static_cast<const MachineJmpiInstruction&>(callInstructionAt(Function, 4)).getTarget().bits(), 0U);
        EXPECT_EQ(static_cast<const MachineJmpiInstruction&>(callInstructionAt(Function, 5)).getTarget().bits(), std::numeric_limits<std::uint64_t>::max());
    }

    // A function declaration may bind to another input's Ink definition without changing the call form.
    TEST(VMModuleLinkerCallTest, ResolvesDeclaredFunctionsFromOtherInputs)
    {
        const auto Name = callName("call.declared.external.function");
        MachineModule Caller;
        MachineModule Provider;
        Caller.declareSymbol(Name, MachineSymbolKind::Function);
        Provider.createFunction(Name, MachineSymbolLinkage::External).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Caller.createFunction().createBasicBlock());
        Builder.create<Opcode::Calli>(ImmediateOperand(Name));
        const auto Output = linkCallModules({&Caller, &Provider});
        ASSERT_NE(Output, nullptr);
        EXPECT_TRUE(Output->symbolDeclarations().empty());
        const auto& Call = static_cast<const MachineCalliInstruction&>(callInstructionAt(*Output->functions().begin()));
        EXPECT_EQ(Call.getTarget().getSymbol(), Name);
        EXPECT_EQ(Call.getOpcode(), Opcode::Calli);
    }

    // Direct symbols must select an exact function entry or imported function and cannot add positive or negative byte offsets.
    TEST(VMModuleLinkerCallTest, RejectsNonzeroDirectSymbolOffsets)
    {
        const auto Target = callName("call.direct.offset.target");
        for (const bool Imported : {false, true})
        {
            for (const std::int64_t Offset : {-1, 1})
            {
                SCOPED_TRACE(Imported);
                SCOPED_TRACE(Offset);
                MachineModule Input;
                if (Imported)
                {
                    Input.declareSymbol(Target, MachineSymbolKind::Function);
                }
                else
                {
                    Input.createFunction(Target);
                }
                auto& Block = Input.createFunction().createBasicBlock();
                MachineInstructionBuilder Builder;
                Builder.setInsertPoint(Block);
                Builder.create<Opcode::Calli>(ImmediateOperand(Target, Offset));
                expectCallLinkFailure({&Input});
            }
        }
    }

    // Indirect calls clone stack addresses and data-slot symbols without confusing the slot's name with its stored function target.
    TEST(VMModuleLinkerCallTest, ClonesMemoryCallsThroughStackAndDataSlots)
    {
        const auto SlotName = callName("call.memory.local.slot");
        const auto NativeSlotAlias = callName("call.memory.native.slot");
        MachineModule Input;
        ASSERT_TRUE(Input.createGlobalVariable(16, 8, {}, SlotName).has_value());
        Input.declareSymbol(NativeSlotAlias, MachineSymbolKind::GlobalVariable);
        auto& Block = Input.createFunction().createBasicBlock();
        const std::array<AddressOperand, 4> Targets{AddressOperand(RegisterId::SP), AddressOperand(RegisterId::BP, 24), AddressOperand(SlotName, 8), AddressOperand(NativeSlotAlias, -8)};
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        for (const auto& Target : Targets)
        {
            Builder.create<Opcode::Callm>(Target);
        }
        const auto Output = linkCallModules({&Input});
        ASSERT_NE(Output, nullptr);
        ASSERT_EQ(Output->globalVariableCount(), 1U);
        const auto LinkedSlotName = Output->globalVariables().begin()->name();
        EXPECT_NE(LinkedSlotName, SlotName);
        const auto& Function = *Output->functions().begin();
        for (std::size_t Index = 0; Index < Targets.size(); ++Index)
        {
            const auto& Instruction = callInstructionAt(Function, Index);
            EXPECT_EQ(Instruction.getOpcode(), Opcode::Callm);
            ASSERT_NE(Instruction.getOperand(0), nullptr);
            const auto& Target = static_cast<const AddressOperand&>(*Instruction.getOperand(0));
            EXPECT_EQ(Target.getRegister(), Targets[Index].getRegister());
            EXPECT_EQ(Target.getByteOffset(), Targets[Index].getByteOffset());
            EXPECT_EQ(Target.getSymbol(), Index == 2 ? std::optional{LinkedSlotName} : Targets[Index].getSymbol());
        }
        const auto& OriginalTarget = static_cast<const MachineCallmInstruction&>(callInstructionAt(*Input.functions().begin(), 2)).getTarget();
        EXPECT_EQ(OriginalTarget.getSymbol(), SlotName);
        EXPECT_EQ(OriginalTarget.getByteOffset(), 8);
    }

    // Indirect call addresses still reject unresolved storage symbols and invalid address registers for the unified call family.
    TEST(VMModuleLinkerCallTest, RejectsInvalidMemoryCallAddresses)
    {
        const std::array<AddressOperand, 3> Targets{AddressOperand(callName("call.memory.undefined.slot")), AddressOperand(core::NAME_None), AddressOperand(static_cast<RegisterId>(255))};
        for (const auto& Target : Targets)
        {
            MachineModule Input;
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Input.createFunction().createBasicBlock());
            Builder.create<Opcode::Callm>(Target);
            expectCallLinkFailure({&Input});
        }
    }

    // Immediate jumps resolve VM functions across modules while register jumps preserve their address-value operands.
    TEST(VMModuleLinkerJumpTest, ResolvesFunctionEntriesAndClonesRegisterJumpTargets)
    {
        const auto TargetName = callName("jump.external.function");
        MachineModule Caller;
        MachineModule Callee;
        auto& CallerBlock = Caller.createFunction(callName("jump.external.caller"), MachineSymbolLinkage::External).createBasicBlock();
        Callee.createFunction(TargetName, MachineSymbolLinkage::External).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(CallerBlock);
        Builder.create<Opcode::Jmpi>(ImmediateOperand(TargetName));
        Builder.create<Opcode::Jmpr>(RegisterOperand(RegisterId::PC, -8));

        const auto Output = linkCallModules({&Caller, &Callee});

        ASSERT_NE(Output, nullptr);
        ASSERT_EQ(Output->functionCount(), 2U);
        const auto& Function = *Output->functions().begin();
        EXPECT_EQ(static_cast<const MachineJmpiInstruction&>(callInstructionAt(Function, 0)).getTarget().getSymbol(), TargetName);
        const auto& RegisterTarget = static_cast<const MachineJmprInstruction&>(callInstructionAt(Function, 1)).getTarget();
        EXPECT_EQ(RegisterTarget.getRegister(), RegisterId::PC);
        EXPECT_EQ(RegisterTarget.getByteOffset(), -8);
    }

    // Immediate jump symbols reject VM data, native declarations, missing definitions and nonzero function addends.
    TEST(VMModuleLinkerJumpTest, RejectsNonFunctionAndDisplacedImmediateJumpSymbols)
    {
        for (unsigned Scenario = 0; Scenario != 5; ++Scenario)
        {
            SCOPED_TRACE(Scenario);
            const auto Target = callName("jump.invalid.function.target");
            MachineModule Module;
            if (Scenario == 0)
            {
                ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, Target).has_value());
            }
            else if (Scenario == 1)
            {
                Module.declareSymbol(Target, MachineSymbolKind::Function);
            }
            else if (Scenario == 2)
            {
                Module.declareSymbol(Target, MachineSymbolKind::GlobalVariable);
            }
            else if (Scenario == 3)
            {
                Module.createFunction(Target);
            }
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Module.createFunction().createBasicBlock());
            Builder.create<Opcode::Jmpi>(ImmediateOperand(Target, Scenario == 3 ? 1 : 0));
            expectCallLinkFailure({&Module});
        }
    }

    // Memory jumps bind the referenced pointer slot as data and rewrite Local names without requiring the slot to be a function.
    TEST(VMModuleLinkerJumpTest, RewritesMemoryJumpSlotSymbolsWithSignedOffsets)
    {
        const auto LocalSlot = callName("jump.memory.local.slot");
        const auto NativeSlot = callName("jump.memory.native.slot");
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(16, 8, {}, LocalSlot).has_value());
        Module.declareSymbol(NativeSlot, MachineSymbolKind::GlobalVariable);
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction().createBasicBlock());
        Builder.create<Opcode::Jmpm>(AddressOperand(LocalSlot, 8));
        Builder.create<Opcode::Jmpm>(AddressOperand(NativeSlot, -8));

        const auto Output = linkCallModules({&Module});

        ASSERT_NE(Output, nullptr);
        ASSERT_EQ(Output->globalVariableCount(), 1U);
        const auto LinkedSlot = Output->globalVariables().begin()->name();
        EXPECT_NE(LinkedSlot, LocalSlot);
        const auto& Function = *Output->functions().begin();
        const auto& LocalTarget = static_cast<const MachineJmpmInstruction&>(callInstructionAt(Function, 0)).getTarget();
        const auto& NativeTarget = static_cast<const MachineJmpmInstruction&>(callInstructionAt(Function, 1)).getTarget();
        EXPECT_EQ(LocalTarget.getSymbol(), LinkedSlot);
        EXPECT_EQ(LocalTarget.getByteOffset(), 8);
        EXPECT_EQ(NativeTarget.getSymbol(), NativeSlot);
        EXPECT_EQ(NativeTarget.getByteOffset(), -8);
        EXPECT_EQ(static_cast<const MachineJmpmInstruction&>(callInstructionAt(*Module.functions().begin())).getTarget().getSymbol(), LocalSlot);
    }
} // namespace ink::vm::test
