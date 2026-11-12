#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  namespace
  {
    core::Name linkName(std::string_view Text)
    {
      return core::NamePool::instance().intern(Text);
    }

    std::unique_ptr<MachineModule> link(std::initializer_list<const MachineModule *> Inputs, core::Name Name = core::NAME_None)
    {
      core::CollectingDiagnosticConsumer Consumer;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Consumer);
      auto Result = ModuleLinker::link(std::span<const MachineModule *const>(Inputs.begin(), Inputs.size()), Diagnostics, Name);
      EXPECT_TRUE(Consumer.diagnostics().empty());
      return Result;
    }

    const MachineInstruction &instructionAt(const MachineBasicBlock &Block, std::size_t Index)
    {
      return *std::next(Block.instructions().begin(), static_cast<std::ptrdiff_t>(Index));
    }

    void expectLinkFailure(std::initializer_list<const MachineModule *> Inputs)
    {
      core::CollectingDiagnosticConsumer Consumer;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Consumer);
      const auto Result = ModuleLinker::link(std::span<const MachineModule *const>(Inputs.begin(), Inputs.size()), Diagnostics);
      ASSERT_EQ(Result, nullptr);
      ASSERT_EQ(Consumer.diagnostics().size(), 1U);
      const auto &Diagnostic = Consumer.diagnostics().front();
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::VMModuleLinkFailed);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_FALSE(Diagnostic.Arguments.empty());
    }

    template <Opcode Operation, typename OperandType>
    OperandType registeredOperand(std::size_t Index, const MachineBasicBlock &Target)
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        if (OpcodeTraits<Operation>::HasDestination && Index == 0)
        {
          return RegisterOperand(Operation == Opcode::Add64rrr ? RegisterId::SP : RegisterId::BP);
        }
        return RegisterOperand(Index % 2 == 0 ? RegisterId::SP : RegisterId::PC, Index % 2 == 0 ? std::numeric_limits<std::int64_t>::max() : std::numeric_limits<std::int64_t>::min());
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        return AddressOperand(RegisterId::SP, -static_cast<std::int64_t>(8 + Index));
      }
      else if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
      {
        return BasicBlockOperand(&Target);
      }
      else
      {
        static_assert(std::is_same_v<OperandType, ImmediateOperand>);
        if constexpr (Operation == Opcode::Calli || Operation == Opcode::CallNativei)
        {
          return ImmediateOperand(linkName(Operation == Opcode::CallNativei ? "link.registered.native" : "link.registered.function"));
        }
        else
        {
          return ImmediateOperand(std::numeric_limits<std::uint64_t>::max() - Index);
        }
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    void appendRegisteredInstruction(MachineInstructionBuilder &Builder, const MachineBasicBlock &Target, std::index_sequence<Indices...>)
    {
      using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
      Builder.create<Operation>(registeredOperand<Operation, std::tuple_element_t<Indices, Tuple>>(Indices, Target)...);
    }

    void expectSameOperand(const Operand &Input, const Operand &Output, const MachineBasicBlock &OutputTarget)
    {
      ASSERT_EQ(Input.getKind(), Output.getKind());
      switch (Input.getKind())
      {
      case OperandKind::Register:
        EXPECT_EQ(static_cast<const RegisterOperand &>(Input).getRegister(), static_cast<const RegisterOperand &>(Output).getRegister());
        EXPECT_EQ(static_cast<const RegisterOperand &>(Input).getByteOffset(), static_cast<const RegisterOperand &>(Output).getByteOffset());
        break;
      case OperandKind::Address:
      {
        const auto &InputAddress = static_cast<const AddressOperand &>(Input);
        const auto &OutputAddress = static_cast<const AddressOperand &>(Output);
        EXPECT_EQ(InputAddress.getBaseKind(), OutputAddress.getBaseKind());
        EXPECT_EQ(InputAddress.getRegister(), OutputAddress.getRegister());
        EXPECT_EQ(InputAddress.getSymbol(), OutputAddress.getSymbol());
        EXPECT_EQ(InputAddress.getByteOffset(), OutputAddress.getByteOffset());
        break;
      }
      case OperandKind::Immediate:
      {
        const auto &InputImmediate = static_cast<const ImmediateOperand &>(Input);
        const auto &OutputImmediate = static_cast<const ImmediateOperand &>(Output);
        EXPECT_EQ(InputImmediate.getValueKind(), OutputImmediate.getValueKind());
        EXPECT_EQ(InputImmediate.bits(), OutputImmediate.bits());
        EXPECT_EQ(InputImmediate.getSymbol(), OutputImmediate.getSymbol());
        EXPECT_EQ(InputImmediate.getByteOffset(), OutputImmediate.getByteOffset());
        break;
      }
      case OperandKind::BasicBlock:
        EXPECT_EQ(static_cast<const BasicBlockOperand &>(Output).getBlock(), &OutputTarget);
        EXPECT_NE(static_cast<const BasicBlockOperand &>(Input).getBlock(), &OutputTarget);
        break;
      }
    }
  } // namespace

  // Linking no inputs produces an independently owned empty module with the requested output name.
  TEST(VMModuleLinkerTest, AcceptsEmptyInputAndPreservesOutputName)
  {
    const auto OutputName = linkName("link.empty");
    const auto Result = link({}, OutputName);
    ASSERT_NE(Result, nullptr);
    EXPECT_EQ(Result->name(), OutputName);
    EXPECT_EQ(Result->functionCount(), 0U);
    EXPECT_EQ(Result->globalVariableCount(), 0U);
    EXPECT_TRUE(Result->symbolDeclarations().empty());
    EXPECT_TRUE(Result->nativeFunctions().empty());
    EXPECT_TRUE(Result->nativeData().empty());
  }

  // Null inputs return an explicit diagnostic instead of dereferencing an invalid module pointer.
  TEST(VMModuleLinkerTest, RejectsNullInput)
  {
    MachineModule Input;
    expectLinkFailure({&Input, nullptr});
    EXPECT_EQ(Input.functionCount(), 0U);
  }

  // Repeating an input pointer is an invalid invocation instead of a request to duplicate its private definitions.
  TEST(VMModuleLinkerTest, RejectsRepeatedInputPointers)
  {
    MachineModule Input;
    Input.createFunction(linkName("link.repeated.input"));
    expectLinkFailure({&Input, &Input});
    EXPECT_EQ(Input.functionCount(), 1U);
  }

  // Cloned code, data and laid-out frames remain usable after all input owners have been destroyed.
  TEST(VMModuleLinkerTest, OwnsCodeDataAndFramesBeyondInputLifetimes)
  {
    std::unique_ptr<MachineModule> Output;
    const auto EntryName = linkName("link.lifetime.entry");
    const auto ConstantName = linkName("link.lifetime.constant");
    const auto GlobalName = linkName("link.lifetime.global");
    {
      MachineModule First;
      MachineModule Second;
      const std::array<std::uint8_t, 3> Bytes{3, 5, 8};
      ASSERT_TRUE(First.createGlobalVariable(Bytes.size(), 16, Bytes, ConstantName, MachineSymbolLinkage::Local, true).has_value());
      ASSERT_TRUE(Second.createGlobalVariable(std::numeric_limits<std::uint64_t>::max(), 8, Bytes, GlobalName).has_value());
      auto &Function = First.createFunction(EntryName, MachineSymbolLinkage::External);
      ASSERT_TRUE(Function.frameInfo().createStackSlot(4, 4).has_value());
      ASSERT_TRUE(Function.frameInfo().createStackSlot(8, 8).has_value());
      ASSERT_TRUE(Function.frameInfo().layout(16).has_value());
      auto &Block = Function.createBasicBlock(linkName("entry"));
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      Builder.create<Opcode::Ret>();
      const auto FunctionAddress = &Function;
      const auto BlockAddress = &Block;
      auto Result = link({&First, &Second});
      ASSERT_NE(Result, nullptr);
      EXPECT_NE(&*Result->functions().begin(), FunctionAddress);
      EXPECT_NE(&*Result->functions().begin()->basicBlocks().begin(), BlockAddress);
      EXPECT_EQ(First.functionCount(), 1U);
      EXPECT_EQ(First.functions().begin()->name(), EntryName);
      EXPECT_EQ(First.functions().begin()->parent(), &First);
      EXPECT_EQ(Block.parent(), &Function);
      Output = std::move(Result);
    }
    ASSERT_NE(Output, nullptr);
    const auto &Function = *Output->functions().begin();
    EXPECT_EQ(Function.name(), EntryName);
    EXPECT_EQ(Function.linkage(), MachineSymbolLinkage::External);
    EXPECT_EQ(Function.parent(), Output.get());
    EXPECT_EQ(Function.frameInfo().stackSlotCount(), 2U);
    EXPECT_EQ(Function.frameInfo().baseOffset(), 16U);
    EXPECT_EQ(Function.frameInfo().byteSize(), 16U);
    EXPECT_EQ(Function.frameInfo().alignment(), 8U);
    EXPECT_EQ(Function.frameInfo().stackSlots().begin()->byteOffset(), 16U);
    EXPECT_EQ(std::next(Function.frameInfo().stackSlots().begin())->byteOffset(), 24U);
    const auto &Block = *Function.basicBlocks().begin();
    EXPECT_EQ(Block.parent(), &Function);
    EXPECT_EQ(instructionAt(Block, 0).parent(), &Block);
    EXPECT_EQ(instructionAt(Block, 0).getOpcode(), Opcode::Ret);
    ASSERT_EQ(Output->globalVariableCount(), 2U);
    const auto &Constant = *Output->globalVariables().begin();
    EXPECT_EQ(Constant.parent(), Output.get());
    EXPECT_EQ(Constant.alignment(), 16U);
    EXPECT_EQ(Constant.byteSize(), 3U);
    EXPECT_TRUE(Constant.isConstant());
    const std::array<std::uint8_t, 3> ExpectedBytes{3, 5, 8};
    EXPECT_TRUE(std::ranges::equal(Constant.initialBytes(), ExpectedBytes));
    const auto &Global = *std::next(Output->globalVariables().begin());
    EXPECT_EQ(Global.parent(), Output.get());
    EXPECT_FALSE(Global.isConstant());
    EXPECT_EQ(Global.byteSize(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Global.alignment(), 8U);
    EXPECT_TRUE(std::ranges::equal(Global.initialBytes(), ExpectedBytes));
    EXPECT_TRUE(writeMachineModuleText(*Output).has_value());
  }

  // Pending frame layouts stay pending rather than acquiring offsets during module linking.
  TEST(VMModuleLinkerTest, PreservesUnlaidOutFramesAndAnonymousDefinitions)
  {
    MachineModule Input;
    auto &Function = Input.createFunction();
    ASSERT_TRUE(Function.frameInfo().createStackSlot(8, 16).has_value());
    ASSERT_TRUE(Input.createGlobalVariable(0, 1, {}, core::NAME_None, MachineSymbolLinkage::Local, true).has_value());
    ASSERT_TRUE(Input.createGlobalVariable(0).has_value());
    const auto Result = link({&Input});
    ASSERT_NE(Result, nullptr);
    const auto &OutputFunction = *Result->functions().begin();
    EXPECT_EQ(OutputFunction.name(), core::NAME_None);
    EXPECT_EQ(OutputFunction.linkage(), MachineSymbolLinkage::Local);
    EXPECT_FALSE(OutputFunction.frameInfo().baseOffset().has_value());
    EXPECT_FALSE(OutputFunction.frameInfo().byteSize().has_value());
    EXPECT_FALSE(OutputFunction.frameInfo().stackSlots().begin()->byteOffset().has_value());
    ASSERT_EQ(Result->globalVariableCount(), 2U);
    EXPECT_EQ(Result->globalVariables().begin()->name(), core::NAME_None);
    EXPECT_TRUE(Result->globalVariables().begin()->isConstant());
    EXPECT_EQ(std::next(Result->globalVariables().begin())->name(), core::NAME_None);
    EXPECT_FALSE(std::next(Result->globalVariables().begin())->isConstant());
  }

  // Forward, backward and self branches target output blocks even when input block names are identical.
  TEST(VMModuleLinkerTest, RemapsBranchesByBlockIdentity)
  {
    MachineModule Input;
    auto &Function = Input.createFunction();
    const auto SharedName = linkName("link.branch.duplicate");
    auto &Entry = Function.createBasicBlock(SharedName);
    auto &Later = Function.createBasicBlock(SharedName);
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry);
    Builder.create<Opcode::Jne>(&Later);
    Builder.create<Opcode::Jmp>(&Entry);
    Builder.setInsertPoint(Later);
    Builder.create<Opcode::Jmp>(&Entry);
    const auto Result = link({&Input});
    ASSERT_NE(Result, nullptr);
    const auto &OutputFunction = *Result->functions().begin();
    const auto &OutputEntry = *OutputFunction.basicBlocks().begin();
    const auto &OutputLater = *std::next(OutputFunction.basicBlocks().begin());
    EXPECT_EQ(static_cast<const MachineJneInstruction &>(instructionAt(OutputEntry, 0)).getTarget(), &OutputLater);
    EXPECT_EQ(static_cast<const MachineJmpInstruction &>(instructionAt(OutputEntry, 1)).getTarget(), &OutputEntry);
    EXPECT_EQ(static_cast<const MachineJmpInstruction &>(instructionAt(OutputLater, 0)).getTarget(), &OutputEntry);
    EXPECT_EQ(static_cast<const MachineJneInstruction &>(instructionAt(Entry, 0)).getTarget(), &Later);
  }

  // Every registered opcode survives linking with its operands, function symbols and output-owned block references intact.
  TEST(VMModuleLinkerTest, ClonesEveryRegisteredOpcode)
  {
    MachineModule Input;
    auto &Block = Input.createFunction().createBasicBlock();
    Input.createFunction(linkName("link.registered.function"), MachineSymbolLinkage::External);
    Input.declareNativeFunction(linkName("link.registered.native"), NativeFunctionImport{linkName("host.registered.native")});
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) appendRegisteredInstruction<Opcode::Kind>(Builder, Block, std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{});
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    const auto Result = link({&Input});
    ASSERT_NE(Result, nullptr);
    const auto &OutputBlock = *Result->functions().begin()->basicBlocks().begin();
    ASSERT_EQ(OutputBlock.instructionCount(), Block.instructionCount());
    auto OutputPosition = OutputBlock.instructions().begin();
    for (const auto &Instruction : Block.instructions())
    {
      SCOPED_TRACE(Instruction.getName());
      const auto &OutputInstruction = *OutputPosition++;
      EXPECT_NE(&OutputInstruction, &Instruction);
      EXPECT_EQ(OutputInstruction.parent(), &OutputBlock);
      EXPECT_EQ(OutputInstruction.getOpcode(), Instruction.getOpcode());
      ASSERT_EQ(OutputInstruction.operandCount(), Instruction.operandCount());
      for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
      {
        expectSameOperand(*Instruction.getOperand(Index), *OutputInstruction.getOperand(Index), OutputBlock);
      }
    }
  }

  // Register destinations require plain BP/SP, while explicit sources reject EFLAGS and undefined identifiers.
  TEST(VMModuleLinkerTest, RejectsInvalidRegisterDestinationsAndSources)
  {
    const std::array<std::pair<RegisterOperand, RegisterOperand>, 8> InvalidOperands = {{
        {RegisterOperand(RegisterId::PC), RegisterOperand(RegisterId::SP, -8)},
        {RegisterOperand(RegisterId::ZERO), RegisterOperand(RegisterId::SP, -8)},
        {RegisterOperand(RegisterId::BP, std::numeric_limits<std::int64_t>::min()), RegisterOperand(RegisterId::PC, 8)},
        {RegisterOperand(RegisterId::SP, std::numeric_limits<std::int64_t>::max()), RegisterOperand(RegisterId::BP, -8)},
        {RegisterOperand(RegisterId::EFLAGS), RegisterOperand(RegisterId::PC, 8)},
        {RegisterOperand(static_cast<RegisterId>(255)), RegisterOperand(RegisterId::SP, -8)},
        {RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::EFLAGS, -8)},
        {RegisterOperand(RegisterId::SP), RegisterOperand(static_cast<RegisterId>(255), 8)},
    }};
    for (const auto &[Destination, Source] : InvalidOperands)
    {
      MachineModule Input;
      auto &Block = Input.createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      const auto &Original = Builder.create<Opcode::Add64rri>(Destination, Source, ImmediateOperand(1));
      expectLinkFailure({&Input});
      EXPECT_EQ(Original.getDestination().getRegister(), Destination.getRegister());
      EXPECT_EQ(Original.getDestination().getByteOffset(), Destination.getByteOffset());
      EXPECT_EQ(Original.getLHS().getRegister(), Source.getRegister());
      EXPECT_EQ(Original.getLHS().getByteOffset(), Source.getByteOffset());
    }
  }

  // ZERO remains a valid read-only source and memory base when linking clones instruction operands.
  TEST(VMModuleLinkerTest, AcceptsZeroRegisterSourcesAndAddressBases)
  {
    MachineModule Input;
    auto &Block = Input.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), ImmediateOperand(1));
    Builder.create<Opcode::Store64m>(AddressOperand(RegisterId::ZERO, 4096), AddressOperand(RegisterId::ZERO, -8));
    Builder.create<Opcode::Cmp64rr>(RegisterOperand(RegisterId::ZERO, 16), RegisterOperand(RegisterId::ZERO));
    const auto Result = link({&Input});
    ASSERT_NE(Result, nullptr);
    const auto &OutputBlock = *Result->functions().begin()->basicBlocks().begin();
    const auto &Add = static_cast<const MachineAdd64rriInstruction &>(instructionAt(OutputBlock, 0));
    EXPECT_EQ(Add.getLHS().getRegister(), RegisterId::ZERO);
    EXPECT_EQ(Add.getLHS().getByteOffset(), 0);
    const auto &Store = static_cast<const MachineStore64mInstruction &>(instructionAt(OutputBlock, 1));
    EXPECT_EQ(Store.getDestination().getRegister(), RegisterId::ZERO);
    EXPECT_EQ(Store.getDestination().getByteOffset(), 4096);
    EXPECT_EQ(Store.getSource().getRegister(), RegisterId::ZERO);
    EXPECT_EQ(Store.getSource().getByteOffset(), -8);
    const auto &Compare = static_cast<const MachineCmp64rrInstruction &>(instructionAt(OutputBlock, 2));
    EXPECT_EQ(Compare.getLHS().getRegister(), RegisterId::ZERO);
    EXPECT_EQ(Compare.getLHS().getByteOffset(), 16);
    EXPECT_EQ(Compare.getRHS().getRegister(), RegisterId::ZERO);
  }

  // Repeated private names remain distinct and both symbol operand forms follow their own module's definition.
  TEST(VMModuleLinkerTest, RenamesLocalDefinitionsAndRewritesBothSymbolOperandForms)
  {
    MachineModule First;
    MachineModule Second;
    const auto Symbol = linkName("link.local.data");
    const std::array<std::uint8_t, 1> Bytes{17};
    ASSERT_TRUE(First.createGlobalVariable(Bytes.size(), 1, Bytes, Symbol, MachineSymbolLinkage::Local, true).has_value());
    ASSERT_TRUE(Second.createGlobalVariable(Bytes.size(), 1, Bytes, Symbol, MachineSymbolLinkage::Local, true).has_value());
    for (auto *Module : {&First, &Second})
    {
      auto &Block = Module->createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      Builder.create<Opcode::Add64mmi>(AddressOperand(RegisterId::BP, 16), AddressOperand(Symbol, std::numeric_limits<std::int64_t>::min()), ImmediateOperand(Symbol, std::numeric_limits<std::int64_t>::max()));
    }
    const auto Result = link({&First, &Second});
    ASSERT_NE(Result, nullptr);
    const auto FirstName = Result->globalVariables().begin()->name();
    const auto SecondName = std::next(Result->globalVariables().begin())->name();
    EXPECT_NE(FirstName, SecondName);
    auto FunctionPosition = Result->functions().begin();
    for (const auto Name : {FirstName, SecondName})
    {
      const auto &Instruction = static_cast<const MachineAdd64mmiInstruction &>(instructionAt(*FunctionPosition++->basicBlocks().begin(), 0));
      EXPECT_EQ(Instruction.getLHS().getSymbol(), Name);
      EXPECT_EQ(Instruction.getRHS().getSymbol(), Name);
      EXPECT_EQ(Instruction.getLHS().getByteOffset(), std::numeric_limits<std::int64_t>::min());
      EXPECT_EQ(Instruction.getRHS().getByteOffset(), std::numeric_limits<std::int64_t>::max());
    }
    EXPECT_EQ(First.globalVariables().begin()->name(), Symbol);
    EXPECT_EQ(Second.globalVariables().begin()->name(), Symbol);
    const auto &Original = static_cast<const MachineAdd64mmiInstruction &>(instructionAt(*Second.functions().begin()->basicBlocks().begin(), 0));
    EXPECT_EQ(Original.getLHS().getSymbol(), Symbol);
    EXPECT_EQ(Original.getRHS().getSymbol(), Symbol);
    const auto Repeated = link({&First, &Second});
    ASSERT_NE(Repeated, nullptr);
    const auto FirstText = writeMachineModuleText(*Result);
    const auto RepeatedText = writeMachineModuleText(*Repeated);
    ASSERT_TRUE(FirstText.has_value());
    ASSERT_TRUE(RepeatedText.has_value());
    EXPECT_EQ(*FirstText, *RepeatedText);
  }

  // External references resolve across inputs with or without an explicit ordinary declaration.
  TEST(VMModuleLinkerTest, ResolvesExternalDefinitionsAndDropsResolvedDeclarations)
  {
    MachineModule Consumer;
    MachineModule Producer;
    const auto ConstantName = linkName("link.external.constant");
    const auto FunctionName = linkName("link.external.function");
    Consumer.declareSymbol(ConstantName, MachineSymbolKind::GlobalVariable, true);
    Consumer.declareSymbol(FunctionName, MachineSymbolKind::Function);
    Producer.createFunction(FunctionName, MachineSymbolLinkage::External);
    ASSERT_TRUE(Producer.createGlobalVariable(0, 1, {}, ConstantName, MachineSymbolLinkage::External, true).has_value());
    auto &Block = Consumer.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(ConstantName, -7));
    const auto Result = link({&Consumer, &Producer});
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Result->symbolDeclarations().empty());
    EXPECT_TRUE(Result->globalVariables().begin()->isConstant());
    EXPECT_EQ(Consumer.symbolDeclarations().size(), 2U);
    EXPECT_TRUE(Consumer.symbolDeclarations()[0].IsConstant);
    const auto &OutputInstruction = static_cast<const MachineStore64iInstruction &>(instructionAt(*Result->functions().begin()->basicBlocks().begin(), 0));
    EXPECT_EQ(OutputInstruction.getSource().getSymbol(), ConstantName);
    EXPECT_EQ(OutputInstruction.getSource().getByteOffset(), -7);
    MachineModule ImplicitConsumer;
    auto &ImplicitBlock = ImplicitConsumer.createFunction().createBasicBlock();
    Builder.setInsertPoint(ImplicitBlock);
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(ConstantName));
    EXPECT_NE(link({&ImplicitConsumer, &Producer}), nullptr);
  }

  // Read-only and writable globals use the same definition kind while preserving their declared mutability after linking.
  TEST(VMModuleLinkerTest, ResolvesMatchingReadOnlyAndWritableGlobalDeclarations)
  {
    for (const bool IsConstant : {false, true})
    {
      SCOPED_TRACE(IsConstant);
      const auto Name = linkName("link.global.readonly.compatible");
      MachineModule Consumer;
      MachineModule Producer;
      Consumer.declareSymbol(Name, MachineSymbolKind::GlobalVariable, IsConstant);
      ASSERT_TRUE(Producer.createGlobalVariable(32, 16, {}, Name, MachineSymbolLinkage::External, IsConstant).has_value());
      const auto Result = link({&Consumer, &Producer});
      ASSERT_NE(Result, nullptr);
      ASSERT_EQ(Result->globalVariableCount(), 1U);
      EXPECT_TRUE(Result->symbolDeclarations().empty());
      const auto &Global = *Result->globalVariables().begin();
      EXPECT_EQ(Global.name(), Name);
      EXPECT_EQ(Global.isConstant(), IsConstant);
      EXPECT_EQ(Global.byteSize(), 32U);
      EXPECT_EQ(Global.linkage(), MachineSymbolLinkage::External);
      EXPECT_EQ(Producer.globalVariables().begin()->isConstant(), IsConstant);
    }
  }

  // Unused global declarations still require exact read-only compatibility with their external definitions in both directions.
  TEST(VMModuleLinkerTest, RejectsUnusedGlobalDeclarationsWithDifferentReadOnlyFlags)
  {
    for (const bool DeclarationIsConstant : {false, true})
    {
      SCOPED_TRACE(DeclarationIsConstant);
      const auto Name = linkName("link.global.readonly.mismatch");
      MachineModule Consumer;
      MachineModule Producer;
      Consumer.declareSymbol(Name, MachineSymbolKind::GlobalVariable, DeclarationIsConstant);
      ASSERT_TRUE(Producer.createGlobalVariable(8, 8, {}, Name, MachineSymbolLinkage::External, !DeclarationIsConstant).has_value());
      expectLinkFailure({&Consumer, &Producer});
      EXPECT_EQ(Consumer.symbolDeclarations()[0].IsConstant, DeclarationIsConstant);
      EXPECT_EQ(Producer.globalVariables().begin()->isConstant(), !DeclarationIsConstant);
    }
  }

  // Function declarations cannot carry the read-only global flag even when a matching external function exists.
  TEST(VMModuleLinkerTest, RejectsReadOnlyFunctionDeclarations)
  {
    const auto Name = linkName("link.function.invalid.readonly");
    MachineModule Consumer;
    MachineModule Producer;
    Consumer.declareSymbol(Name, MachineSymbolKind::Function, true);
    Producer.createFunction(Name, MachineSymbolLinkage::External);
    expectLinkFailure({&Consumer, &Producer});
  }

  // Repeating a global declaration with conflicting read-only flags is invalid even when one declaration matches the definition.
  TEST(VMModuleLinkerTest, RejectsIncompatibleRepeatedGlobalDeclarations)
  {
    const auto Name = linkName("link.global.repeated.readonly");
    MachineModule Input;
    ASSERT_TRUE(Input.createGlobalVariable(0, 1, {}, Name, MachineSymbolLinkage::External, true).has_value());
    Input.declareSymbol(Name, MachineSymbolKind::GlobalVariable, true);
    Input.declareSymbol(Name, MachineSymbolKind::GlobalVariable, false);
    expectLinkFailure({&Input});
  }

  // An ordinary declaration cannot bind another module's private definition or an external symbol of the wrong kind.
  TEST(VMModuleLinkerTest, RejectsPrivateAndWrongKindDeclarationProviders)
  {
    const auto Symbol = linkName("link.declaration.provider");
    MachineModule Consumer;
    Consumer.declareSymbol(Symbol, MachineSymbolKind::Function);
    MachineModule PrivateProvider;
    PrivateProvider.createFunction(Symbol);
    expectLinkFailure({&Consumer, &PrivateProvider});
    MachineModule WrongKindProvider;
    ASSERT_TRUE(WrongKindProvider.createGlobalVariable(0, 1, {}, Symbol, MachineSymbolLinkage::External, true).has_value());
    expectLinkFailure({&Consumer, &WrongKindProvider});
  }

  // Unresolved symbol operands and unused unresolved declarations both produce link diagnostics.
  TEST(VMModuleLinkerTest, RejectsUnresolvedReferencesAndDeclarations)
  {
    const auto Missing = linkName("link.missing");
    MachineModule Declaration;
    Declaration.declareSymbol(Missing, MachineSymbolKind::GlobalVariable);
    expectLinkFailure({&Declaration});
    for (const bool UseAddress : {false, true})
    {
      MachineModule Reference;
      auto &Block = Reference.createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      if (UseAddress)
      {
        Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(Missing));
      }
      else
      {
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(Missing));
      }
      expectLinkFailure({&Reference});
    }
  }

  // Several invalid operands produce only the first error in semantic operand order and leave the source instruction unchanged.
  TEST(VMModuleLinkerTest, ReportsFirstInvalidOperandOnly)
  {
    const std::array<core::Name, 3> Missing{linkName("link.first.error.destination"), linkName("link.first.error.source"), linkName("link.first.error.immediate")};
    for (std::size_t FirstInvalid = 0; FirstInvalid < Missing.size(); ++FirstInvalid)
    {
      SCOPED_TRACE(FirstInvalid);
      MachineModule Input;
      auto &Block = Input.createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      const AddressOperand Destination = FirstInvalid == 0 ? AddressOperand(Missing[0]) : AddressOperand(RegisterId::BP);
      const AddressOperand Source = FirstInvalid <= 1 ? AddressOperand(Missing[1]) : AddressOperand(RegisterId::SP);
      Builder.create<Opcode::Add64mmi>(Destination, Source, ImmediateOperand(Missing[2]));
      core::CollectingDiagnosticConsumer Consumer;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Consumer);
      const std::array<const MachineModule *, 1> Inputs{&Input};
      EXPECT_EQ(ModuleLinker::link(Inputs, Diagnostics), nullptr);
      ASSERT_EQ(Consumer.diagnostics().size(), 1U);
      const auto &Diagnostic = Consumer.diagnostics().front();
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::VMModuleLinkFailed);
      const auto Description = std::ranges::find(Diagnostic.Arguments, core::DiagnosticArgumentName::Description, &core::DiagnosticArgument::Name);
      ASSERT_NE(Description, Diagnostic.Arguments.end());
      const auto &Text = std::get<std::string>(Description->Value);
      EXPECT_NE(Text.find(core::NamePool::instance().text(Missing[FirstInvalid])), std::string::npos);
      for (std::size_t Index = FirstInvalid + 1; Index < Missing.size(); ++Index)
      {
        EXPECT_EQ(Text.find(core::NamePool::instance().text(Missing[Index])), std::string::npos);
      }
      ASSERT_EQ(Block.instructionCount(), 1U);
      const auto &Original = static_cast<const MachineAdd64mmiInstruction &>(instructionAt(Block, 0));
      EXPECT_EQ(Original.getDestination().getSymbol(), Destination.getSymbol());
      EXPECT_EQ(Original.getLHS().getSymbol(), Source.getSymbol());
      EXPECT_EQ(Original.getRHS().getSymbol(), Missing[2]);
    }
  }

  // Named definitions share one namespace within each input and external definitions cannot be repeated across inputs.
  TEST(VMModuleLinkerTest, RejectsDuplicateDefinitionsWithoutChangingInputs)
  {
    const auto Symbol = linkName("link.duplicate");
    MachineModule SameModule;
    SameModule.createFunction(Symbol);
    ASSERT_TRUE(SameModule.createGlobalVariable(8, 8, {}, Symbol).has_value());
    expectLinkFailure({&SameModule});
    EXPECT_EQ(SameModule.functionCount(), 1U);
    EXPECT_EQ(SameModule.globalVariableCount(), 1U);
    EXPECT_EQ(SameModule.functions().begin()->name(), Symbol);
    MachineModule First;
    MachineModule Second;
    First.createFunction(Symbol, MachineSymbolLinkage::External);
    Second.createFunction(Symbol, MachineSymbolLinkage::External);
    expectLinkFailure({&First, &Second});
    EXPECT_EQ(First.functions().begin()->linkage(), MachineSymbolLinkage::External);
    EXPECT_EQ(Second.functions().begin()->name(), Symbol);
  }

  // Every exported definition requires a usable link name even when its declaration has no payload.
  TEST(VMModuleLinkerTest, RejectsAnonymousExternalDefinitions)
  {
    MachineModule Function;
    Function.createFunction(core::NAME_None, MachineSymbolLinkage::External);
    expectLinkFailure({&Function});
    MachineModule Constant;
    ASSERT_TRUE(Constant.createGlobalVariable(0, 1, {}, core::NAME_None, MachineSymbolLinkage::External, true).has_value());
    expectLinkFailure({&Constant});
    MachineModule Global;
    ASSERT_TRUE(Global.createGlobalVariable(0, 1, {}, core::NAME_None, MachineSymbolLinkage::External).has_value());
    expectLinkFailure({&Global});
  }

  // Ordinary declarations can accompany their own exported definition but cannot collide with private or native bindings.
  TEST(VMModuleLinkerTest, ValidatesDeclarationAndDefinitionCollisionsWithinInputs)
  {
    const auto Symbol = linkName("link.same.input.declaration");
    MachineModule Exported;
    Exported.createFunction(Symbol, MachineSymbolLinkage::External);
    Exported.declareSymbol(Symbol, MachineSymbolKind::Function);
    EXPECT_NE(link({&Exported}), nullptr);
    MachineModule Private;
    Private.createFunction(Symbol);
    Private.declareSymbol(Symbol, MachineSymbolKind::Function);
    expectLinkFailure({&Private});
    expectLinkFailure({&Private, &Exported});
    MachineModule Native;
    Native.declareSymbol(Symbol, MachineSymbolKind::Function);
    Native.declareNativeFunction(Symbol, NativeFunctionImport{linkName("host.declaration")});
    expectLinkFailure({&Native});
    MachineModule PrivateNative;
    ASSERT_TRUE(PrivateNative.createGlobalVariable(8, 8, {}, Symbol).has_value());
    PrivateNative.declareNativeData(Symbol, NativeDataImport{linkName("host.private.declaration")});
    expectLinkFailure({&PrivateNative});
  }

  // Invalid names and enum values in link metadata return diagnostics before an output module is published.
  TEST(VMModuleLinkerTest, RejectsInvalidSymbolMetadata)
  {
    MachineModule Declaration;
    Declaration.declareSymbol(core::NAME_None, MachineSymbolKind::Function);
    expectLinkFailure({&Declaration});
    MachineModule Kind;
    Kind.declareSymbol(linkName("link.invalid.kind"), static_cast<MachineSymbolKind>(255));
    expectLinkFailure({&Kind});
    MachineModule Linkage;
    Linkage.createFunction(linkName("link.invalid.linkage"), static_cast<MachineSymbolLinkage>(255));
    expectLinkFailure({&Linkage});
    MachineModule Function;
    Function.declareNativeFunction(linkName("link.invalid.native.function"), NativeFunctionImport{});
    expectLinkFailure({&Function});
    MachineModule Data;
    Data.declareNativeData(linkName("link.invalid.native.data"), NativeDataImport{});
    expectLinkFailure({&Data});
    MachineModule UnnamedFunction;
    UnnamedFunction.declareNativeFunction(core::NAME_None, NativeFunctionImport{linkName("host.invalid.function.alias")});
    expectLinkFailure({&UnnamedFunction});
    MachineModule UnnamedData;
    UnnamedData.declareNativeData(core::NAME_None, NativeDataImport{linkName("host.invalid.data.alias")});
    expectLinkFailure({&UnnamedData});
  }

  // Local definitions may share external spellings while each reference keeps the correct binding.
  TEST(VMModuleLinkerTest, KeepsLocalAndExternalDefinitionsDistinct)
  {
    const auto Symbol = linkName("link.local.and.external");
    MachineModule PrivateModule;
    MachineModule PublicModule;
    ASSERT_TRUE(PrivateModule.createGlobalVariable(8, 8, {}, Symbol).has_value());
    ASSERT_TRUE(PublicModule.createGlobalVariable(8, 8, {}, Symbol, MachineSymbolLinkage::External).has_value());
    auto &Block = PrivateModule.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(Symbol));
    const auto Result = link({&PrivateModule, &PublicModule});
    ASSERT_NE(Result, nullptr);
    const auto &Private = *Result->globalVariables().begin();
    const auto &Public = *std::next(Result->globalVariables().begin());
    EXPECT_NE(Private.name(), Public.name());
    EXPECT_EQ(Private.linkage(), MachineSymbolLinkage::Local);
    EXPECT_EQ(Public.name(), Symbol);
    EXPECT_EQ(Public.linkage(), MachineSymbolLinkage::External);
    const auto &Instruction = static_cast<const MachineStore64iInstruction &>(instructionAt(*Result->functions().begin()->basicBlocks().begin(), 0));
    EXPECT_EQ(Instruction.getSource().getSymbol(), Private.name());
  }

  // Compatible native declarations coalesce by host identity and both VM aliases rewrite to the surviving binding.
  TEST(VMModuleLinkerTest, CoalescesNativeAliasesAndRewritesReferences)
  {
    const auto HostFunction = linkName("host.function");
    const auto HostData = linkName("host.data");
    const std::array<core::Name, 2> Functions{linkName("link.native.function.first"), linkName("link.native.function.second")};
    const std::array<core::Name, 2> Data{linkName("link.native.data.first"), linkName("link.native.data.second")};
    MachineModule First;
    MachineModule Second;
    const std::array<MachineModule *, 2> Modules{&First, &Second};
    for (std::size_t Index = 0; Index < Modules.size(); ++Index)
    {
      auto &Module = *Modules[Index];
      Module.declareNativeFunction(Functions[Index], NativeFunctionImport{HostFunction});
      Module.declareNativeData(Data[Index], NativeDataImport{HostData});
      auto &Block = Module.createFunction().createBasicBlock();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(Functions[Index], 13));
      Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(Data[Index], -11));
    }
    const auto Result = link({&First, &Second});
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->nativeFunctions().size(), 1U);
    ASSERT_EQ(Result->nativeData().size(), 1U);
    const auto &FunctionBinding = Result->nativeFunctions()[0];
    const auto &DataBinding = Result->nativeData()[0];
    EXPECT_EQ(FunctionBinding.Name, Functions[0]);
    EXPECT_EQ(DataBinding.Name, Data[0]);
    EXPECT_EQ(FunctionBinding.Import.Name, HostFunction);
    EXPECT_EQ(DataBinding.Import.Name, HostData);
    for (const auto &Function : Result->functions())
    {
      const auto &Block = *Function.basicBlocks().begin();
      const auto &FunctionReference = static_cast<const MachineStore64iInstruction &>(instructionAt(Block, 0));
      const auto &DataReference = static_cast<const MachineLoad64Instruction &>(instructionAt(Block, 1));
      EXPECT_EQ(FunctionReference.getSource().getSymbol(), FunctionBinding.Name);
      EXPECT_EQ(FunctionReference.getSource().getByteOffset(), 13);
      EXPECT_EQ(DataReference.getSource().getSymbol(), DataBinding.Name);
      EXPECT_EQ(DataReference.getSource().getByteOffset(), -11);
    }
    EXPECT_EQ(First.nativeFunctions()[0].Name, Functions[0]);
    EXPECT_EQ(Second.nativeData()[0].Name, Data[1]);
  }

  // Equal host names merge native function aliases while function and data declarations retain their distinct categories.
  TEST(VMModuleLinkerTest, CoalescesFunctionNamesWhileKeepingFunctionAndDataDeclarationsDistinct)
  {
    const auto Host = linkName("host.shared.spelling");
    MachineModule Input;
    Input.declareNativeFunction(linkName("link.native.shared.first"), NativeFunctionImport{Host});
    Input.declareNativeFunction(linkName("link.native.shared.second"), NativeFunctionImport{Host});
    Input.declareNativeData(linkName("link.native.data"), NativeDataImport{Host});
    const auto Result = link({&Input});
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->nativeFunctions().size(), 1U);
    ASSERT_EQ(Result->nativeData().size(), 1U);
    EXPECT_EQ(Result->nativeFunctions()[0].Import.Name, Host);
    EXPECT_EQ(Result->nativeData()[0].Import.Name, Host);
  }

  // Different host names preserve distinct native function bindings even when declarations arrive from separate modules.
  TEST(VMModuleLinkerTest, PreservesDistinctNativeFunctionHostNames)
  {
    const auto FirstHost = linkName("host.function.first");
    const auto SecondHost = linkName("host.function.second");
    MachineModule First;
    MachineModule Second;
    First.declareNativeFunction(linkName("link.native.distinct.first"), NativeFunctionImport{FirstHost});
    Second.declareNativeFunction(linkName("link.native.distinct.second"), NativeFunctionImport{SecondHost});
    const auto Result = link({&First, &Second});
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->nativeFunctions().size(), 2U);
    EXPECT_EQ(Result->nativeFunctions()[0].Import.Name, FirstHost);
    EXPECT_EQ(Result->nativeFunctions()[1].Import.Name, SecondHost);
  }

  // A native declaration cannot silently change the meaning of a conflicting VM definition or native alias.
  TEST(VMModuleLinkerTest, RejectsConflictingNativeLinkNames)
  {
    const auto Alias = linkName("link.native.conflict");
    MachineModule First;
    MachineModule Second;
    First.declareNativeData(Alias, NativeDataImport{linkName("host.first")});
    Second.declareNativeData(Alias, NativeDataImport{linkName("host.second")});
    expectLinkFailure({&First, &Second});
    MachineModule Definition;
    ASSERT_TRUE(Definition.createGlobalVariable(8, 8, {}, Alias, MachineSymbolLinkage::External).has_value());
    expectLinkFailure({&First, &Definition});
  }

  // Native aliases remain scoped to declaring inputs while other inputs can bind the same spelling to their own Local definition.
  TEST(VMModuleLinkerTest, KeepsNativeAliasesScopedToDeclaringInputs)
  {
    const auto Alias = linkName("link.native.scoped");
    MachineModule Provider;
    Provider.declareNativeData(Alias, NativeDataImport{linkName("host.scoped")});
    MachineModule Consumer;
    auto &Block = Consumer.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP), AddressOperand(Alias));
    expectLinkFailure({&Provider, &Consumer});
    Consumer.declareNativeData(Alias, NativeDataImport{linkName("host.scoped")});
    EXPECT_NE(link({&Provider, &Consumer}), nullptr);
    MachineModule PrivateConsumer;
    ASSERT_TRUE(PrivateConsumer.createGlobalVariable(8, 8, {}, Alias).has_value());
    auto &PrivateBlock = PrivateConsumer.createFunction().createBasicBlock();
    Builder.setInsertPoint(PrivateBlock);
    Builder.create<Opcode::Store64i>(AddressOperand(Alias, -4), ImmediateOperand(Alias, 8));
    const auto Result = link({&Provider, &PrivateConsumer});
    ASSERT_NE(Result, nullptr);
    ASSERT_EQ(Result->nativeData().size(), 1U);
    EXPECT_EQ(Result->nativeData()[0].Name, Alias);
    const auto PrivateName = Result->globalVariables().begin()->name();
    EXPECT_NE(PrivateName, Alias);
    const auto &Reference = static_cast<const MachineStore64iInstruction &>(instructionAt(*Result->functions().begin()->basicBlocks().begin(), 0));
    EXPECT_EQ(Reference.getDestination().getSymbol(), PrivateName);
    EXPECT_EQ(Reference.getDestination().getByteOffset(), -4);
    EXPECT_EQ(Reference.getSource().getSymbol(), PrivateName);
    EXPECT_EQ(Reference.getSource().getByteOffset(), 8);
    const auto &Original = static_cast<const MachineStore64iInstruction &>(instructionAt(PrivateBlock, 0));
    EXPECT_EQ(Original.getDestination().getSymbol(), Alias);
    EXPECT_EQ(Original.getSource().getSymbol(), Alias);
  }
} // namespace ink::vm::test
