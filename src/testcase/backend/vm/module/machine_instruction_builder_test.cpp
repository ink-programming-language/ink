#include "ink/core/name/name_pool.h"
#include "ink/backend/vm/module/machine_basic_block.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"

#include <gtest/gtest.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  namespace
  {
    std::vector<const MachineInstruction *> blockInstructionAddresses(const MachineBasicBlock &Block)
    {
      std::vector<const MachineInstruction *> Values;
      for (const MachineInstruction &Value : Block.instructions())
      {
        Values.push_back(&Value);
      }
      return Values;
    }

    template <Opcode Operation, typename... ValueTypes>
    concept CanBuildVMInstruction = requires(MachineInstructionBuilder &Builder, ValueTypes &&...Values) {
      { Builder.create<Operation>(std::forward<ValueTypes>(Values)...) } -> std::same_as<MachineInstructionImpl<Operation> &>;
      { Builder.tryCreate<Operation>(std::forward<ValueTypes>(Values)...) } -> std::same_as<MachineInstructionImpl<Operation> *>;
    };

    template <typename OperandType>
    OperandType makeBuilderTestOperand(std::size_t Index)
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        return RegisterOperand(static_cast<RegisterId>(Index % 3));
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        return AddressOperand(RegisterId::BP, static_cast<std::int64_t>(16 + Index));
      }
      else
      {
        static_assert(std::is_same_v<OperandType, ImmediateOperand>);
        return ImmediateOperand(64 + Index);
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    void buildRegisteredInstruction(MachineInstructionBuilder &Builder, MachineBasicBlock &Block, std::index_sequence<Indices...>)
    {
      using OperandTuple = typename OpcodeTraits<Operation>::OperandTuple;
      auto &Value = Builder.create<Operation>(makeBuilderTestOperand<std::tuple_element_t<Indices, OperandTuple>>(Indices)...);
      const MachineInstruction &Base = Value;
      EXPECT_EQ(Value.parent(), &Block);
      EXPECT_EQ(Base.getOpcode(), Operation);
      EXPECT_EQ(Base.operandCount(), sizeof...(Indices));
      for (std::size_t Index = 0; Index < Base.operandCount(); ++Index)
      {
        EXPECT_EQ(Base.getOperand(Index), Value.getOperand(Index));
      }
    }
  } // namespace

  // A detached block preserves its name and exposes an empty instruction sequence without an owning function.
  TEST(VMMachineBasicBlockTest, PreservesDetachedIdentityAndConstRangeTypes)
  {
    const core::Name BlockName = core::NamePool::instance().intern("vm.detached.entry");
    MachineBasicBlock Block(BlockName);
    static_assert(!std::is_copy_constructible_v<MachineBasicBlock> && !std::is_move_constructible_v<MachineBasicBlock>);
    static_assert(std::is_same_v<decltype(std::declval<MachineBasicBlock &>().parent()), MachineFunction *>);
    static_assert(std::is_same_v<decltype(std::declval<const MachineBasicBlock &>().parent()), const MachineFunction *>);
    static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<MachineBasicBlock &>().instructions())>, MachineInstruction &>);
    static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const MachineBasicBlock &>().instructions())>, const MachineInstruction &>);
    EXPECT_EQ(Block.name(), BlockName);
    EXPECT_EQ(Block.parent(), nullptr);
    EXPECT_EQ(Block.instructionCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(Block.instructions()));
    const MachineBasicBlock &ConstBlock = Block;
    EXPECT_TRUE(std::ranges::empty(ConstBlock.instructions()));
  }

  // An unset insertion point reports a recoverable creation failure and leaves detached operands unowned.
  TEST(VMMachineInstructionBuilderTest, RejectsCreationWithoutAnInsertionPoint)
  {
    MachineInstructionBuilder Builder;
    static_assert(!std::is_copy_constructible_v<MachineInstructionBuilder> && !std::is_move_constructible_v<MachineInstructionBuilder>);
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    EXPECT_EQ(Builder.tryCreate<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(42)), nullptr);
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    const MachineStore32iInstruction Detached(AddressOperand(RegisterId::BP), ImmediateOperand(7));
    static_assert(std::is_same_v<decltype(Detached.parent()), const MachineBasicBlock *>);
    EXPECT_EQ(Detached.parent(), nullptr);
  }

  // Appending creates typed block-owned instructions and copies each caller-owned operand payload.
  TEST(VMMachineInstructionBuilderTest, AppendsTypedInstructionsWithOwnedOperands)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    AddressOperand Destination(RegisterId::BP, 16);
    ImmediateOperand Source(20);
    auto &First = Builder.create<Opcode::Store32i>(Destination, Source);
    auto *Second = Builder.tryCreate<Opcode::Add32mmi>(AddressOperand(RegisterId::BP, 20), Destination, ImmediateOperand(22));
    ASSERT_NE(Second, nullptr);
    Destination = AddressOperand(RegisterId::SP, -8);
    Source = ImmediateOperand(99);
    EXPECT_EQ(Block.instructionCount(), 2U);
    EXPECT_EQ(First.parent(), &Block);
    EXPECT_EQ(Second->parent(), &Block);
    EXPECT_EQ(First.getDestination().getRegister(), RegisterId::BP);
    EXPECT_EQ(First.getDestination().getByteOffset(), 16);
    EXPECT_EQ(First.getSource().bits(), 20U);
    EXPECT_EQ(Second->getLHS().getByteOffset(), 16);
    EXPECT_EQ(Builder.getInsertBlock(), &Block);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    const std::vector<const MachineInstruction *> Expected = {&First, Second};
    EXPECT_EQ(blockInstructionAddresses(Block), Expected);
  }

  // Repeated insertion before one anchor preserves creation order and keeps the anchor selected.
  TEST(VMMachineInstructionBuilderTest, RepeatedInsertionPreservesAnchorAndOrder)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    auto &First = Builder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    auto &Anchor = Builder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(4));
    ASSERT_TRUE(Builder.setInsertPoint(Anchor));
    auto &Second = Builder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(2));
    auto &Third = Builder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(3));
    const std::vector<const MachineInstruction *> Expected = {&First, &Second, &Third, &Anchor};
    EXPECT_EQ(blockInstructionAddresses(Block), Expected);
    EXPECT_EQ(Builder.getInsertBefore(), &Anchor);
    EXPECT_EQ(Anchor.parent(), &Block);
    EXPECT_EQ(Second.parent(), &Block);
  }

  // Selecting a block's first instruction permits insertion at its beginning without moving existing instructions.
  TEST(VMMachineInstructionBuilderTest, InsertsBeforeTheFirstInstruction)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    auto &Anchor = Builder.create<Opcode::Store16i>(AddressOperand(RegisterId::BP), ImmediateOperand(2));
    ASSERT_TRUE(Builder.setInsertPoint(Anchor));
    auto &First = Builder.create<Opcode::Store16i>(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    const std::vector<const MachineInstruction *> Expected = {&First, &Anchor};
    EXPECT_EQ(blockInstructionAddresses(Block), Expected);
    EXPECT_EQ(Builder.getInsertBefore(), &Anchor);
  }

  // Selecting an instruction in another block transfers the insertion point to its actual owning block.
  TEST(VMMachineInstructionBuilderTest, SelectsOwningBlockWhenChangingAnchors)
  {
    MachineBasicBlock Left;
    MachineBasicBlock Right;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Left);
    auto &LeftValue = Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    Builder.setInsertPoint(Right);
    auto &RightValue = Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(3));
    ASSERT_TRUE(Builder.setInsertPoint(LeftValue));
    EXPECT_EQ(Builder.getInsertBlock(), &Left);
    EXPECT_EQ(Builder.getInsertBefore(), &LeftValue);
    auto &BeforeLeft = Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(0));
    const std::vector<const MachineInstruction *> LeftExpected = {&BeforeLeft, &LeftValue};
    const std::vector<const MachineInstruction *> RightExpected = {&RightValue};
    EXPECT_EQ(blockInstructionAddresses(Left), LeftExpected);
    EXPECT_EQ(blockInstructionAddresses(Right), RightExpected);
  }

  // Rejecting a detached instruction preserves the previously selected block and anchor.
  TEST(VMMachineInstructionBuilderTest, RejectsDetachedAnchorsWithoutChangingInsertionPoint)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    auto &Anchor = Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(2));
    ASSERT_TRUE(Builder.setInsertPoint(Anchor));
    MachineStore64iInstruction Detached(AddressOperand(RegisterId::BP), ImmediateOperand(99));
    EXPECT_FALSE(Builder.setInsertPoint(Detached));
    EXPECT_EQ(Builder.getInsertBlock(), &Block);
    EXPECT_EQ(Builder.getInsertBefore(), &Anchor);
    auto &First = Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    const std::vector<const MachineInstruction *> Expected = {&First, &Anchor};
    EXPECT_EQ(blockInstructionAddresses(Block), Expected);
    EXPECT_EQ(Detached.parent(), nullptr);
  }

  // Clearing an insertion point disables creation while preserving already-owned instructions for later reuse.
  TEST(VMMachineInstructionBuilderTest, ClearsInsertionPointWithoutRemovingInstructions)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    auto &First = Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    Builder.clearInsertPoint();
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    EXPECT_EQ(Builder.tryCreate<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(99)), nullptr);
    EXPECT_EQ(Block.instructionCount(), 1U);
    Builder.setInsertPoint(Block);
    auto &Second = Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::BP), ImmediateOperand(2));
    const std::vector<const MachineInstruction *> Expected = {&First, &Second};
    EXPECT_EQ(blockInstructionAddresses(Block), Expected);
  }

  // Destroying the builder leaves both the instruction and its inline operands alive in the owning block.
  TEST(VMMachineInstructionBuilderTest, KeepsInstructionsAliveAfterBuilderDestruction)
  {
    MachineBasicBlock Block;
    const MachineStore64iInstruction *Created = nullptr;
    {
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      Created = &Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, -24), ImmediateOperand(42));
    }
    ASSERT_NE(Created, nullptr);
    EXPECT_EQ(Block.instructionCount(), 1U);
    EXPECT_EQ(Created->parent(), &Block);
    EXPECT_EQ(Created->getDestination().getByteOffset(), -24);
    EXPECT_EQ(Created->getSource().bits(), 42U);
    EXPECT_EQ(blockInstructionAddresses(Block).front(), Created);
  }

  // Multiple builders can share an anchor while each retains its own insertion state.
  TEST(VMMachineInstructionBuilderTest, KeepsIndependentInsertionPointsForSharedAnchors)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder FirstBuilder;
    MachineInstructionBuilder SecondBuilder;
    FirstBuilder.setInsertPoint(Block);
    auto &Anchor = FirstBuilder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(4));
    ASSERT_TRUE(FirstBuilder.setInsertPoint(Anchor));
    ASSERT_TRUE(SecondBuilder.setInsertPoint(Anchor));
    auto &First = FirstBuilder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    auto &Second = SecondBuilder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(2));
    FirstBuilder.clearInsertPoint();
    auto &Third = SecondBuilder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP), ImmediateOperand(3));
    const std::vector<const MachineInstruction *> Expected = {&First, &Second, &Third, &Anchor};
    EXPECT_EQ(blockInstructionAddresses(Block), Expected);
    EXPECT_EQ(SecondBuilder.getInsertBefore(), &Anchor);
  }

  // Appending many nodes preserves every earlier instruction address, payload, and parent association.
  TEST(VMMachineInstructionBuilderTest, PreservesInstructionIdentityAcrossManyInsertions)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    std::array<const MachineStore64iInstruction *, 256> Created{};
    for (std::size_t Index = 0; Index < Created.size(); ++Index)
    {
      Created[Index] = &Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, static_cast<std::int64_t>(Index * 8)), ImmediateOperand(Index));
    }
    ASSERT_EQ(Block.instructionCount(), Created.size());
    std::size_t Index = 0;
    for (MachineInstruction &Value : Block.instructions())
    {
      ASSERT_LT(Index, Created.size());
      EXPECT_EQ(&Value, Created[Index]);
      EXPECT_EQ(Value.parent(), &Block);
      EXPECT_EQ(Created[Index]->getSource().bits(), Index);
      ++Index;
    }
    EXPECT_EQ(Index, Created.size());
  }

  // Builder construction accepts exact typed operand packs and rejects raw values, wrong kinds, and wrong arity.
  TEST(VMMachineInstructionBuilderTest, RestrictsCreationToRegisteredTypedLayouts)
  {
    static_assert(CanBuildVMInstruction<Opcode::Add64rri, RegisterOperand, RegisterOperand, ImmediateOperand>);
    static_assert(CanBuildVMInstruction<Opcode::Store32i, const AddressOperand &, const ImmediateOperand &>);
    static_assert(!CanBuildVMInstruction<Opcode::Add64rri, RegisterOperand, AddressOperand, ImmediateOperand>);
    static_assert(!CanBuildVMInstruction<Opcode::Add64rri, RegisterId, RegisterId, std::uint64_t>);
    static_assert(!CanBuildVMInstruction<Opcode::Store32i, AddressOperand>);
    static_assert(!CanBuildVMInstruction<Opcode::Store32i, AddressOperand, ImmediateOperand, ImmediateOperand>);
    SUCCEED();
  }

  // Every registered opcode can be created, traversed, and destroyed through the block's concrete-type owner.
  TEST(VMMachineInstructionBuilderTest, BuildsAndOwnsEveryRegisteredOpcode)
  {
    MachineBasicBlock Block;
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    std::size_t ExpectedCount = 0;
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount)                                     \
  buildRegisteredInstruction<Opcode::Kind>(Builder, Block, std::make_index_sequence<RegisterCount + AddressCount + ImmediateCount>{}); \
  ++ExpectedCount;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    EXPECT_EQ(Block.instructionCount(), ExpectedCount);
    EXPECT_EQ(blockInstructionAddresses(Block).size(), ExpectedCount);
  }

  // A module-owned function and block retain instructions after the temporary builder leaves scope.
  TEST(VMMachineInstructionBuilderTest, IntegratesWithModuleFunctionAndBlockOwnership)
  {
    MachineModule ModuleValue;
    auto &FunctionValue = ModuleValue.createFunction();
    auto &Block = FunctionValue.createBasicBlock();
    const MachineAdd64rriInstruction *Created = nullptr;
    {
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Block);
      Created = &Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(16));
    }
    ASSERT_NE(Created, nullptr);
    EXPECT_EQ(ModuleValue.functionCount(), 1U);
    EXPECT_EQ(FunctionValue.parent(), &ModuleValue);
    EXPECT_EQ(FunctionValue.basicBlockCount(), 1U);
    EXPECT_EQ(Block.parent(), &FunctionValue);
    EXPECT_EQ(Created->parent(), &Block);
    EXPECT_EQ(Created->getRHS().bits(), 16U);
  }
} // namespace ink::vm::test
