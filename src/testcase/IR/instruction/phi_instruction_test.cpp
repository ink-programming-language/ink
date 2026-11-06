#include "ink/core/support/core_define.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/instruction/phi_instruction.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Instruction, PhiInstruction>);
  static_assert(!std::is_copy_constructible_v<PhiInstruction> && !std::is_copy_assignable_v<PhiInstruction>);
  static_assert(!std::is_move_constructible_v<PhiInstruction> && !std::is_move_assignable_v<PhiInstruction>);
  static_assert(std::is_nothrow_constructible_v<PhiInstruction, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<PhiInstruction, const IntegerType &>);
  static_assert(!std::is_constructible_v<PhiInstruction, IntegerType &&> && !std::is_constructible_v<PhiInstruction, const IntegerType &&>);
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createPhi(std::declval<ValueRef>())), PhiInstruction &>);
  static_assert(std::is_same_v<decltype(std::declval<const PhiInstruction &>().incomingValue(0)), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<PhiInstruction &>().incomingBlock(0)), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const PhiInstruction &>().incomingBlock(0)), const BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const PhiInstruction &>().incoming()), std::span<const PhiInstruction::Incoming>>);
  static_assert(std::is_same_v<decltype(std::declval<PhiInstruction &>().incoming()), std::span<const PhiInstruction::Incoming>>);
  static_assert(std::is_same_v<decltype(std::declval<const PhiInstruction::Incoming &>().getValue()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<PhiInstruction::Incoming &>().getBlock()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const PhiInstruction::Incoming &>().getBlock()), const BasicBlock &>);

  template <typename Operand>
  concept PhiAcceptsIncomingValue = requires(PhiInstruction &Phi, Operand &&IncomingValue, BasicBlock &Block)
  {
    Phi.addIncoming(std::forward<Operand>(IncomingValue), Block);
  };

  template <typename BlockType>
  concept PhiAcceptsIncomingBlock = requires(PhiInstruction &Phi, ValueRef IncomingValue, BlockType &&Block)
  {
    Phi.addIncoming(IncomingValue, std::forward<BlockType>(Block));
  };

  template <typename ResultType>
  concept BuilderAcceptsPhiResult = requires(InstructionBuilder &Builder, ResultType &&TypeValue)
  {
    Builder.createPhi(std::forward<ResultType>(TypeValue));
  };

  static_assert(PhiAcceptsIncomingValue<Argument &> && PhiAcceptsIncomingValue<const Argument &>);
  static_assert(PhiAcceptsIncomingValue<IntegerConstant &> && PhiAcceptsIncomingValue<const IntegerConstant &>);
  static_assert(!PhiAcceptsIncomingValue<Argument> && !PhiAcceptsIncomingValue<const Argument>);
  static_assert(!PhiAcceptsIncomingValue<IntegerConstant> && !PhiAcceptsIncomingValue<const IntegerConstant>);
  static_assert(PhiAcceptsIncomingBlock<BasicBlock &> && !PhiAcceptsIncomingBlock<BasicBlock> && !PhiAcceptsIncomingBlock<const BasicBlock &>);
  static_assert(BuilderAcceptsPhiResult<IntegerType &> && BuilderAcceptsPhiResult<const Argument &>);
  static_assert(!BuilderAcceptsPhiResult<IntegerType> && !BuilderAcceptsPhiResult<const IntegerType>);
  static_assert(!BuilderAcceptsPhiResult<Argument> && !BuilderAcceptsPhiResult<const Argument>);

  // A loop phi is created empty and receives its backedge only after the dependent add and branch exist.
  TEST(IRPhiInstructionTest, StagesIncomingPairsWhileBuildingALoop)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Function FunctionValue(Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Int32), Parameter(Types.getBoolType())}));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    BasicBlock &Loop = FunctionValue.createBasicBlock();
    BasicBlock &Exit = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createBranch(Loop));
    Builder.setInsertPoint(Loop);
    PhiInstruction &Phi = Builder.createPhi(Int32);
    EXPECT_EQ(Phi.incomingCount(), 0U);
    EXPECT_TRUE(Phi.incoming().empty());
    Phi.addIncoming(FunctionValue.argument(0), Entry);
    EXPECT_EQ(Phi.incomingCount(), 1U);
    AddInstruction &Next = Builder.createAdd(Phi, FunctionValue.argument(1));
    static_cast<void>(Builder.createCondBranch(FunctionValue.argument(2), Loop, Exit));
    Phi.addIncoming(Next, Loop);
    Builder.setInsertPoint(Exit);
    ReturnInstruction &Return = Builder.createReturn(Phi);
    const PhiInstruction &ConstPhi = Phi;
    EXPECT_EQ(Phi.getKind(), InstructionKind::Phi);
    EXPECT_EQ(Phi.getName(), "phi");
    EXPECT_FALSE(Phi.isTerminator());
    EXPECT_TRUE(PhiInstruction::classof(&Phi));
    EXPECT_FALSE(PhiInstruction::classof(&Next));
    EXPECT_FALSE(PhiInstruction::classof(nullptr));
    EXPECT_EQ(&Phi.getType(), &Int32);
    EXPECT_EQ(Phi.parent(), &Loop);
    ASSERT_EQ(Phi.incomingCount(), 2U);
    EXPECT_EQ(&Phi.incomingValue(0), &FunctionValue.argument(0));
    EXPECT_EQ(&ConstPhi.incomingValue(1), &Next);
    EXPECT_EQ(&Phi.incomingBlock(0), &Entry);
    EXPECT_EQ(&ConstPhi.incomingBlock(1), &Loop);
    EXPECT_EQ(Phi.incoming()[0].IncomingValue, &FunctionValue.argument(0));
    EXPECT_EQ(Phi.incoming()[1].Predecessor, &Loop);
    EXPECT_EQ(&Phi.incoming()[1].getValue(), &Next);
    EXPECT_EQ(&Phi.incoming()[0].getBlock(), &Entry);
    EXPECT_EQ(&Next.getLHS(), &Phi);
    EXPECT_EQ(Return.getReturnValue(), &Phi);
  }

  // Direct incoming mutation retains mismatched values, duplicate edges, and foreign blocks for caller-controlled construction.
  TEST(IRPhiInstructionTest, RetainsUnvalidatedIncomingPairsWithoutChangingTheirIdentities)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Value(Int32, 1);
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    Function ForeignOwner(Types.getFunctionType(Types.getVoidType()));
    BasicBlock &OwnerBlock = Owner.createBasicBlock();
    BasicBlock &ForeignBlock = ForeignOwner.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(OwnerBlock);
    PhiInstruction &Phi = Builder.createPhi(Int32);
    Phi.addIncoming(Owner.argument(0), ForeignBlock);
    Phi.addIncoming(Value, ForeignBlock);
    ASSERT_EQ(Phi.incomingCount(), 2U);
    EXPECT_EQ(&Phi.incomingValue(0), &Owner.argument(0));
    EXPECT_EQ(&Phi.incomingValue(1), &Value);
    EXPECT_EQ(&Phi.incomingBlock(0), &ForeignBlock);
    EXPECT_EQ(&Phi.incomingBlock(1), &ForeignBlock);
    EXPECT_EQ(&Phi.getType(), &Int32);
  }

  // Phi insertion can extend or prepend the leading prefix after ordinary instructions and a terminator have been built.
  TEST(IRPhiInstructionTest, InsertsOnlyWithinTheLeadingPhiPrefix)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Value(Int32, 1);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    PhiInstruction &First = Builder.createPhi(Int32);
    PhiInstruction &Second = Builder.createPhi(Int32);
    AddInstruction &Body = Builder.createAdd(Value, Value);
    ReturnInstruction &Return = Builder.createReturn(Body);
    ASSERT_TRUE(Builder.setInsertPoint(Body));
    PhiInstruction &Third = Builder.createPhi(Int32);
    ASSERT_TRUE(Builder.setInsertPoint(First));
    PhiInstruction &Leading = Builder.createPhi(Int32);
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(Value, Value)), "Non-phi instructions cannot be inserted before phi instructions");
    ASSERT_TRUE(Builder.setInsertPoint(Return));
    EXPECT_DEATH(static_cast<void>(Builder.createPhi(Int32)), "Phi instructions must form a prefix at the start of a block");
    Builder.setInsertPoint(Block);
    EXPECT_DEATH(static_cast<void>(Builder.createPhi(Int32)), "Cannot append instructions after a terminator");
