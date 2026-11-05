#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/parameter.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <functional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_nothrow_constructible_v<LoadInstruction, const Type &, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<LoadInstruction, ValueRef, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<StoreInstruction, const VoidType &, ValueRef, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<StoreInstruction, ValueRef, ValueRef, ValueRef>);
  static_assert(std::is_constructible_v<LoadInstruction, const Type &, AllocaInstruction &> && std::is_constructible_v<LoadInstruction, const Type &, const AllocaInstruction &>);
  static_assert(std::is_constructible_v<StoreInstruction, const VoidType &, AllocaInstruction &, const IntegerConstant &> && std::is_constructible_v<StoreInstruction, const VoidType &, const AllocaInstruction &, IntegerConstant &>);
  static_assert(!std::is_constructible_v<LoadInstruction, IntegerType &&, ValueRef> && !std::is_constructible_v<LoadInstruction, const IntegerType &&, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<LoadInstruction, const Parameter &, ValueRef>);
  static_assert(!std::is_constructible_v<LoadInstruction, Parameter &&, ValueRef> && !std::is_constructible_v<LoadInstruction, const Parameter &&, ValueRef>);
  static_assert(!std::is_constructible_v<StoreInstruction, VoidType &&, ValueRef, ValueRef> && !std::is_constructible_v<StoreInstruction, const VoidType &&, ValueRef, ValueRef>);

  template <typename OperandType>
  constexpr bool RejectsTemporaryMemoryOperands = !std::is_constructible_v<LoadInstruction, const Type &, OperandType &&> && !std::is_constructible_v<StoreInstruction, const VoidType &, OperandType &&, ValueRef> && !std::is_constructible_v<StoreInstruction, const VoidType &, ValueRef, OperandType &&> && !requires(InstructionBuilder &Builder, OperandType &&Operand, ValueRef Other)
  {
    Builder.createLoad(std::forward<OperandType>(Operand));
  } && !requires(InstructionBuilder &Builder, OperandType &&Operand, ValueRef Other)
  {
    Builder.createStore(std::forward<OperandType>(Operand), Other);
  } && !requires(InstructionBuilder &Builder, OperandType &&Operand, ValueRef Other)
  {
    Builder.createStore(Other, std::forward<OperandType>(Operand));
  };

  static_assert(RejectsTemporaryMemoryOperands<AllocaInstruction> && RejectsTemporaryMemoryOperands<const AllocaInstruction>);
  static_assert(RejectsTemporaryMemoryOperands<IntegerConstant> && RejectsTemporaryMemoryOperands<const IntegerConstant>);

  // A block owns the complete store-load-add-return graph after its builder is destroyed, retaining each borrowed value and type.
  TEST(IRMemoryInstructionTest, BuildsOwnedIntegerMemoryExpressionInInstructionOrder)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *Twenty = Constants.getSignedInteger(Int32, 20);
    const IntegerConstant *TwentyTwo = Constants.getSignedInteger(Int32, 22);
    ASSERT_NE(Twenty, nullptr);
    ASSERT_NE(TwentyTwo, nullptr);
    Function FunctionValue(Types.getFunctionType(Int32));
    BasicBlock &Block = FunctionValue.createBasicBlock();
    AllocaInstruction *Slot = nullptr;
    StoreInstruction *Store = nullptr;
    LoadInstruction *Load = nullptr;
    AddInstruction *Sum = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Block);
      Slot = Builder.createAlloca(Int32);
      ASSERT_NE(Slot, nullptr);
      Store = Builder.createStore(*Slot, ValueRef(*Twenty));
      ASSERT_NE(Store, nullptr);
      Load = Builder.createLoad(std::cref(*Slot));
      ASSERT_NE(Load, nullptr);
      Sum = Builder.createAdd(*Load, *TwentyTwo);
      ASSERT_NE(Sum, nullptr);
      Return = Builder.createReturn(*Sum);
      ASSERT_NE(Return, nullptr);
    }

    EXPECT_EQ(&Store->getDestination(), Slot);
    EXPECT_EQ(&Store->getSource(), Twenty);
    EXPECT_EQ(&Store->getType(), &Types.getVoidType());
    EXPECT_EQ(&Load->getSource(), Slot);
    EXPECT_EQ(&Load->getType(), &Int32);
    EXPECT_EQ(&Sum->getLHS(), Load);
    EXPECT_EQ(Return->getReturnValue(), Sum);
    EXPECT_EQ(Load->getName(), "load");
    EXPECT_EQ(Store->getName(), "store");
    EXPECT_TRUE(LoadInstruction::classof(Load));
    EXPECT_FALSE(LoadInstruction::classof(Store));
    EXPECT_FALSE(LoadInstruction::classof(nullptr));
    EXPECT_TRUE(StoreInstruction::classof(Store));
    EXPECT_FALSE(StoreInstruction::classof(Load));
    EXPECT_FALSE(StoreInstruction::classof(nullptr));
    const Instruction *Expected[] = {
        Slot,
        Store,
        Load,
        Sum,
        Return,
    };
    ASSERT_EQ(Block.instructionCount(), 5u);
    auto Iterator = Block.instructions().begin();
    for (const Instruction *ExpectedInstruction : Expected)
    {
      EXPECT_EQ(&*Iterator++, ExpectedInstruction);
      EXPECT_EQ(ExpectedInstruction->parent(), &Block);
    }
    EXPECT_EQ(Iterator, Block.instructions().end());
  }

  // Storing an address in a reference-valued slot preserves both reference layers, and each load removes exactly one layer.
  TEST(IRMemoryInstructionTest, LoadsNestedReferenceStorageOneLayerAtATime)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ReferenceType &Int32Reference = Types.getReferenceType(Int32);
    const IntegerConstant Value(Int32, 42);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    AllocaInstruction *IntegerSlot = Builder.createAlloca(Int32);
    AllocaInstruction *ReferenceSlot = Builder.createAlloca(Int32Reference);
    ASSERT_NE(IntegerSlot, nullptr);
    ASSERT_NE(ReferenceSlot, nullptr);
    ASSERT_NE(Builder.createStore(*IntegerSlot, Value), nullptr);
    StoreInstruction *AddressStore = Builder.createStore(*ReferenceSlot, *IntegerSlot);
    ASSERT_NE(AddressStore, nullptr);
    LoadInstruction *LoadedAddress = Builder.createLoad(*ReferenceSlot);
    ASSERT_NE(LoadedAddress, nullptr);
    LoadInstruction *LoadedInteger = Builder.createLoad(*LoadedAddress);
    ASSERT_NE(LoadedInteger, nullptr);
    EXPECT_EQ(&ReferenceSlot->getType(), &Types.getReferenceType(Int32Reference));
    EXPECT_EQ(&AddressStore->getDestination(), ReferenceSlot);
    EXPECT_EQ(&AddressStore->getSource(), IntegerSlot);
    EXPECT_EQ(&LoadedAddress->getType(), &Int32Reference);
    EXPECT_EQ(&LoadedAddress->getSource(), ReferenceSlot);
    EXPECT_EQ(&LoadedInteger->getType(), &Int32);
    EXPECT_EQ(&LoadedInteger->getSource(), LoadedAddress);
    EXPECT_NE(&LoadedAddress->getType(), &LoadedInteger->getType());
  }

  // Missing insertion points, non-reference addresses, and unequal source type identities fail without changing the block or anchor.
  TEST(IRMemoryInstructionTest, RejectsInvalidMemoryCreationWithoutMutatingInsertionState)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType SeparateInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant CanonicalValue(Int32, 20);
    const IntegerConstant SeparateValue(SeparateInt32, 20);
    const AllocaInstruction DetachedSlot(Types.getReferenceType(Int32));
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    EXPECT_EQ(Builder.createLoad(DetachedSlot), nullptr);
    EXPECT_EQ(Builder.createStore(DetachedSlot, CanonicalValue), nullptr);
    EXPECT_EQ(Block.instructionCount(), 0u);
    Builder.setInsertPoint(Block);
    AllocaInstruction *Slot = Builder.createAlloca(Int32);
    ReturnInstruction *Anchor = Builder.createReturn();
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Anchor, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    EXPECT_EQ(Builder.createLoad(CanonicalValue), nullptr);
    EXPECT_EQ(Builder.createStore(CanonicalValue, CanonicalValue), nullptr);
    EXPECT_EQ(Builder.createStore(*Slot, SeparateValue), nullptr);
    EXPECT_EQ(Builder.createStore(*Slot, DetachedSlot), nullptr);
    EXPECT_EQ(Block.instructionCount(), 2u);
    EXPECT_EQ(Builder.getInsertBlock(), &Block);
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);
    StoreInstruction *ValidStore = Builder.createStore(*Slot, CanonicalValue);
    ASSERT_NE(ValidStore, nullptr);
    auto Iterator = Block.instructions().begin();
    EXPECT_EQ(&*Iterator++, Slot);
    EXPECT_EQ(&*Iterator++, ValidStore);
    EXPECT_EQ(&*Iterator++, Anchor);
    EXPECT_EQ(Iterator, Block.instructions().end());

    Builder.clearInsertPoint();
    EXPECT_EQ(Builder.createLoad(*Slot), nullptr);
    EXPECT_EQ(Builder.createStore(*Slot, CanonicalValue), nullptr);
    EXPECT_EQ(Block.instructionCount(), 3u);
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
  }

  // Generic storage loads and stores use the exact type parameter identity, even when distinct parameters have the same name.
  TEST(IRMemoryInstructionTest, PreservesGenericTypesAcrossMemoryOperationsAndRejectsOtherParameters)
  {
    const auto TypeName = core::NamePool::instance().intern("T");
    const Parameter T(MetaType::instance(), TypeName);
    const Parameter OtherT(MetaType::instance(), TypeName);
    const Parameter ValueOfT(std::cref(T));
    const Parameter ValueOfOtherT(std::cref(OtherT));
    TypePool Types;
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    AllocaInstruction *Slot = Builder.createAlloca(T);
    ASSERT_NE(Slot, nullptr);
    StoreInstruction *Store = Builder.createStore(*Slot, ValueOfT);
    ASSERT_NE(Store, nullptr);
    LoadInstruction *Load = Builder.createLoad(*Slot);
    ASSERT_NE(Load, nullptr);
    ReturnInstruction *Return = Builder.createReturn(*Load);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(&Slot->getAllocatedType(), &T);
    EXPECT_EQ(&Load->getType(), &T);
    EXPECT_EQ(&Store->getSource(), &ValueOfT);
    EXPECT_EQ(Return->getReturnValue(), Load);
    ASSERT_TRUE(Builder.setInsertPoint(*Return));
    EXPECT_EQ(Builder.createStore(*Slot, ValueOfOtherT), nullptr);
    EXPECT_EQ(Builder.createStore(ValueOfT, ValueOfT), nullptr);
    EXPECT_EQ(Builder.createLoad(ValueOfT), nullptr);
    EXPECT_EQ(Block.instructionCount(), 4u);
    EXPECT_EQ(Builder.getInsertBefore(), Return);
    StoreInstruction *StoreLoaded = Builder.createStore(*Slot, *Load);
    ASSERT_NE(StoreLoaded, nullptr);
    EXPECT_EQ(&StoreLoaded->getDestination(), Slot);
    EXPECT_EQ(&StoreLoaded->getSource(), Load);
    const Instruction *Expected[] = {
        Slot,
        Store,
        Load,
        StoreLoaded,
        Return,
    };
    auto Iterator = Block.instructions().begin();
    for (const Instruction *ExpectedInstruction : Expected)
    {
      ASSERT_NE(Iterator, Block.instructions().end());
      EXPECT_EQ(&*Iterator++, ExpectedInstruction);
    }
    EXPECT_EQ(Iterator, Block.instructions().end());
  }
} // namespace ink::ir::test
