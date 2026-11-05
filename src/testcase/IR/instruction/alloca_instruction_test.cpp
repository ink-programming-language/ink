#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/parameter.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <functional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_nothrow_constructible_v<AllocaInstruction, const ReferenceType &>);
  static_assert(std::is_nothrow_constructible_v<AllocaInstruction, ValueRef> && std::is_nothrow_constructible_v<AllocaInstruction, const Value &>);
  static_assert(!std::is_constructible_v<AllocaInstruction, ReferenceType &&> && !std::is_constructible_v<AllocaInstruction, const ReferenceType &&>);
  static_assert(!std::is_constructible_v<AllocaInstruction, Parameter &&> && !std::is_constructible_v<AllocaInstruction, const Parameter &&>);
  static_assert(std::is_same_v<decltype(std::declval<const AllocaInstruction &>().getAllocatedType()), const Value &>);

  template <typename AllocatedType>
  constexpr bool BuilderAcceptsAllocatedType = requires(InstructionBuilder &Builder, AllocatedType &&TypeValue)
  {
    Builder.createAlloca(std::forward<AllocatedType>(TypeValue));
  };

  static_assert(BuilderAcceptsAllocatedType<IntegerType &> && BuilderAcceptsAllocatedType<const ReferenceType &>);
  static_assert(BuilderAcceptsAllocatedType<Parameter &> && BuilderAcceptsAllocatedType<const Parameter &>);
  static_assert(!BuilderAcceptsAllocatedType<IntegerType> && !BuilderAcceptsAllocatedType<const IntegerType>);
  static_assert(!BuilderAcceptsAllocatedType<ReferenceType> && !BuilderAcceptsAllocatedType<const ReferenceType>);
  static_assert(!BuilderAcceptsAllocatedType<Parameter> && !BuilderAcceptsAllocatedType<const Parameter>);

  namespace
  {
    constexpr IntegerType AllocatedInteger(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    constexpr ReferenceType AllocationResult(AllocatedInteger);
    constexpr AllocaInstruction CompileTimeAllocation(AllocationResult);
    static_assert(&CompileTimeAllocation.getAllocatedType() == &AllocatedInteger);
  } // namespace

  // Arrays and reference-valued slots borrow their complete allocated type and add exactly one reference layer to their result.
  TEST(IRAllocaInstructionTest, PreservesArrayAndReferenceAllocatedTypeIdentities)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ArrayType &Array = Types.getArrayType(Int32, 4);
    const ReferenceType &ArrayResult = Types.getReferenceType(Array);
    const ReferenceType &ReferenceValue = Types.getReferenceType(Int32);
    const ReferenceType &ReferenceResult = Types.getReferenceType(ReferenceValue);
    const AllocaInstruction ArraySlot(ArrayResult);
    const AllocaInstruction ReferenceSlot(ReferenceResult);
    EXPECT_EQ(&ArraySlot.getType(), &ArrayResult);
    EXPECT_EQ(&ArraySlot.getAllocatedType(), &Array);
    EXPECT_EQ(&ArrayResult.referencedType(), &Array);
    EXPECT_EQ(&ReferenceSlot.getType(), &ReferenceResult);
    EXPECT_EQ(&ReferenceSlot.getAllocatedType(), &ReferenceValue);
    EXPECT_EQ(&ReferenceResult.referencedType(), &ReferenceValue);
    EXPECT_EQ(&ReferenceValue.referencedType(), &Int32);
    EXPECT_NE(&ReferenceSlot.getType(), &ReferenceSlot.getAllocatedType());
    EXPECT_EQ(ArraySlot.getKind(), InstructionKind::Alloca);
    EXPECT_EQ(ArraySlot.getName(), "alloca");
    EXPECT_TRUE(AllocaInstruction::classof(&ReferenceSlot));
    EXPECT_FALSE(AllocaInstruction::classof(nullptr));
  }

  // Distinct block-owned allocations share a canonical reference type and can be inserted before an existing return.
  TEST(IRAllocaInstructionTest, BuildsDistinctSlotsBeforeReturnWithSharedCanonicalResultType)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ReferenceType &SlotType = Types.getReferenceType(Int32);
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

  // Generic allocation preserves its parameter operand, while allocating MetaType creates a separate slot for type values.
  TEST(IRAllocaInstructionTest, DistinguishesGenericStorageFromStorageForTypeValues)
  {
    const Parameter T(MetaType::instance());
    TypePool Types;
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    AllocaInstruction *GenericSlot = Builder.createAlloca(T);
    AllocaInstruction *TypeSlot = Builder.createAlloca(Types.getMetaType());
    ASSERT_NE(GenericSlot, nullptr);
    ASSERT_NE(TypeSlot, nullptr);
    EXPECT_EQ(&GenericSlot->getAllocatedType(), &T);
    EXPECT_EQ(&GenericSlot->getType(), &Types.getReferenceType(T));
    EXPECT_EQ(&TypeSlot->getAllocatedType(), &Types.getMetaType());
    EXPECT_EQ(&TypeSlot->getType(), &Types.getReferenceType(Types.getMetaType()));
    EXPECT_NE(&GenericSlot->getType(), &TypeSlot->getType());
    EXPECT_EQ(&GenericSlot->getAllocatedType().getType(), &Types.getMetaType());
    EXPECT_EQ(&TypeSlot->getAllocatedType().getType(), &Types.getMetaType());
    const Value &ErasedResultType = Types.getReferenceType(T);
    const AllocaInstruction DirectSlot(ErasedResultType);
    EXPECT_EQ(&DirectSlot.getType(), &ErasedResultType);
    EXPECT_EQ(&DirectSlot.getAllocatedType(), &T);
  }

  // Non-type operands cannot create an allocation or disturb an existing insertion anchor.
  TEST(IRAllocaInstructionTest, RejectsOrdinaryValuesWithoutChangingTheInsertionPoint)
  {
    const Parameter T(MetaType::instance());
    const Parameter GenericValue(std::cref(T));
    TypePool Types;
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
