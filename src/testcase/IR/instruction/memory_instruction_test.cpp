#include "ink/core/support/core_define.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/argument.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/function/parameter.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_nothrow_constructible_v<LoadInstruction, const Type &, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<LoadInstruction, ValueRef, ValueRef>);
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createLoad(std::declval<ValueRef>())), LoadInstruction &>);
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createStore(std::declval<ValueRef>(), std::declval<ValueRef>())), StoreInstruction &>);
  static_assert(std::is_nothrow_constructible_v<StoreInstruction, const VoidType &, ValueRef, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<StoreInstruction, ValueRef, ValueRef, ValueRef>);
  static_assert(std::is_constructible_v<LoadInstruction, const Type &, AllocaInstruction &> && std::is_constructible_v<LoadInstruction, const Type &, const AllocaInstruction &>);
  static_assert(std::is_constructible_v<StoreInstruction, const VoidType &, AllocaInstruction &, const IntegerConstant &> && std::is_constructible_v<StoreInstruction, const VoidType &, const AllocaInstruction &, IntegerConstant &>);
  static_assert(!std::is_constructible_v<LoadInstruction, IntegerType &&, ValueRef> && !std::is_constructible_v<LoadInstruction, const IntegerType &&, ValueRef>);
  static_assert(std::is_nothrow_constructible_v<LoadInstruction, const Argument &, ValueRef>);
  static_assert(!std::is_constructible_v<LoadInstruction, Argument &&, ValueRef> && !std::is_constructible_v<LoadInstruction, const Argument &&, ValueRef>);
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
  static_assert(RejectsTemporaryMemoryOperands<Argument> && RejectsTemporaryMemoryOperands<const Argument>);

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
      Slot = &Builder.createAlloca(Int32);
      Store = &Builder.createStore(*Slot, ValueRef(*Twenty));
      Load = &Builder.createLoad(std::cref(*Slot));
      Sum = &Builder.createAdd(*Load, *TwentyTwo);
      Return = &Builder.createReturn(*Sum);
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

  // Storing an address in a pointer-valued slot preserves both pointer layers, and each load removes exactly one layer.
  TEST(IRMemoryInstructionTest, LoadsNestedPointerStorageOneLayerAtATime)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const PointerType &Int32Pointer = Types.getPointerType(Int32);
    const IntegerConstant Value(Int32, 42);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    AllocaInstruction *IntegerSlot = &Builder.createAlloca(Int32);
    AllocaInstruction *PointerSlot = &Builder.createAlloca(Int32Pointer);
    static_cast<void>(Builder.createStore(*IntegerSlot, Value));
    StoreInstruction *AddressStore = &Builder.createStore(*PointerSlot, *IntegerSlot);
    LoadInstruction *LoadedAddress = &Builder.createLoad(*PointerSlot);
    LoadInstruction *LoadedInteger = &Builder.createLoad(*LoadedAddress);
    EXPECT_EQ(&PointerSlot->getType(), &Types.getPointerType(Int32Pointer));
    EXPECT_EQ(&AddressStore->getDestination(), PointerSlot);
    EXPECT_EQ(&AddressStore->getSource(), IntegerSlot);
    EXPECT_EQ(&LoadedAddress->getType(), &Int32Pointer);
    EXPECT_EQ(&LoadedAddress->getSource(), PointerSlot);
    EXPECT_EQ(&LoadedInteger->getType(), &Int32);
    EXPECT_EQ(&LoadedInteger->getSource(), LoadedAddress);
    EXPECT_NE(&LoadedAddress->getType(), &LoadedInteger->getType());
  }

  // Valid stores preserve the insertion anchor, while assertions reject missing insertion points and incompatible memory operands.
  TEST(IRMemoryInstructionTest, PreservesValidInsertionStateAndAssertsOnInvalidMemoryOperands)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant CanonicalValue(Int32, 20);
#if INK_ENABLE_ASSERTIONS
    const IntegerType SeparateInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant SeparateValue(SeparateInt32, 20);
    const AllocaInstruction DetachedSlot(Types.getPointerType(Int32));
#endif
    BasicBlock Block;
    InstructionBuilder Builder(Types);
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(DetachedSlot)), "InstructionBuilder requires an insertion point");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(DetachedSlot, CanonicalValue)), "InstructionBuilder requires an insertion point");
#endif
    EXPECT_EQ(Block.instructionCount(), 0u);
    Builder.setInsertPoint(Block);
    AllocaInstruction *Slot = &Builder.createAlloca(Int32);
    ReturnInstruction *Anchor = &Builder.createReturn();
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(CanonicalValue)), "Load requires a pointer source");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(CanonicalValue, CanonicalValue)), "Store requires a pointer destination");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Slot, SeparateValue)), "Store source type must match the destination pointee type");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Slot, DetachedSlot)), "Store source type must match the destination pointee type");
#endif
    EXPECT_EQ(Block.instructionCount(), 2u);
    EXPECT_EQ(Builder.getInsertBlock(), &Block);
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);
    StoreInstruction *ValidStore = &Builder.createStore(*Slot, CanonicalValue);
    auto Iterator = Block.instructions().begin();
    EXPECT_EQ(&*Iterator++, Slot);
    EXPECT_EQ(&*Iterator++, ValidStore);
    EXPECT_EQ(&*Iterator++, Anchor);
    EXPECT_EQ(Iterator, Block.instructions().end());

    Builder.clearInsertPoint();
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Slot)), "InstructionBuilder requires an insertion point");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Slot, CanonicalValue)), "InstructionBuilder requires an insertion point");
#endif
    EXPECT_EQ(Block.instructionCount(), 3u);
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
  }

  // Generic storage loads and stores use the exact type argument identity, even when distinct arguments have the same name.
  TEST(IRMemoryInstructionTest, PreservesGenericTypesAcrossMemoryOperationsAndRejectsOtherArguments)
  {
    const auto TypeName = core::NamePool::instance().intern("T");
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType()), Parameter(Types.getMetaType())}));
    Argument &T = Owner.argument(0);
    Argument &OtherT = Owner.argument(1);
    T.setName(TypeName);
    OtherT.setName(TypeName);
    Function Values(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(T), Parameter(OtherT)}));
    const Argument &ValueOfT = Values.argument(0);
    const Argument &ValueOfOtherT = Values.argument(1);
    EXPECT_EQ(T.name(), OtherT.name());
    EXPECT_NE(&ValueOfT.getType(), &ValueOfOtherT.getType());
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    AllocaInstruction *Slot = &Builder.createAlloca(T);
    StoreInstruction *Store = &Builder.createStore(*Slot, ValueOfT);
    LoadInstruction *Load = &Builder.createLoad(*Slot);
    ReturnInstruction *Return = &Builder.createReturn(*Load);
    EXPECT_EQ(&Slot->getAllocatedType(), &T);
    EXPECT_EQ(&Load->getType(), &T);
    EXPECT_EQ(&Store->getSource(), &ValueOfT);
    EXPECT_EQ(Return->getReturnValue(), Load);
    ASSERT_TRUE(Builder.setInsertPoint(*Return));
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Slot, ValueOfOtherT)), "Store source type must match the destination pointee type");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(ValueOfT, ValueOfT)), "Store requires a pointer destination");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(ValueOfT)), "Load requires a pointer source");
#endif
    EXPECT_EQ(Block.instructionCount(), 4u);
    EXPECT_EQ(Builder.getInsertBefore(), Return);
    StoreInstruction *StoreLoaded = &Builder.createStore(*Slot, *Load);
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
