#include "ink/ir/function/function.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_nothrow_constructible_v<AllocaInstruction, const ReferenceType &>);
  static_assert(!std::is_constructible_v<AllocaInstruction, ReferenceType &&> && !std::is_constructible_v<AllocaInstruction, const ReferenceType &&>);

  template <typename AllocatedType>
  constexpr bool BuilderAcceptsAllocatedType = requires(InstructionBuilder &Builder, AllocatedType &&TypeValue)
  {
    Builder.createAlloca(std::forward<AllocatedType>(TypeValue));
  };

  static_assert(BuilderAcceptsAllocatedType<IntegerType &> && BuilderAcceptsAllocatedType<const ReferenceType &>);
  static_assert(!BuilderAcceptsAllocatedType<IntegerType> && !BuilderAcceptsAllocatedType<const IntegerType>);
  static_assert(!BuilderAcceptsAllocatedType<ReferenceType> && !BuilderAcceptsAllocatedType<const ReferenceType>);

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
} // namespace ink::ir::test
