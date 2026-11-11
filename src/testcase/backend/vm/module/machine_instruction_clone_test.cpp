#include "ink/backend/vm/instruction/machine_instruction_visitor.h"
#include "ink/backend/vm/module/machine_instruction_clone.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace ink::vm::test
{
  // Value and void visitors receive the concrete const instruction once through a base reference.
  TEST(VMMachineInstructionVisitorTest, DispatchesConcreteConstTypesWithValueAndVoidResults)
  {
    const MachineStore64iInstruction Source(AddressOperand(RegisterId::BP, 8), ImmediateOperand(42));
    const MachineInstruction &Base = Source;
    std::size_t Calls = 0;
    const auto ReadValue = [&](const auto &Concrete) -> std::uint64_t
    {
      using InstructionType = std::remove_cvref_t<decltype(Concrete)>;
      static_assert(std::is_const_v<std::remove_reference_t<decltype(Concrete)>>);
      ++Calls;
      if constexpr (std::is_same_v<InstructionType, MachineStore64iInstruction>)
      {
        EXPECT_EQ(&Concrete, &Source);
        return Concrete.getSource().bits().value_or(0);
      }
      return 0;
    };
    const auto Result = visitInstruction(Base, ReadValue);
    EXPECT_EQ(Result, 42U);
    EXPECT_EQ(Calls, 1U);
    bool VisitedStore = false;
    const auto MarkVisited = [&](const auto &Concrete)
    {
      using InstructionType = std::remove_cvref_t<decltype(Concrete)>;
      ++Calls;
      VisitedStore = std::is_same_v<InstructionType, MachineStore64iInstruction>;
      EXPECT_EQ(static_cast<const MachineInstruction *>(&Concrete), &Base);
    };
    visitInstruction(Base, MarkVisited);
    EXPECT_TRUE(VisitedStore);
    EXPECT_EQ(Calls, 2U);
  }

  // Rewriters receive copied address, register and immediate operands in semantic order without changing their source instruction.
  TEST(VMMachineInstructionCloneTest, RewritesOperandCopiesAndPreservesSource)
  {
    const auto Symbol = core::NamePool::instance().intern("clone.rewritten.symbol");
    const MachineAdd64mriInstruction Source(AddressOperand(RegisterId::BP, 8), RegisterOperand(RegisterId::SP, std::numeric_limits<std::int64_t>::min()), ImmediateOperand(42));
    MachineBasicBlock Destination;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Destination);
    std::vector<OperandKind> Kinds;
    const auto Rewrite = [&](auto &Value)
    {
      using OperandType = std::remove_cvref_t<decltype(Value)>;
      Kinds.push_back(Value.getKind());
      if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        Value = AddressOperand(Symbol, -5);
      }
      else if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        EXPECT_EQ(Value.getByteOffset(), std::numeric_limits<std::int64_t>::min());
        Value = RegisterOperand(RegisterId::BP, std::numeric_limits<std::int64_t>::max());
      }
      else if constexpr (std::is_same_v<OperandType, ImmediateOperand>)
      {
        Value = ImmediateOperand(Symbol, 7);
      }
      return true;
    };
    auto *Cloned = tryCloneInstruction(Source, Builder, Rewrite);
    ASSERT_NE(Cloned, nullptr);
    const std::vector<OperandKind> ExpectedKinds{OperandKind::Address, OperandKind::Register, OperandKind::Immediate};
    EXPECT_EQ(Kinds, ExpectedKinds);
    EXPECT_EQ(Cloned->getOpcode(), Opcode::Add64mri);
    EXPECT_EQ(Cloned->parent(), &Destination);
    EXPECT_EQ(Destination.instructionCount(), 1U);
    const auto &TypedClone = static_cast<const MachineAdd64mriInstruction &>(*Cloned);
    EXPECT_EQ(TypedClone.getDestination().getSymbol(), Symbol);
    EXPECT_EQ(TypedClone.getDestination().getByteOffset(), -5);
    EXPECT_EQ(TypedClone.getLHS().getRegister(), RegisterId::BP);
    EXPECT_EQ(TypedClone.getLHS().getByteOffset(), std::numeric_limits<std::int64_t>::max());
    EXPECT_EQ(TypedClone.getRHS().getSymbol(), Symbol);
    EXPECT_EQ(TypedClone.getRHS().getByteOffset(), 7);
    EXPECT_EQ(Source.parent(), nullptr);
    EXPECT_EQ(Source.getDestination().getRegister(), RegisterId::BP);
    EXPECT_EQ(Source.getDestination().getByteOffset(), 8);
    EXPECT_EQ(Source.getLHS().getRegister(), RegisterId::SP);
    EXPECT_EQ(Source.getLHS().getByteOffset(), std::numeric_limits<std::int64_t>::min());
    EXPECT_EQ(Source.getRHS().bits(), 42U);
  }

  // Rejection of a middle operand stops subsequent hooks and preserves the destination anchor, source and prior callback side effects.
  TEST(VMMachineInstructionCloneTest, ShortCircuitsRewriteFailureWithoutInsertingCode)
  {
    const MachineAdd64mmiInstruction Source(AddressOperand(RegisterId::BP, 8), AddressOperand(RegisterId::SP, 16), ImmediateOperand(42));
    MachineBasicBlock Destination;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Destination);
    auto &Anchor = Builder.create<Opcode::Ret>();
    ASSERT_TRUE(Builder.setInsertPoint(Anchor));
    std::vector<OperandKind> Kinds;
    const auto Rewrite = [&](auto &Value)
    {
      using OperandType = std::remove_cvref_t<decltype(Value)>;
      Kinds.push_back(Value.getKind());
      if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        Value = AddressOperand(RegisterId::PC, 99);
      }
      return Kinds.size() != 2;
    };
    auto *Cloned = tryCloneInstruction(Source, Builder, Rewrite);
    EXPECT_EQ(Cloned, nullptr);
    const std::vector<OperandKind> ExpectedKinds{OperandKind::Address, OperandKind::Address};
    EXPECT_EQ(Kinds, ExpectedKinds);
    ASSERT_EQ(Destination.instructionCount(), 1U);
    EXPECT_EQ(&*Destination.instructions().begin(), &Anchor);
    EXPECT_EQ(Builder.getInsertBefore(), &Anchor);
    EXPECT_EQ(Source.getDestination().getRegister(), RegisterId::BP);
    EXPECT_EQ(Source.getDestination().getByteOffset(), 8);
    EXPECT_EQ(Source.getLHS().getRegister(), RegisterId::SP);
    EXPECT_EQ(Source.getLHS().getByteOffset(), 16);
    EXPECT_EQ(Source.getRHS().bits(), 42U);
  }

  // Branch clones require a target owned by the destination function and can remap their copied block operand to satisfy that rule.
  TEST(VMMachineInstructionCloneTest, RemapsBranchTargetsAndRejectsInvalidOwnership)
  {
    MachineModule SourceModule;
    auto &SourceFunction = SourceModule.createFunction();
    auto &SourceEntry = SourceFunction.createBasicBlock();
    auto &SourceTarget = SourceFunction.createBasicBlock();
    MachineInstructionBuilder SourceBuilder;
    SourceBuilder.setInsertPoint(SourceEntry);
    const auto &Source = SourceBuilder.create<Opcode::Jmp>(&SourceTarget);
    MachineModule DestinationModule;
    auto &DestinationFunction = DestinationModule.createFunction();
    auto &DestinationEntry = DestinationFunction.createBasicBlock();
    auto &DestinationTarget = DestinationFunction.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(DestinationEntry);
    const auto KeepOperand = [](auto &)
    {
      return true;
    };
    EXPECT_EQ(tryCloneInstruction(Source, Builder, KeepOperand), nullptr);
    EXPECT_EQ(DestinationEntry.instructionCount(), 0U);
    std::size_t BlockHooks = 0;
    const auto RemapTarget = [&](auto &Value)
    {
      using OperandType = std::remove_cvref_t<decltype(Value)>;
      if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
      {
        ++BlockHooks;
        EXPECT_EQ(Value.getBlock(), &SourceTarget);
        Value = BasicBlockOperand(&DestinationTarget);
      }
      return true;
    };
    auto *Cloned = tryCloneInstruction(Source, Builder, RemapTarget);
    ASSERT_NE(Cloned, nullptr);
    EXPECT_EQ(BlockHooks, 1U);
    EXPECT_EQ(Cloned->getOpcode(), Opcode::Jmp);
    EXPECT_EQ(static_cast<const MachineJmpInstruction &>(*Cloned).getTarget(), &DestinationTarget);
    EXPECT_EQ(Cloned->parent(), &DestinationEntry);
    EXPECT_EQ(Source.getTarget(), &SourceTarget);
    EXPECT_EQ(Source.parent(), &SourceEntry);
    const auto NullTarget = [](auto &Value)
    {
      using OperandType = std::remove_cvref_t<decltype(Value)>;
      if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
      {
        Value = BasicBlockOperand(nullptr);
      }
      return true;
    };
    EXPECT_EQ(tryCloneInstruction(Source, Builder, NullTarget), nullptr);
    EXPECT_EQ(DestinationEntry.instructionCount(), 1U);
  }

  // Cloning a zero-operand return inserts it successfully without invoking an operand rewriter that would reject all operands.
  TEST(VMMachineInstructionCloneTest, ClonesZeroOperandInstructionsWithoutCallingRewriter)
  {
    const MachineRetInstruction Source;
    MachineBasicBlock Destination;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Destination);
    std::size_t Calls = 0;
    const auto RejectOperand = [&](auto &)
    {
      ++Calls;
      return false;
    };
    auto *Cloned = tryCloneInstruction(Source, Builder, RejectOperand);
    ASSERT_NE(Cloned, nullptr);
    EXPECT_EQ(Cloned->getOpcode(), Opcode::Ret);
    EXPECT_EQ(Cloned->parent(), &Destination);
    EXPECT_EQ(Destination.instructionCount(), 1U);
    EXPECT_EQ(Calls, 0U);
    EXPECT_EQ(Source.parent(), nullptr);
  }

  // Unset and cleared insertion points fail safely without assigning ownership or inserting a cloned instruction.
  TEST(VMMachineInstructionCloneTest, RejectsMissingInsertionPointsWithoutMutation)
  {
    const MachineStore64iInstruction Source(AddressOperand(RegisterId::BP, 8), ImmediateOperand(42));
    MachineInstructionBuilder Builder;
    const auto KeepOperand = [](auto &)
    {
      return true;
    };
    EXPECT_EQ(tryCloneInstruction(Source, Builder, KeepOperand), nullptr);
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    MachineBasicBlock Destination;
    Builder.setInsertPoint(Destination);
    Builder.clearInsertPoint();
    EXPECT_EQ(tryCloneInstruction(Source, Builder, KeepOperand), nullptr);
    EXPECT_EQ(Destination.instructionCount(), 0U);
    EXPECT_EQ(Source.parent(), nullptr);
    EXPECT_EQ(Source.getDestination().getRegister(), RegisterId::BP);
    EXPECT_EQ(Source.getDestination().getByteOffset(), 8);
    EXPECT_EQ(Source.getSource().bits(), 42U);
  }
} // namespace ink::vm::test
