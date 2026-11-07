#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/support/core_define.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  template <typename Operand>
  concept AcceptsOperandReplacement = requires(Instruction &Node, Operand &&Replacement)
  {
    Node.setOperand(0, std::forward<Operand>(Replacement));
  };

  template <typename NodeType>
  concept AllowsOperandMutation = requires(NodeType &Node, ValueRef Replacement)
  {
    Node.setOperand(0, Replacement);
  };

  static_assert(std::is_same_v<decltype(std::declval<const Instruction &>().operandCount()), std::size_t>);
  static_assert(std::is_same_v<decltype(std::declval<Instruction &>().operand(0)), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const Instruction &>().operand(0)), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<Instruction &>().setOperand(0, std::declval<ValueRef>())), bool>);
  static_assert(noexcept(std::declval<const Instruction &>().operandCount()));
  static_assert(noexcept(std::declval<const Instruction &>().operand(0)));
  static_assert(noexcept(std::declval<Instruction &>().setOperand(0, std::declval<ValueRef>())));
  static_assert(AllowsOperandMutation<Instruction> && !AllowsOperandMutation<const Instruction>);
  static_assert(AcceptsOperandReplacement<ValueRef>);
  static_assert(AcceptsOperandReplacement<IntegerConstant &> && AcceptsOperandReplacement<const IntegerConstant &>);
  static_assert(AcceptsOperandReplacement<Argument &> && AcceptsOperandReplacement<const Argument &>);
  static_assert(AcceptsOperandReplacement<AddInstruction &> && AcceptsOperandReplacement<const AddInstruction &>);
  static_assert(!AcceptsOperandReplacement<IntegerConstant> && !AcceptsOperandReplacement<const IntegerConstant>);
  static_assert(!AcceptsOperandReplacement<Argument> && !AcceptsOperandReplacement<const Argument>);
  static_assert(!AcceptsOperandReplacement<AddInstruction> && !AcceptsOperandReplacement<const AddInstruction>);

  // Base-class access preserves arithmetic, memory, and return slot order while replacing only the selected operand.
  TEST(IROperandTest, ReplacesArithmeticMemoryAndReturnOperandsThroughTheBaseClass)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant Two(Int32, 2);
    const IntegerConstant Three(Int32, 3);
    AllocaInstruction FirstSlot(Types.getPointerType(Int32));
    AllocaInstruction SecondSlot(Types.getPointerType(Int32));
    AddInstruction Add(One, One);
    LoadInstruction Load(Int32, FirstSlot);
    StoreInstruction Store(Types.getVoidType(), FirstSlot, One);
    ReturnInstruction Return(Types.getVoidType(), Add);
    Instruction &AddBase = Add;
    Instruction &LoadBase = Load;
    Instruction &StoreBase = Store;
    Instruction &ReturnBase = Return;

    ASSERT_EQ(AddBase.operandCount(), 2U);
    EXPECT_EQ(&AddBase.operand(0), &One);
    EXPECT_EQ(&AddBase.operand(1), &One);
    ASSERT_TRUE(AddBase.setOperand(0, Two));
    EXPECT_EQ(&Add.getLHS(), &Two);
    EXPECT_EQ(&Add.getRHS(), &One);
    ASSERT_TRUE(AddBase.setOperand(1, Three));
    EXPECT_EQ(&AddBase.operand(0), &Two);
    EXPECT_EQ(&AddBase.operand(1), &Three);
    EXPECT_EQ(&Add.getType(), &Int32);

    ASSERT_EQ(LoadBase.operandCount(), 1U);
    EXPECT_EQ(&LoadBase.operand(0), &FirstSlot);
    ASSERT_TRUE(LoadBase.setOperand(0, SecondSlot));
    EXPECT_EQ(&Load.getSource(), &SecondSlot);
    EXPECT_EQ(&Load.getType(), &Int32);

    ASSERT_EQ(StoreBase.operandCount(), 2U);
    EXPECT_EQ(&StoreBase.operand(0), &FirstSlot);
    EXPECT_EQ(&StoreBase.operand(1), &One);
    ASSERT_TRUE(StoreBase.setOperand(0, SecondSlot));
    ASSERT_TRUE(StoreBase.setOperand(1, Two));
    EXPECT_EQ(&Store.getDestination(), &SecondSlot);
    EXPECT_EQ(&Store.getSource(), &Two);
    EXPECT_EQ(&Store.getType(), &Types.getVoidType());

    ASSERT_EQ(ReturnBase.operandCount(), 1U);
    EXPECT_EQ(&ReturnBase.operand(0), &Add);
    ASSERT_TRUE(ReturnBase.setOperand(0, Two));
    EXPECT_EQ(Return.getReturnValue(), &Two);
    EXPECT_EQ(&Return.getType(), &Types.getVoidType());
    EXPECT_TRUE(ReturnBase.setOperand(0, Two));
    EXPECT_EQ(ReturnBase.operandCount(), 1U);
  }

  // Types and control-flow destinations are not data operands, and replacing a branch condition preserves its targets.
  TEST(IROperandTest, ExcludesTypesAndBlocksFromDataOperands)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType()), Parameter(Types.getBoolType())}));
    BasicBlock &TrueTarget = Owner.createBasicBlock();
    BasicBlock &FalseTarget = Owner.createBasicBlock();
    AllocaInstruction Slot(Types.getPointerType(Int32));
    BranchInstruction Branch(Types.getVoidType(), TrueTarget);
    ReturnInstruction VoidReturn(Types.getVoidType());
    PhiInstruction EmptyPhi(Int32);
    const std::array<Instruction *, 4> WithoutOperands = {
        &Slot,
        &Branch,
        &VoidReturn,
        &EmptyPhi,
    };
    for (Instruction *Node : WithoutOperands)
    {
      EXPECT_EQ(Node->operandCount(), 0U);
      EXPECT_FALSE(Node->setOperand(0, One));
      EXPECT_FALSE(Node->setOperand(std::numeric_limits<std::size_t>::max(), One));
      EXPECT_EQ(Node->operandCount(), 0U);
    }
    EXPECT_EQ(&Slot.getAllocatedType(), &Int32);
    EXPECT_EQ(&Branch.getTarget(), &TrueTarget);
    EXPECT_EQ(VoidReturn.getReturnValue(), nullptr);
    EXPECT_EQ(EmptyPhi.incomingCount(), 0U);

    CondBranchInstruction Conditional(Types.getVoidType(), Owner.argument(0), TrueTarget, FalseTarget);
    Instruction &Base = Conditional;
    ASSERT_EQ(Base.operandCount(), 1U);
    EXPECT_EQ(&Base.operand(0), &Owner.argument(0));
    ASSERT_TRUE(Base.setOperand(0, Owner.argument(1)));
    EXPECT_EQ(&Conditional.getCondition(), &Owner.argument(1));
    EXPECT_EQ(&Conditional.getTrueTarget(), &TrueTarget);
    EXPECT_EQ(&Conditional.getFalseTarget(), &FalseTarget);
    EXPECT_EQ(&Conditional.getType(), &Types.getVoidType());
    EXPECT_EQ(Base.operandCount(), 1U);
  }

  // Phi replacement changes an individual incoming value without altering repeated-value slots, predecessor edges, or growth.
  TEST(IROperandTest, ReplacesPhiValuesIndependentlyAndPreservesIncomingBlocks)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant Two(Int32, 2);
    const IntegerConstant Three(Int32, 3);
    BasicBlock Left;
    BasicBlock Right;
    PhiInstruction Phi(Int32);
    Phi.addIncoming(One, Left);
    Phi.addIncoming(One, Right);
    Instruction &Base = Phi;
    const Instruction &ConstBase = Phi;
    ASSERT_EQ(ConstBase.operandCount(), 2U);
    EXPECT_EQ(&ConstBase.operand(0), &One);
    EXPECT_EQ(&ConstBase.operand(1), &One);
    ASSERT_TRUE(Base.setOperand(0, Two));
    EXPECT_EQ(&Phi.incomingValue(0), &Two);
    EXPECT_EQ(&Phi.incomingValue(1), &One);
    ASSERT_TRUE(Base.setOperand(1, Three));
    EXPECT_EQ(&Phi.incoming()[0].getValue(), &Two);
    EXPECT_EQ(&Phi.incoming()[1].getValue(), &Three);
    EXPECT_EQ(&Phi.incomingBlock(0), &Left);
    EXPECT_EQ(&Phi.incomingBlock(1), &Right);
    EXPECT_EQ(&Phi.getType(), &Int32);
    Phi.addIncoming(One, Left);
    ASSERT_EQ(ConstBase.operandCount(), 3U);
    EXPECT_EQ(&ConstBase.operand(0), &Two);
    EXPECT_EQ(&ConstBase.operand(1), &Three);
    EXPECT_EQ(&ConstBase.operand(2), &One);
    ASSERT_TRUE(Base.setOperand(2, Two));
    EXPECT_EQ(&Phi.incomingValue(2), &Two);
    EXPECT_EQ(&Phi.incomingBlock(2), &Left);
  }

  // A type-producing phi can replace a MetaType input locally while allocation metadata continues to borrow the same phi identity.
  TEST(IROperandTest, ReplacesTypeValuedPhiInputsWithoutChangingTypeMetadata)
  {
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType()), Parameter(Types.getMetaType())}));
    BasicBlock &Left = Owner.createBasicBlock();
    BasicBlock &Right = Owner.createBasicBlock();
    PhiInstruction Phi(Types.getMetaType());
    Phi.addIncoming(Owner.argument(0), Left);
    Phi.addIncoming(Owner.argument(1), Right);
    AllocaInstruction Slot(Types.getPointerType(Phi));
    const Value *OriginalPointerType = &Slot.getType();
    Instruction &PhiBase = Phi;
    const Instruction &SlotBase = Slot;
    ASSERT_TRUE(Phi.isTypeValue());
    ASSERT_EQ(PhiBase.operandCount(), 2U);
    EXPECT_EQ(&PhiBase.operand(0), &Owner.argument(0));
    ASSERT_TRUE(PhiBase.setOperand(0, Owner.argument(1)));
    EXPECT_EQ(&Phi.incomingValue(0), &Owner.argument(1));
    EXPECT_EQ(&Phi.incomingValue(1), &Owner.argument(1));
    EXPECT_EQ(&Phi.incomingBlock(0), &Left);
    EXPECT_EQ(&Phi.incomingBlock(1), &Right);
    EXPECT_EQ(&Phi.getType(), &Types.getMetaType());
    EXPECT_EQ(PhiBase.operandCount(), 2U);
    EXPECT_EQ(&Slot.getAllocatedType(), &Phi);
    EXPECT_EQ(&Slot.getType(), OriginalPointerType);
    EXPECT_EQ(SlotBase.operandCount(), 0U);
  }

  // Every populated instruction rejects invalid indices and nonidentical types without modifying any operand or result type.
  TEST(IROperandTest, RejectsInvalidIndicesAndTypeIdentityMismatchesAtomically)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType OtherInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant OtherOne(OtherInt32, 1);
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock Target;
    AllocaInstruction Slot(Types.getPointerType(Int32));
    const PointerType OtherPointerType(Int32);
    AllocaInstruction OtherSlot(OtherPointerType);
    AddInstruction Add(One, One);
    LoadInstruction Load(Int32, Slot);
    StoreInstruction Store(Types.getVoidType(), Slot, One);
    ReturnInstruction Return(Types.getVoidType(), One);
    CondBranchInstruction Conditional(Types.getVoidType(), Owner.argument(0), Target, Target);
    PhiInstruction Phi(Int32);
    Phi.addIncoming(One, Target);
    Phi.addIncoming(One, Target);
    const std::array<Instruction *, 6> Nodes = {
        &Add,
        &Load,
        &Store,
        &Return,
        &Conditional,
        &Phi,
    };
    for (Instruction *Node : Nodes)
    {
      const Value *OriginalType = &Node->getType();
      const std::size_t OriginalCount = Node->operandCount();
      std::array<const Value *, 2> OriginalOperands{};
      for (std::size_t Index = 0; Index < OriginalCount; ++Index)
      {
        OriginalOperands[Index] = &Node->operand(Index);
      }
      EXPECT_FALSE(Node->setOperand(OriginalCount, One));
      EXPECT_FALSE(Node->setOperand(std::numeric_limits<std::size_t>::max(), One));
      for (std::size_t Index = 0; Index < OriginalCount; ++Index)
      {
        EXPECT_FALSE(Node->setOperand(Index, OtherOne));
      }
      EXPECT_EQ(&Node->getType(), OriginalType);
      EXPECT_EQ(Node->operandCount(), OriginalCount);
      for (std::size_t Index = 0; Index < OriginalCount; ++Index)
      {
        EXPECT_EQ(&Node->operand(Index), OriginalOperands[Index]);
      }
    }
    Instruction &LoadBase = Load;
    EXPECT_NE(&Slot.getType(), &OtherSlot.getType());
    EXPECT_FALSE(LoadBase.setOperand(0, OtherSlot));
    EXPECT_EQ(&Load.getSource(), &Slot);
    EXPECT_EQ(&Conditional.getTrueTarget(), &Target);
    EXPECT_EQ(&Conditional.getFalseTarget(), &Target);
    EXPECT_EQ(&Phi.incomingBlock(0), &Target);
    EXPECT_EQ(&Phi.incomingBlock(1), &Target);
  }

  // A completed block retains ownership, instruction order, and valid SSA after its memory and expression operands are changed.
  TEST(IROperandTest, VerifiesACompleteFunctionAfterOperandMutation)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Function Owner(Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Int32)}));
    BasicBlock &Entry = Owner.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    AllocaInstruction &FirstSlot = Builder.createAlloca(Int32);
    AllocaInstruction &SecondSlot = Builder.createAlloca(Int32);
    StoreInstruction &Store = Builder.createStore(FirstSlot, Owner.argument(0));
    LoadInstruction &Load = Builder.createLoad(FirstSlot);
    AddInstruction &Sum = Builder.createAdd(Load, Owner.argument(1));
    ReturnInstruction &Return = Builder.createReturn(Sum);
    const core::DiagnosticEngine Diagnostics;
    verifyFunction(Owner, Diagnostics);
    Instruction &StoreBase = Store;
    Instruction &LoadBase = Load;
    Instruction &SumBase = Sum;
    Instruction &ReturnBase = Return;
    ASSERT_TRUE(StoreBase.setOperand(0, SecondSlot));
    ASSERT_TRUE(StoreBase.setOperand(1, Owner.argument(1)));
    ASSERT_TRUE(LoadBase.setOperand(0, SecondSlot));
    ASSERT_TRUE(SumBase.setOperand(1, Owner.argument(0)));
    ASSERT_TRUE(ReturnBase.setOperand(0, Load));
    verifyFunction(Owner, Diagnostics);
    EXPECT_EQ(Entry.instructionCount(), 6U);
    EXPECT_EQ(Entry.terminator(), &Return);
    EXPECT_EQ(Store.parent(), &Entry);
    EXPECT_EQ(Load.parent(), &Entry);
    EXPECT_EQ(Sum.parent(), &Entry);
    EXPECT_EQ(Return.parent(), &Entry);
    EXPECT_EQ(&Store.getDestination(), &SecondSlot);
    EXPECT_EQ(&Store.getSource(), &Owner.argument(1));
    EXPECT_EQ(&Load.getSource(), &SecondSlot);
    EXPECT_EQ(&Sum.getRHS(), &Owner.argument(0));
    EXPECT_EQ(Return.getReturnValue(), &Load);
  }

  // A loop phi accepts replacement of its entry value and a valid self-referential backedge without changing CFG edges.
  TEST(IROperandTest, VerifiesLoopPhiMutationAtItsPredecessorEdges)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Function Owner(Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Int32), Parameter(Types.getBoolType())}));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Loop = Owner.createBasicBlock();
    BasicBlock &Exit = Owner.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createBranch(Loop));
    Builder.setInsertPoint(Loop);
    PhiInstruction &Phi = Builder.createPhi(Int32);
    Phi.addIncoming(Owner.argument(0), Entry);
    AddInstruction &Next = Builder.createAdd(Phi, Owner.argument(1));
    static_cast<void>(Builder.createCondBranch(Owner.argument(2), Loop, Exit));
    Phi.addIncoming(Next, Loop);
    Builder.setInsertPoint(Exit);
    static_cast<void>(Builder.createReturn(Phi));
    const core::DiagnosticEngine Diagnostics;
    verifyFunction(Owner, Diagnostics);
    Instruction &Base = Phi;
    ASSERT_TRUE(Base.setOperand(0, Owner.argument(1)));
    ASSERT_TRUE(Base.setOperand(1, Phi));
    verifyFunction(Owner, Diagnostics);
    EXPECT_EQ(Base.operandCount(), 2U);
    EXPECT_EQ(&Phi.incomingValue(0), &Owner.argument(1));
    EXPECT_EQ(&Phi.incomingValue(1), &Phi);
    EXPECT_EQ(&Phi.incomingBlock(0), &Entry);
    EXPECT_EQ(&Phi.incomingBlock(1), &Loop);
    EXPECT_EQ(Loop.successorCount(), 2U);
    EXPECT_EQ(&Loop.successor(0), &Loop);
    EXPECT_EQ(&Loop.successor(1), &Exit);
  }