#endif
    const Instruction *Expected[] = {
        &Leading,
        &First,
        &Second,
        &Third,
        &Body,
        &Return,
    };
    ASSERT_EQ(Block.instructionCount(), 6U);
    auto Position = Block.instructions().begin();
    for (const Instruction *ExpectedInstruction : Expected)
    {
      EXPECT_EQ(&*Position++, ExpectedInstruction);
    }
    EXPECT_EQ(Position, Block.instructions().end());
  }

#if INK_ENABLE_ASSERTIONS
  // Phi creation checks its insertion point and non-void type value, and indexed access checks the incoming bounds.
  TEST(IRPhiInstructionDeathTest, AssertsOnInvalidResultTypesInsertionPointsAndIncomingIndices)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Value(Int32, 1);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    EXPECT_DEATH(static_cast<void>(Builder.createPhi(Int32)), "InstructionBuilder requires an insertion point");
    Builder.setInsertPoint(Block);
    EXPECT_DEATH(static_cast<void>(Builder.createPhi(Types.getVoidType())), "Phi requires a non-void type value");
    EXPECT_DEATH(static_cast<void>(Builder.createPhi(Value)), "Phi requires a non-void type value");
    PhiInstruction &Phi = Builder.createPhi(Int32);
    EXPECT_DEATH(static_cast<void>(Phi.incomingValue(0)), "Phi incoming index is out of range");
    EXPECT_DEATH(static_cast<void>(Phi.incomingBlock(0)), "Phi incoming index is out of range");
    Phi.addIncoming(Value, Block);
    const PhiInstruction &ConstPhi = Phi;
    EXPECT_DEATH(static_cast<void>(ConstPhi.incomingValue(1)), "Phi incoming index is out of range");
    EXPECT_DEATH(static_cast<void>(ConstPhi.incomingBlock(1)), "Phi incoming index is out of range");
  }
#endif
} // namespace ink::ir::test
