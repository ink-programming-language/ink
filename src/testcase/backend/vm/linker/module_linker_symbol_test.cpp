#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string_view>

namespace ink::vm::test
{
  namespace
  {
    core::Name symbolName(std::string_view Text)
    {
      return core::NamePool::instance().intern(Text);
    }

    const MachineInstruction &symbolInstructionAt(const MachineBasicBlock &Block, std::size_t Index)
    {
      return *std::next(Block.instructions().begin(), static_cast<std::ptrdiff_t>(Index));
    }
  } // namespace

  // A shared VM alias cannot identify two host function providers even when their lookup names are equal.
  TEST(VMModuleLinkerSymbolTest, RejectsSameNativeFunctionAliasAcrossDifferentProviders)
  {
    const auto Alias = symbolName("symbol.native.alias");
    const auto HostName = symbolName("symbol.host.function");
    MachineModule First;
    MachineModule Second;
    First.declareNativeFunction(Alias, NativeFunctionImport{NativeSymbolProvider::Native, HostName, NativeCallKind::C});
    Second.declareNativeFunction(Alias, NativeFunctionImport{NativeSymbolProvider::SystemApi, HostName, NativeCallKind::C});
    const std::array<const MachineModule *, 2> Inputs{&First, &Second};
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
    auto &Block = Consumer.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(symbolName(".__ink_link_local_0")));
    const std::array<const MachineModule *, 2> Inputs{&Provider, &Consumer};
    core::CollectingDiagnosticConsumer Collector;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Collector);
    const auto Result = ModuleLinker::link(Inputs, Diagnostics);
    ASSERT_EQ(Result, nullptr);
    ASSERT_EQ(Collector.diagnostics().size(), 1U);
    EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMModuleLinkFailed);
    EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
  }

  // Relinking a linked module preserves private bindings when new private definitions collide with its names and native alias.
  TEST(VMModuleLinkerSymbolTest, RelinksOutputWithoutCapturingLocalOrNativeBindings)
  {
    const auto OriginalLocal = symbolName("symbol.relink.private");
    const auto NativeAlias = symbolName(".__ink_link_local_0");
    const auto HostName = symbolName("symbol.relink.host.data");
    MachineModule First;
    ASSERT_TRUE(First.createGlobalVariable(0, 1, {}, OriginalLocal, MachineSymbolLinkage::Local, true).has_value());
    First.declareNativeData(NativeAlias, NativeDataImport{HostName});
    auto &FirstBlock = First.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(FirstBlock);
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(OriginalLocal, 3));
    Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(NativeAlias, 5));
    const std::array<const MachineModule *, 1> FirstInputs{&First};
    core::CollectingDiagnosticConsumer Collector;
    core::DiagnosticEngine Diagnostics;
    Diagnostics.addConsumer(Collector);
    const auto FirstResult = ModuleLinker::link(FirstInputs, Diagnostics);
    EXPECT_TRUE(Collector.diagnostics().empty());
    ASSERT_NE(FirstResult, nullptr);
    const auto LinkedLocal = FirstResult->globalVariables().begin()->name();

    MachineModule Additional;
    const auto AdditionalAlias = symbolName("symbol.relink.additional.alias");
    ASSERT_TRUE(Additional.createGlobalVariable(0, 1, {}, LinkedLocal, MachineSymbolLinkage::Local, true).has_value());
    ASSERT_TRUE(Additional.createGlobalVariable(8, 8, {}, NativeAlias).has_value());
    Additional.declareNativeData(AdditionalAlias, NativeDataImport{HostName});
    auto &AdditionalBlock = Additional.createFunction().createBasicBlock();
    Builder.setInsertPoint(AdditionalBlock);
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(LinkedLocal, 7));
    Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(AdditionalAlias, 11));
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(NativeAlias, 13));
    const std::array<const MachineModule *, 2> Inputs{FirstResult.get(), &Additional};
    const auto Result = ModuleLinker::link(Inputs, Diagnostics);
    EXPECT_TRUE(Collector.diagnostics().empty());
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->globalVariableCount(), 3U);
    EXPECT_TRUE(Result->globalVariables().begin()->isConstant());
    EXPECT_TRUE(std::next(Result->globalVariables().begin())->isConstant());
    EXPECT_FALSE(std::next(Result->globalVariables().begin(), 2)->isConstant());
    ASSERT_EQ(Result->nativeData().size(), 1U);
    const auto FirstLocalName = Result->globalVariables().begin()->name();
    const auto AdditionalLocalName = std::next(Result->globalVariables().begin())->name();
    const auto AdditionalGlobalName = std::next(Result->globalVariables().begin(), 2)->name();
    const auto OutputNativeAlias = Result->nativeData()[0].Name;
    EXPECT_NE(FirstLocalName, AdditionalLocalName);
    EXPECT_NE(AdditionalGlobalName, OutputNativeAlias);

    const auto &OutputFirstBlock = *Result->functions().begin()->basicBlocks().begin();
    const auto &FirstLocalReference = static_cast<const MachineStore64iInstruction &>(symbolInstructionAt(OutputFirstBlock, 0));
    const auto &FirstNativeReference = static_cast<const MachineLoad64Instruction &>(symbolInstructionAt(OutputFirstBlock, 1));
    EXPECT_EQ(FirstLocalReference.getSource().getSymbol(), FirstLocalName);
    EXPECT_EQ(FirstLocalReference.getSource().getByteOffset(), 3);
    EXPECT_EQ(FirstNativeReference.getSource().getSymbol(), OutputNativeAlias);
    EXPECT_EQ(FirstNativeReference.getSource().getByteOffset(), 5);

    const auto &OutputAdditionalBlock = *std::next(Result->functions().begin())->basicBlocks().begin();
    const auto &AdditionalLocalReference = static_cast<const MachineStore64iInstruction &>(symbolInstructionAt(OutputAdditionalBlock, 0));
    const auto &AdditionalNativeReference = static_cast<const MachineLoad64Instruction &>(symbolInstructionAt(OutputAdditionalBlock, 1));
    const auto &AdditionalGlobalReference = static_cast<const MachineStore64iInstruction &>(symbolInstructionAt(OutputAdditionalBlock, 2));
    EXPECT_EQ(AdditionalLocalReference.getSource().getSymbol(), AdditionalLocalName);
    EXPECT_EQ(AdditionalLocalReference.getSource().getByteOffset(), 7);
    EXPECT_EQ(AdditionalNativeReference.getSource().getSymbol(), OutputNativeAlias);
    EXPECT_EQ(AdditionalNativeReference.getSource().getByteOffset(), 11);
    EXPECT_EQ(AdditionalGlobalReference.getSource().getSymbol(), AdditionalGlobalName);
    EXPECT_EQ(AdditionalGlobalReference.getSource().getByteOffset(), 13);
  }
} // namespace ink::vm::test
