#include "ink/core/support/core_define.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createAdd(std::declval<ValueRef>(), std::declval<ValueRef>())), AddInstruction &>);
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createReturn()), ReturnInstruction &>);
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createReturn(std::declval<ValueRef>())), ReturnInstruction &>);

  // Add creation returns owned references for integer and floating operands sharing the exact same concrete type value.
  TEST(IRBuilderContractTest, BuildsIntegerAndFloatingAddsWithExactOperandTypeIdentity)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FloatType &Float32 = Types.getFloatType(FloatType::Width::Bits32);
    const std::array Parameters{Parameter(Int32), Parameter(Int32), Parameter(Float32), Parameter(Float32)};
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), Parameters));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    AddInstruction &IntegerAdd = Builder.createAdd(FunctionValue.argument(0), FunctionValue.argument(1));
    AddInstruction &FloatAdd = Builder.createAdd(FunctionValue.argument(2), FunctionValue.argument(3));
    EXPECT_EQ(&IntegerAdd.getType(), &Int32);
    EXPECT_EQ(&FloatAdd.getType(), &Float32);
    EXPECT_EQ(&IntegerAdd.getLHS(), &FunctionValue.argument(0));
    EXPECT_EQ(&IntegerAdd.getRHS(), &FunctionValue.argument(1));
    EXPECT_EQ(&FloatAdd.getLHS(), &FunctionValue.argument(2));
    EXPECT_EQ(&FloatAdd.getRHS(), &FunctionValue.argument(3));
    EXPECT_EQ(IntegerAdd.parent(), &Entry);
    EXPECT_EQ(FloatAdd.parent(), &Entry);
  }

  // Void, concrete, and symbolic returns preserve their enclosing function's exact declared return type.
  TEST(IRBuilderContractTest, ReturnsVoidConcreteAndGenericValuesThroughOwnedReferences)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Function TypeOwner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    const Argument &T = TypeOwner.argument(0);
    Function VoidFunction(Types.getFunctionType(Types.getVoidType()));
    Function IntegerFunction(Types.getFunctionType(Int32, std::array{Parameter(Int32)}));
    Function GenericFunction(Types.getFunctionType(T, std::array{Parameter(T)}));
    InstructionBuilder Builder(Types);
    BasicBlock &VoidEntry = VoidFunction.createBasicBlock();
    Builder.setInsertPoint(VoidEntry);
    ReturnInstruction &VoidReturn = Builder.createReturn();
    EXPECT_EQ(VoidReturn.getReturnValue(), nullptr);
    EXPECT_EQ(VoidEntry.terminator(), &VoidReturn);
    BasicBlock &IntegerEntry = IntegerFunction.createBasicBlock();
    Builder.setInsertPoint(IntegerEntry);
    ReturnInstruction &IntegerReturn = Builder.createReturn(IntegerFunction.argument(0));
    EXPECT_EQ(IntegerReturn.getReturnValue(), &IntegerFunction.argument(0));
    EXPECT_EQ(IntegerEntry.terminator(), &IntegerReturn);
    BasicBlock &GenericEntry = GenericFunction.createBasicBlock();
    Builder.setInsertPoint(GenericEntry);
    ReturnInstruction &GenericReturn = Builder.createReturn(GenericFunction.argument(0));
    EXPECT_EQ(GenericReturn.getReturnValue(), &GenericFunction.argument(0));
    EXPECT_EQ(&GenericReturn.getReturnValue()->getType(), &T);
    EXPECT_EQ(GenericEntry.terminator(), &GenericReturn);
  }

#if INK_ENABLE_ASSERTIONS
  // Equal numeric categories are insufficient when operands differ by category, width, or borrowed type-object identity.
  TEST(IRBuilderContractDeathTest, RejectsAddOperandsWithDifferentTypeValues)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType &Int64 = Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Signed);
    const FloatType &Float32 = Types.getFloatType(FloatType::Width::Bits32);
    const IntegerType OtherInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Int32), Parameter(Int64), Parameter(Float32), Parameter(OtherInt32)};
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), Parameters));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(0), FunctionValue.argument(2))), "Add operands must have the same type");
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(0), FunctionValue.argument(1))), "Add operands must have the same type");
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(0), FunctionValue.argument(3))), "Add operands must have the same type");
  }

  // Boolean, pointer, metatype, and unresolved symbolic operands cannot satisfy Add's concrete numeric type contract.
  TEST(IRBuilderContractDeathTest, RejectsNonNumericAndUnresolvedAddOperandTypes)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Function TypeOwner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    const Argument &T = TypeOwner.argument(0);
    const std::array Parameters{Parameter(Types.getBoolType()), Parameter(Types.getPointerType(Int32)), Parameter(Types.getMetaType()), Parameter(T)};
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), Parameters));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(0), FunctionValue.argument(0))), "Add requires integer or floating-point operands");
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(1), FunctionValue.argument(1))), "Add requires integer or floating-point operands");
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(2), FunctionValue.argument(2))), "Add requires integer or floating-point operands");
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(FunctionValue.argument(3), FunctionValue.argument(3))), "Add requires integer or floating-point operands");
  }

  // Returning no value from a value-returning function or a mismatched concrete value violates the declared return contract.
  TEST(IRBuilderContractDeathTest, RejectsMissingAndMismatchedConcreteReturnValues)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType OtherInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Types.getBoolType()), Parameter(OtherInt32)};
    Function FunctionValue(Types.getFunctionType(Int32, Parameters));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    EXPECT_DEATH(static_cast<void>(Builder.createReturn()), "Void return requires a void function return type");
    EXPECT_DEATH(static_cast<void>(Builder.createReturn(FunctionValue.argument(0))), "Return value type must match the function return type");
    EXPECT_DEATH(static_cast<void>(Builder.createReturn(FunctionValue.argument(1))), "Return value type must match the function return type");
  }

  // Distinct MetaType argument identities remain different symbolic return types even when both are unbound generic parameters.
  TEST(IRBuilderContractDeathTest, RejectsReturnValuesFromAnotherGenericTypeIdentity)
  {
    TypePool Types;
    Function TypeOwner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType()), Parameter(Types.getMetaType())}));
    const Argument &T = TypeOwner.argument(0);
    const Argument &OtherT = TypeOwner.argument(1);
    Function FunctionValue(Types.getFunctionType(T, std::array{Parameter(OtherT)}));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    EXPECT_DEATH(static_cast<void>(Builder.createReturn(FunctionValue.argument(0))), "Return value type must match the function return type");
  }

  // Void-typed instructions cannot serve as a return payload, including when building a detached block without a function signature.
  TEST(IRBuilderContractDeathTest, RejectsVoidTypedReturnPayloadsInDetachedBlocks)
  {
    TypePool Types;
    ReturnInstruction VoidValue(Types.getVoidType());
    BasicBlock Detached;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Detached);
    EXPECT_DEATH(static_cast<void>(Builder.createReturn(VoidValue)), "Return value must have a non-void type");
  }
#endif
} // namespace ink::ir::test
