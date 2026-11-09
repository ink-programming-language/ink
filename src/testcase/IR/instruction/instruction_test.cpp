#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/phi_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Instruction, AddInstruction>);
  static_assert(std::is_base_of_v<Instruction, ReturnInstruction>);
  static_assert(std::is_base_of_v<Instruction, BranchInstruction> && std::is_base_of_v<Instruction, CondBranchInstruction>);
  static_assert(!std::is_copy_constructible_v<BranchInstruction> && !std::is_copy_assignable_v<BranchInstruction>);
  static_assert(!std::is_move_constructible_v<BranchInstruction> && !std::is_move_assignable_v<BranchInstruction>);
  static_assert(!std::is_copy_constructible_v<CondBranchInstruction> && !std::is_copy_assignable_v<CondBranchInstruction>);
  static_assert(!std::is_move_constructible_v<CondBranchInstruction> && !std::is_move_assignable_v<CondBranchInstruction>);
  static_assert(std::is_nothrow_constructible_v<BranchInstruction, ValueRef, BasicBlock &>);
  static_assert(std::is_nothrow_constructible_v<CondBranchInstruction, ValueRef, ValueRef, BasicBlock &, BasicBlock &>);
  static_assert(!std::is_constructible_v<BranchInstruction, ValueRef, BasicBlock &&>);
  static_assert(!std::is_constructible_v<BranchInstruction, VoidType &&, BasicBlock &>);
  static_assert(!std::is_constructible_v<CondBranchInstruction, ValueRef, ValueRef, BasicBlock &&, BasicBlock &>);
  static_assert(!std::is_constructible_v<CondBranchInstruction, ValueRef, ValueRef, BasicBlock &, BasicBlock &&>);
  static_assert(!std::is_constructible_v<CondBranchInstruction, ValueRef, Argument &&, BasicBlock &, BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<BranchInstruction &>().getTarget()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const BranchInstruction &>().getTarget()), const BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const CondBranchInstruction &>().getCondition()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<CondBranchInstruction &>().getTrueTarget()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const CondBranchInstruction &>().getTrueTarget()), const BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<CondBranchInstruction &>().getFalseTarget()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const CondBranchInstruction &>().getFalseTarget()), const BasicBlock &>);
  static_assert(!std::is_copy_constructible_v<AddInstruction> && !std::is_copy_assignable_v<AddInstruction>);
  static_assert(!std::is_move_constructible_v<AddInstruction> && !std::is_move_assignable_v<AddInstruction>);
  static_assert(!std::is_copy_constructible_v<ReturnInstruction> && !std::is_copy_assignable_v<ReturnInstruction>);
  static_assert(!std::is_move_constructible_v<ReturnInstruction> && !std::is_move_assignable_v<ReturnInstruction>);
  static_assert(std::is_nothrow_constructible_v<AddInstruction, const IntegerConstant &, const IntegerConstant &>);
  static_assert(std::is_nothrow_constructible_v<AddInstruction, ValueRef, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<ReturnInstruction, const VoidType &>);
  static_assert(std::is_nothrow_constructible_v<ReturnInstruction, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<ReturnInstruction, const VoidType &, const AddInstruction &>);
  static_assert(std::is_nothrow_constructible_v<ReturnInstruction, const VoidType &, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<ReturnInstruction, ValueRef, ValueRef>);
  static_assert(std::is_same_v<ValueRef, std::reference_wrapper<const Value>>);
  static_assert(std::is_convertible_v<IntegerConstant &, ValueRef> && std::is_convertible_v<const IntegerConstant &, ValueRef>);
  static_assert(std::is_convertible_v<AddInstruction &, ValueRef> && std::is_convertible_v<const AddInstruction &, ValueRef>);