#if INK_ENABLE_ASSERTIONS
  // Indexed base-class reads assert at the exact operand-count boundary for every instruction kind and both return forms.
  TEST(IROperandDeathTest, AssertsOnOutOfRangeOperandReads)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock Target;
    AddInstruction Add(One, One);
    AllocaInstruction Slot(Types.getPointerType(Int32));
    LoadInstruction Load(Int32, Slot);
    StoreInstruction Store(Types.getVoidType(), Slot, One);
    ReturnInstruction Return(Types.getVoidType(), One);
    ReturnInstruction VoidReturn(Types.getVoidType());
    BranchInstruction Branch(Types.getVoidType(), Target);
    CondBranchInstruction Conditional(Types.getVoidType(), Owner.argument(0), Target, Target);
    PhiInstruction Phi(Int32);
    Phi.addIncoming(One, Target);
    const std::array<const Instruction *, 9> Nodes = {
        &Add,
        &Slot,
        &Load,
        &Store,
        &Return,
        &VoidReturn,
        &Branch,
        &Conditional,
        &Phi,
    };
    for (const Instruction *Node : Nodes)
    {
      EXPECT_DEATH(static_cast<void>(Node->operand(Node->operandCount())), "out of range");
    }
  }
#endif
} // namespace ink::ir::test
