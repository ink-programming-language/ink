#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/argument.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/instruction_builder.h"
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
  static_assert(std::is_nothrow_constructible_v<AllocaInstruction, const PointerType &>);
  static_assert(std::is_nothrow_constructible_v<AllocaInstruction, ValueRef> && std::is_nothrow_constructible_v<AllocaInstruction, const Value &>);
  static_assert(!std::is_constructible_v<AllocaInstruction, PointerType &&> && !std::is_constructible_v<AllocaInstruction, const PointerType &&>);
  static_assert(!std::is_constructible_v<AllocaInstruction, Argument &&> && !std::is_constructible_v<AllocaInstruction, const Argument &&>);
  static_assert(std::is_same_v<decltype(std::declval<const AllocaInstruction &>().getAllocatedType()), const Value &>);

  template <typename AllocatedType>
  constexpr bool BuilderAcceptsAllocatedType = requires(InstructionBuilder &Builder, AllocatedType &&TypeValue)
  {
    Builder.createAlloca(std::forward<AllocatedType>(TypeValue));
  };

  static_assert(BuilderAcceptsAllocatedType<IntegerType &> && BuilderAcceptsAllocatedType<const PointerType &>);
  static_assert(BuilderAcceptsAllocatedType<Argument &> && BuilderAcceptsAllocatedType<const Argument &>);
  static_assert(!BuilderAcceptsAllocatedType<IntegerType> && !BuilderAcceptsAllocatedType<const IntegerType>);
  static_assert(!BuilderAcceptsAllocatedType<PointerType> && !BuilderAcceptsAllocatedType<const PointerType>);
  static_assert(!BuilderAcceptsAllocatedType<Argument> && !BuilderAcceptsAllocatedType<const Argument>);

  namespace
  {
    constexpr IntegerType AllocatedInteger(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    constexpr PointerType AllocationResult(AllocatedInteger);
    constexpr AllocaInstruction CompileTimeAllocation(AllocationResult);
    static_assert(&CompileTimeAllocation.getAllocatedType() == &AllocatedInteger);
    static_assert(PointerType::classof(&CompileTimeAllocation.getType()));
    static_assert(&CompileTimeAllocation.getType() == &AllocationResult);
  } // namespace

  // Arrays and pointer-valued slots borrow their complete allocated type and add exactly one pointer layer to their result.
  TEST(IRAllocaInstructionTest, PreservesArrayAndPointerAllocatedTypeIdentities)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ArrayType &Array = Types.getArrayType(Int32, 4);
    const PointerType &ArrayResult = Types.getPointerType(Array);
    const PointerType &PointerValue = Types.getPointerType(Int32);
    const PointerType &PointerResult = Types.getPointerType(PointerValue);
    const AllocaInstruction ArraySlot(ArrayResult);
    const AllocaInstruction PointerSlot(PointerResult);
    EXPECT_EQ(&ArraySlot.getType(), &ArrayResult);
    EXPECT_EQ(&ArraySlot.getAllocatedType(), &Array);
    EXPECT_EQ(&ArrayResult.pointeeType(), &Array);
    EXPECT_EQ(&PointerSlot.getType(), &PointerResult);
    EXPECT_EQ(&PointerSlot.getAllocatedType(), &PointerValue);
    EXPECT_EQ(&PointerResult.pointeeType(), &PointerValue);
    EXPECT_EQ(&PointerValue.pointeeType(), &Int32);
    EXPECT_NE(&PointerSlot.getType(), &PointerSlot.getAllocatedType());
    EXPECT_EQ(ArraySlot.getKind(), InstructionKind::Alloca);
    EXPECT_EQ(ArraySlot.getName(), "alloca");
    EXPECT_TRUE(AllocaInstruction::classof(&PointerSlot));
    EXPECT_FALSE(AllocaInstruction::classof(nullptr));
  }

  // Distinct block-owned allocations share a canonical pointer type and can be inserted before an existing return.
  TEST(IRAllocaInstructionTest, BuildsDistinctSlotsBeforeReturnWithSharedCanonicalResultType)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const PointerType &SlotType = Types.getPointerType(Int32);
    Function FunctionValue(Types.getFunctionType(Types.getVoidType()));
    BasicBlock &Block = FunctionValue.createBasicBlock();
    AllocaInstruction *FirstSlot = nullptr;
    AllocaInstruction *SecondSlot = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Block);
      FirstSlot = Builder.createAlloca(Int32);
      ASSERT_NE(FirstSlot, nullptr);
      Return = Builder.createReturn();
      ASSERT_NE(Return, nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Return));
      SecondSlot = Builder.createAlloca(Int32);
      ASSERT_NE(SecondSlot, nullptr);
    }

    EXPECT_NE(FirstSlot, SecondSlot);
    EXPECT_EQ(&FirstSlot->getType(), &SlotType);
    EXPECT_TRUE(PointerType::classof(&FirstSlot->getType()));
    EXPECT_EQ(&SecondSlot->getType(), &SlotType);
    EXPECT_EQ(&FirstSlot->getAllocatedType(), &Int32);
    EXPECT_EQ(&SecondSlot->getAllocatedType(), &Int32);
    EXPECT_EQ(FirstSlot->parent(), &Block);
    EXPECT_EQ(SecondSlot->parent(), &Block);
    EXPECT_EQ(Return->getReturnValue(), nullptr);
    EXPECT_EQ(&FunctionValue.signature().returnType(), &Types.getVoidType());
    ASSERT_EQ(Block.instructionCount(), 3u);
    auto Iterator = Block.instructions().begin();
    EXPECT_EQ(&*Iterator++, FirstSlot);
    EXPECT_EQ(&*Iterator++, SecondSlot);
    EXPECT_EQ(&*Iterator++, Return);
    EXPECT_EQ(Iterator, Block.instructions().end());
  }

  // Generic allocation preserves its type-valued argument, while allocating MetaType creates a separate slot for type values.
  TEST(IRAllocaInstructionTest, DistinguishesGenericStorageFromStorageForTypeValues)
  {
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    const Argument &T = Owner.argument(0);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    AllocaInstruction *GenericSlot = Builder.createAlloca(T);
    AllocaInstruction *TypeSlot = Builder.createAlloca(Types.getMetaType());
    ASSERT_NE(GenericSlot, nullptr);
    ASSERT_NE(TypeSlot, nullptr);
    EXPECT_EQ(&GenericSlot->getAllocatedType(), &T);
    EXPECT_EQ(&GenericSlot->getType(), &Types.getPointerType(T));
    EXPECT_EQ(&TypeSlot->getAllocatedType(), &Types.getMetaType());
    EXPECT_EQ(&TypeSlot->getType(), &Types.getPointerType(Types.getMetaType()));
    EXPECT_NE(&GenericSlot->getType(), &TypeSlot->getType());
    EXPECT_EQ(&GenericSlot->getAllocatedType().getType(), &Types.getMetaType());
    EXPECT_EQ(&TypeSlot->getAllocatedType().getType(), &Types.getMetaType());
    const Value &ErasedResultType = Types.getPointerType(T);
    const AllocaInstruction DirectSlot(ErasedResultType);
    EXPECT_EQ(&DirectSlot.getType(), &ErasedResultType);
    EXPECT_EQ(&DirectSlot.getAllocatedType(), &T);
  }

  // Non-type operands cannot create an allocation or disturb an existing insertion anchor.
  TEST(IRAllocaInstructionTest, RejectsOrdinaryValuesWithoutChangingTheInsertionPoint)
  {
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    const Argument &T = Owner.argument(0);
    Function Values(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(T)}));
    const Argument &GenericValue = Values.argument(0);
    const IntegerConstant IntegerValue(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 42);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    ReturnInstruction *Anchor = Builder.createReturn();
    ASSERT_NE(Anchor, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    EXPECT_EQ(Builder.createAlloca(IntegerValue), nullptr);
    EXPECT_EQ(Builder.createAlloca(GenericValue), nullptr);
    EXPECT_EQ(Builder.getInsertBlock(), &Block);
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);
    EXPECT_EQ(Block.instructionCount(), 1u);
    AllocaInstruction *Slot = Builder.createAlloca(T);
    ASSERT_NE(Slot, nullptr);
    auto Iterator = Block.instructions().begin();
    EXPECT_EQ(&*Iterator++, Slot);
    EXPECT_EQ(&*Iterator++, Anchor);
    EXPECT_EQ(Iterator, Block.instructions().end());
  }
} // namespace ink::ir::test