#define INSTRUCTION(EntryKind, Number, Class, Spelling) static_assert(Class::Kind == InstructionKind::EntryKind && Class::Kind == static_cast<InstructionKind>(Number));
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION

  template <typename Operand>
  constexpr bool RejectsTemporaryAddOperands = !std::is_constructible_v<AddInstruction, Operand &&, ValueRef> && !std::is_constructible_v<AddInstruction, ValueRef, Operand &&> && !std::is_constructible_v<AddInstruction, Operand &&, Operand &&>;

  static_assert(RejectsTemporaryAddOperands<IntegerConstant> && RejectsTemporaryAddOperands<const IntegerConstant>);
  static_assert(RejectsTemporaryAddOperands<AddInstruction> && RejectsTemporaryAddOperands<const AddInstruction>);
  static_assert(!std::is_constructible_v<ReturnInstruction, VoidType &&> && !std::is_constructible_v<ReturnInstruction, const VoidType &&>);
  static_assert(!std::is_constructible_v<ReturnInstruction, VoidType &&, const Value &> && !std::is_constructible_v<ReturnInstruction, const VoidType &&, const Value &>);
  static_assert(!std::is_constructible_v<ReturnInstruction, const VoidType &, AddInstruction &&> && !std::is_constructible_v<ReturnInstruction, const VoidType &, const AddInstruction &&>);
  static_assert(!std::is_constructible_v<ReturnInstruction, const VoidType &, IntegerConstant &&> && !std::is_constructible_v<ReturnInstruction, const VoidType &, const IntegerConstant &&>);
  static_assert(!std::is_constructible_v<ReturnInstruction, VoidType &&, AddInstruction &&> && !std::is_constructible_v<ReturnInstruction, const VoidType &&, const AddInstruction &&>);

  // Chained additions borrow pooled integer operands, while their return instruction has a separate void result type.
  TEST(IRInstructionTest, PreservesIntegerExpressionGraphAndReturnType)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *Twenty = Constants.getSignedInteger(Int32, 20);
    const IntegerConstant *TwentyTwo = Constants.getSignedInteger(Int32, 22);
    ASSERT_NE(Twenty, nullptr);
    ASSERT_NE(TwentyTwo, nullptr);
    const AddInstruction Sum(ValueRef(*Twenty), std::cref(*TwentyTwo));
    const AddInstruction ChainedSum(Sum, *Twenty);
    const ReturnInstruction Return(Types.getVoidType(), ChainedSum);
    const Value &ExpressionValue = ChainedSum;
    const Value &ReturnValue = Return;

    EXPECT_EQ(&Sum.getLHS(), Twenty);
    EXPECT_EQ(&Sum.getRHS(), TwentyTwo);
    EXPECT_EQ(&ChainedSum.getLHS(), &Sum);
    EXPECT_EQ(&ChainedSum.getRHS(), Twenty);
    EXPECT_EQ(&Sum.getType(), &Int32);
    EXPECT_EQ(&ExpressionValue.getType(), &Int32);
    EXPECT_EQ(Return.getReturnValue(), &ChainedSum);
    EXPECT_EQ(&ReturnValue.getType(), &Types.getVoidType());
    EXPECT_NE(&ReturnValue.getType(), &Return.getReturnValue()->getType());
    EXPECT_EQ(ChainedSum.getKind(), InstructionKind::Add);
    EXPECT_EQ(Return.getKind(), InstructionKind::Return);
    EXPECT_EQ(ChainedSum.getName(), "add");
    EXPECT_EQ(Return.getName(), "return");
    EXPECT_TRUE(AddInstruction::classof(&ChainedSum));
    EXPECT_FALSE(AddInstruction::classof(&Return));
    EXPECT_TRUE(ReturnInstruction::classof(&Return));
    EXPECT_FALSE(ReturnInstruction::classof(&ChainedSum));
    EXPECT_FALSE(AddInstruction::classof(nullptr));
    EXPECT_FALSE(ReturnInstruction::classof(nullptr));
  }

  // A return without a payload still borrows its void type and exposes no return operand.
  TEST(IRInstructionTest, RepresentsVoidReturnWithoutAnOperand)
  {
    TypePool Types;
    const ReturnInstruction Return(Types.getVoidType());
    const Instruction &Base = Return;
    EXPECT_EQ(Return.getReturnValue(), nullptr);
    EXPECT_EQ(&Base.getType(), &Types.getVoidType());
    EXPECT_EQ(Base.getKind(), InstructionKind::Return);
  }

  // Only returns and branches terminate a block; calls and other value or memory operations do not.
  TEST(IRInstructionTest, ClassifiesEveryTerminatorKind)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const FloatConstant &Value = Constants.getFloat(2.0f);
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &TrueTarget = FunctionValue.createBasicBlock();
    BasicBlock &FalseTarget = FunctionValue.createBasicBlock();
    const Argument &Condition = FunctionValue.argument(0);
    BranchInstruction Branch(Types.getVoidType(), TrueTarget);
    CondBranchInstruction CondBranch(Types.getVoidType(), Condition, TrueTarget, FalseTarget);
    const ReturnInstruction Return(Types.getVoidType());
    const Instruction *Terminators[] = {
        &Return,
        &Branch,
        &CondBranch,
    };
    for (const Instruction *Terminator : Terminators)
    {
      EXPECT_TRUE(Terminator->isTerminator());
    }
    const AddInstruction Add(Value, Value);
    const AllocaInstruction Alloca(Types.getPointerType(Value.getType()));
    const LoadInstruction Load(Value.getType(), Alloca);
    const StoreInstruction Store(Types.getVoidType(), Alloca, Value);
    const PhiInstruction Phi(Value.getType());
    const Function Callee(Types.getFunctionType(Types.getVoidType()));
    const CallInstruction Call(Callee);
    const Instruction *OtherInstructions[] = {
        &Add,
        &Alloca,
        &Load,
        &Store,
        &Phi,
        &Call,
    };
    for (const Instruction *Other : OtherInstructions)
    {
      EXPECT_FALSE(Other->isTerminator());
    }
  }

  // Floating additions retain their pooled operands and infer the floating result type without evaluating the expression.
  TEST(IRInstructionTest, ModelsFloatingAdditionWithBorrowedOperands)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const FloatConstant &LHS = Constants.getFloat(1.5f);
    const FloatConstant &RHS = Constants.getFloat(2.25f);
    const AddInstruction Sum(LHS, RHS);
    EXPECT_EQ(&Sum.getLHS(), &LHS);
    EXPECT_EQ(&Sum.getRHS(), &RHS);
    EXPECT_EQ(&Sum.getType(), &Types.getFloatType(FloatType::Width::Bits32));
    EXPECT_EQ(&Sum.getType(), &RHS.getType());
  }
} // namespace ink::ir::test
