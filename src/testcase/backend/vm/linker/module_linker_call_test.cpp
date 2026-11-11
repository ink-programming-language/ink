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

    std::unique_ptr<MachineModule> linkCallModules(std::initializer_list<const MachineModule *> Inputs)
    {
      core::CollectingDiagnosticConsumer Collector;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Collector);
      auto Output = ModuleLinker::link(std::span<const MachineModule *const>(Inputs.begin(), Inputs.size()), Diagnostics);
      EXPECT_TRUE(Collector.diagnostics().empty());
      return Output;
    }

    void expectCallLinkFailure(std::initializer_list<const MachineModule *> Inputs)
    {
      core::CollectingDiagnosticConsumer Collector;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Collector);
      EXPECT_EQ(ModuleLinker::link(std::span<const MachineModule *const>(Inputs.begin(), Inputs.size()), Diagnostics), nullptr);
      ASSERT_EQ(Collector.diagnostics().size(), 1U);
      EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
      EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
    }

    const MachineInstruction &callInstructionAt(const MachineFunction &Function, std::size_t Index = 0)
    {
      return *std::next(Function.basicBlocks().begin()->instructions().begin(), static_cast<std::ptrdiff_t>(Index));
    }

    void appendCall(MachineInstructionBuilder &Builder, bool Native, core::Name Target)
    {
      if (Native)
      {
        Builder.create<Opcode::CallNativei>(ImmediateOperand(Target));
      }
      else
      {
        Builder.create<Opcode::Calli>(ImmediateOperand(Target));
      }
    }

    void appendMemoryCall(MachineInstructionBuilder &Builder, bool Native, const AddressOperand &Target)
    {
      if (Native)
      {
        Builder.create<Opcode::CallNativem>(Target);
      }
      else
      {
        Builder.create<Opcode::Callm>(Target);
      }
    }
  } // namespace

  // Calls resolve forward External definitions across modules while recursive calls retain their target identity.
  TEST(VMModuleLinkerCallTest, ResolvesCrossModuleAndRecursiveVMCalls)
  {
    const auto CallerName = callName("call.cross.caller");
    const auto TargetName = callName("call.cross.target");
    MachineModule Caller;
    MachineModule Callee;
    auto &CallerBlock = Caller.createFunction(CallerName, MachineSymbolLinkage::External).createBasicBlock();
    auto &CalleeBlock = Callee.createFunction(TargetName, MachineSymbolLinkage::External).createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(CallerBlock);
    const auto &Original = Builder.create<Opcode::Calli>(ImmediateOperand(TargetName));
    Builder.setInsertPoint(CalleeBlock);
    Builder.create<Opcode::Calli>(ImmediateOperand(TargetName));
    const auto Output = linkCallModules({&Caller, &Callee});
    ASSERT_NE(Output, nullptr);
    ASSERT_EQ(Output->functionCount(), 2U);
    const auto &LinkedCaller = *Output->functions().begin();
    const auto &LinkedCallee = *std::next(Output->functions().begin());
    const auto &LinkedCall = static_cast<const MachineCalliInstruction &>(callInstructionAt(LinkedCaller));
    EXPECT_EQ(LinkedCaller.name(), CallerName);
    EXPECT_EQ(LinkedCallee.name(), TargetName);
    EXPECT_EQ(LinkedCall.getTarget().getSymbol(), LinkedCallee.name());
    EXPECT_NE(&LinkedCall, &Original);
    EXPECT_EQ(LinkedCall.parent(), &*LinkedCaller.basicBlocks().begin());
    EXPECT_EQ(static_cast<const MachineCalliInstruction &>(callInstructionAt(LinkedCallee)).getTarget().getSymbol(), LinkedCallee.name());
    EXPECT_EQ(Original.getTarget().getSymbol(), TargetName);
  }

  // Equal Local function names are renamed independently and each call follows its own module's definition.
  TEST(VMModuleLinkerCallTest, RewritesCallsToRenamedLocalFunctions)
  {
    const auto TargetName = callName("call.local.target");
    MachineModule First;
    MachineModule Second;
    MachineInstructionBuilder Builder;
    for (auto *Input : {&First, &Second})
    {
      auto &Block = Input->createFunction(TargetName).createBasicBlock();
      Builder.setInsertPoint(Block);
      Builder.create<Opcode::Calli>(ImmediateOperand(TargetName));
    }
    const auto Output = linkCallModules({&First, &Second});
    ASSERT_NE(Output, nullptr);
    ASSERT_EQ(Output->functionCount(), 2U);
    EXPECT_NE(Output->functions().begin()->name(), std::next(Output->functions().begin())->name());
    for (const auto &Function : Output->functions())
    {
      EXPECT_NE(Function.name(), TargetName);
      EXPECT_EQ(static_cast<const MachineCalliInstruction &>(callInstructionAt(Function)).getTarget().getSymbol(), Function.name());
    }
    EXPECT_EQ(static_cast<const MachineCalliInstruction &>(callInstructionAt(*First.functions().begin())).getTarget().getSymbol(), TargetName);
    EXPECT_EQ(static_cast<const MachineCalliInstruction &>(callInstructionAt(*Second.functions().begin())).getTarget().getSymbol(), TargetName);
  }

  // Native aliases collapse to the first host binding and cloned calls remain valid after both source modules die.
  TEST(VMModuleLinkerCallTest, CanonicalizesNativeCallsWithIndependentOwnership)
  {
    const std::array<core::Name, 2> Aliases{callName("call.native.first"), callName("call.native.second")};
    const auto Host = callName("host.call.native.shared");
    std::unique_ptr<MachineModule> Output;
    {
      MachineModule First;
      MachineModule Second;
      const std::array<MachineModule *, 2> Inputs{&First, &Second};
      MachineInstructionBuilder Builder;
      for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
      {
        auto &Input = *Inputs[Index];
        Input.declareNativeFunction(Aliases[Index], NativeFunctionImport{Host});
        auto &Block = Input.createFunction().createBasicBlock();
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::CallNativei>(ImmediateOperand(Aliases[Index]));
      }
      Output = linkCallModules({&First, &Second});
      ASSERT_NE(Output, nullptr);
      for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
      {
        const auto &SourceFunction = *Inputs[Index]->functions().begin();
        const auto &LinkedFunction = *std::next(Output->functions().begin(), static_cast<std::ptrdiff_t>(Index));
        EXPECT_NE(&callInstructionAt(SourceFunction), &callInstructionAt(LinkedFunction));
        EXPECT_EQ(static_cast<const MachineCallNativeiInstruction &>(callInstructionAt(SourceFunction)).getTarget().getSymbol(), Aliases[Index]);
      }
    }
    ASSERT_NE(Output, nullptr);
    ASSERT_EQ(Output->nativeFunctions().size(), 1U);
    EXPECT_EQ(Output->nativeFunctions()[0].Name, Aliases[0]);
    EXPECT_EQ(Output->nativeFunctions()[0].Import.Name, Host);
    for (const auto &Function : Output->functions())
    {
      const auto &Call = static_cast<const MachineCallNativeiInstruction &>(callInstructionAt(Function));
      EXPECT_EQ(Call.getTarget().getSymbol(), Aliases[0]);
      EXPECT_EQ(Call.parent(), &*Function.basicBlocks().begin());
    }
  }

  // Calls reject data targets and the opposite function category instead of silently changing their calling contract.
  TEST(VMModuleLinkerCallTest, RejectsIncompatibleCallTargetKinds)
  {
    const auto Target = callName("call.invalid.kind.target");
    for (const bool Native : {false, true})
    {
      for (unsigned Kind = 0; Kind != 4; ++Kind)
      {
        if ((!Native && Kind == 0) || (Native && Kind == 1))
        {
          continue;
        }
        SCOPED_TRACE(Native);
        SCOPED_TRACE(Kind);
        MachineModule Input;
        if (Kind == 0)
        {
          Input.createFunction(Target);
        }
        else if (Kind == 1)
        {
          Input.declareNativeFunction(Target, NativeFunctionImport{Target});
        }
        else if (Kind == 2)
        {
          ASSERT_TRUE(Input.createGlobalVariable(8, 8, {}, Target).has_value());
        }
        else
        {
          Input.declareNativeData(Target, NativeDataImport{Target});
        }
        auto &Block = Input.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        appendCall(Builder, Native, Target);
        expectCallLinkFailure({&Input});
        EXPECT_EQ(Block.instructionCount(), 1U);
      }
    }
  }

  // External fallback resolution still validates the opcode's function category instead of accepting any named definition.
  TEST(VMModuleLinkerCallTest, RejectsIncompatibleExternalCallTargets)
  {
    const auto Target = callName("call.external.invalid.target");
    for (const bool Native : {false, true})
    {
      SCOPED_TRACE(Native);
      MachineModule Provider;
      MachineModule Caller;
      if (Native)
      {
        Provider.createFunction(Target, MachineSymbolLinkage::External);
      }
      else
      {
        ASSERT_TRUE(Provider.createGlobalVariable(8, 8, {}, Target, MachineSymbolLinkage::External).has_value());
      }
      auto &Block = Caller.createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      appendCall(Builder, Native, Target);
      expectCallLinkFailure({&Provider, &Caller});
    }
  }

  // Direct symbol calls reject absent names and registered-but-undefined symbols for both call opcodes.
  TEST(VMModuleLinkerCallTest, RejectsInvalidAndUndefinedFunctionSymbols)
  {
    for (const bool Native : {false, true})
    {
      for (const auto Target : {core::NAME_None, callName("call.undefined.target")})
      {
        SCOPED_TRACE(Native);
        MachineModule Input;
        auto &Block = Input.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        appendCall(Builder, Native, Target);
        expectCallLinkFailure({&Input});
      }
    }
  }

  // Native declarations in another input do not authorize callNative until the caller declares the same host import.
  TEST(VMModuleLinkerCallTest, RequiresNativeDeclarationsInReferencingModules)
  {
    const auto Alias = callName("call.native.scoped.alias");
    const NativeFunctionImport Import{callName("host.call.native.scoped")};
    MachineModule Provider;
    MachineModule Caller;
    Provider.declareNativeFunction(Alias, Import);
    auto &Block = Caller.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::CallNativei>(ImmediateOperand(Alias));
    expectCallLinkFailure({&Provider, &Caller});
    Caller.declareNativeFunction(Alias, Import);
    EXPECT_NE(linkCallModules({&Provider, &Caller}), nullptr);
  }

  // Numeric calls preserve all signed PC-relative bits and the full u32 native-index range without requiring symbols.
  TEST(VMModuleLinkerCallTest, ClonesNumericCallTargetsWithoutPrematureAddressValidation)
  {
    MachineModule Input;
    auto &Block = Input.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Calli>(ImmediateOperand(0));
    Builder.create<Opcode::Calli>(ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
    Builder.create<Opcode::CallNativei>(ImmediateOperand(0));
    Builder.create<Opcode::CallNativei>(ImmediateOperand(std::numeric_limits<std::uint32_t>::max()));
    const auto Output = linkCallModules({&Input});
    ASSERT_NE(Output, nullptr);
    const auto &Function = *Output->functions().begin();
    EXPECT_EQ(static_cast<const MachineCalliInstruction &>(callInstructionAt(Function, 0)).getTarget().bits(), 0U);
    EXPECT_EQ(static_cast<const MachineCalliInstruction &>(callInstructionAt(Function, 1)).getTarget().bits(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(static_cast<const MachineCallNativeiInstruction &>(callInstructionAt(Function, 2)).getTarget().bits(), 0U);
    EXPECT_EQ(static_cast<const MachineCallNativeiInstruction &>(callInstructionAt(Function, 3)).getTarget().bits(), std::numeric_limits<std::uint32_t>::max());
  }

  // Native immediate indices that require more than four bytes fail linking instead of being silently truncated.
  TEST(VMModuleLinkerCallTest, RejectsNativeImmediateIndexOverflow)
  {
    const std::array<std::uint64_t, 2> Targets{std::uint64_t{std::numeric_limits<std::uint32_t>::max()} + 1, std::numeric_limits<std::uint64_t>::max()};
    for (const auto Target : Targets)
    {
      SCOPED_TRACE(Target);
      MachineModule Input;
      auto &Block = Input.createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      Builder.create<Opcode::CallNativei>(ImmediateOperand(Target));
      expectCallLinkFailure({&Input});
    }
  }

  // Direct symbols must select an exact function entry or native import and cannot add positive or negative byte offsets.
  TEST(VMModuleLinkerCallTest, RejectsNonzeroDirectSymbolOffsets)
  {
    const auto Target = callName("call.direct.offset.target");
    for (const bool Native : {false, true})
    {
      for (const std::int64_t Offset : {-1, 1})
      {
        SCOPED_TRACE(Native);
        SCOPED_TRACE(Offset);
        MachineModule Input;
        if (Native)
        {
          Input.declareNativeFunction(Target, NativeFunctionImport{Target});
        }
        else
        {
          Input.createFunction(Target);
        }
        auto &Block = Input.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        if (Native)
        {
          Builder.create<Opcode::CallNativei>(ImmediateOperand(Target, Offset));
        }
        else
        {
          Builder.create<Opcode::Calli>(ImmediateOperand(Target, Offset));
        }
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
    Input.declareNativeData(NativeSlotAlias, NativeDataImport{callName("host.call.memory.slot")});
    auto &Block = Input.createFunction().createBasicBlock();
    const std::array<AddressOperand, 4> Targets{AddressOperand(RegisterId::SP), AddressOperand(RegisterId::BP, 24), AddressOperand(SlotName, 8), AddressOperand(NativeSlotAlias, -8)};
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    for (const bool Native : {false, true})
    {
      for (const auto &Target : Targets)
      {
        appendMemoryCall(Builder, Native, Target);
      }
    }
    const auto Output = linkCallModules({&Input});
    ASSERT_NE(Output, nullptr);
    ASSERT_EQ(Output->globalVariableCount(), 1U);
    const auto LinkedSlotName = Output->globalVariables().begin()->name();
    EXPECT_NE(LinkedSlotName, SlotName);
    const auto &Function = *Output->functions().begin();
    for (std::size_t Native = 0; Native != 2; ++Native)
    {
      for (std::size_t Index = 0; Index < Targets.size(); ++Index)
      {
        const auto &Instruction = callInstructionAt(Function, Native * Targets.size() + Index);
        EXPECT_EQ(Instruction.getOpcode(), Native ? Opcode::CallNativem : Opcode::Callm);
        ASSERT_NE(Instruction.getOperand(0), nullptr);
        const auto &Target = static_cast<const AddressOperand &>(*Instruction.getOperand(0));
        EXPECT_EQ(Target.getRegister(), Targets[Index].getRegister());
        EXPECT_EQ(Target.getByteOffset(), Targets[Index].getByteOffset());
        EXPECT_EQ(Target.getSymbol(), Index == 2 ? std::optional{LinkedSlotName} : Targets[Index].getSymbol());
      }
    }
    const auto &OriginalTarget = static_cast<const MachineCallmInstruction &>(callInstructionAt(*Input.functions().begin(), 2)).getTarget();
    EXPECT_EQ(OriginalTarget.getSymbol(), SlotName);
    EXPECT_EQ(OriginalTarget.getByteOffset(), 8);
  }

  // Indirect call addresses still reject unresolved storage symbols and invalid address registers for both call families.
  TEST(VMModuleLinkerCallTest, RejectsInvalidMemoryCallAddresses)
  {
    const std::array<AddressOperand, 3> Targets{AddressOperand(callName("call.memory.undefined.slot")), AddressOperand(core::NAME_None), AddressOperand(static_cast<RegisterId>(255))};
    for (const bool Native : {false, true})
    {
      for (const auto &Target : Targets)
      {
        SCOPED_TRACE(Native);
        MachineModule Input;
        auto &Block = Input.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        appendMemoryCall(Builder, Native, Target);
        expectCallLinkFailure({&Input});
      }
    }
  }
} // namespace ink::vm::test
