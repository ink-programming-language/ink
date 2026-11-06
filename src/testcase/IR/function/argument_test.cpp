#include "ink/core/name/name_pool.h"
#include "ink/ir/function/argument.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, Argument>);
  static_assert(!std::is_default_constructible_v<Argument>);
  static_assert(!std::is_copy_constructible_v<Argument> && !std::is_copy_assignable_v<Argument>);
  static_assert(!std::is_move_constructible_v<Argument> && !std::is_move_assignable_v<Argument>);
  static_assert(std::is_same_v<decltype(std::declval<Argument &>().parent()), Function *>);
  static_assert(std::is_same_v<decltype(std::declval<const Argument &>().parent()), const Function *>);
  static_assert(std::is_same_v<decltype(std::declval<Function &>().argument(0)), Argument &>);
  static_assert(std::is_same_v<decltype(std::declval<const Function &>().argument(0)), const Argument &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<Function &>().arguments())>, Argument &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const Function &>().arguments())>, const Argument &>);

  // Functions sharing an interned signature own separate argument identities with the signature's ordered types and kinds.
  TEST(IRArgumentTest, OwnsDistinctArgumentsForFunctionsWithTheSameSignature)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{
        Parameter(Int32),
        Parameter(Types.getBoolType(), Parameter::Kind::Named),
        Parameter(Int32, Parameter::Kind::Variadic),
    };
    const FunctionType &Signature = Types.getFunctionType(Int32, Parameters);
    Function First(Signature);
    Function Second(Signature);
    ASSERT_EQ(First.argumentCount(), Parameters.size());
    ASSERT_EQ(Second.argumentCount(), Parameters.size());
    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
    {
      Argument &FirstArgument = First.argument(Index);
      Argument &SecondArgument = Second.argument(Index);
      EXPECT_NE(&FirstArgument, &SecondArgument);
      EXPECT_EQ(FirstArgument.parent(), &First);
      EXPECT_EQ(SecondArgument.parent(), &Second);
      EXPECT_EQ(FirstArgument.index(), Index);
      EXPECT_EQ(SecondArgument.index(), Index);
      EXPECT_EQ(&FirstArgument.getType(), &Parameters[Index].type());
      EXPECT_EQ(&SecondArgument.getType(), &Parameters[Index].type());
      EXPECT_EQ(FirstArgument.kind(), Parameters[Index].kind());
      EXPECT_EQ(SecondArgument.kind(), Parameters[Index].kind());
      EXPECT_EQ(FirstArgument.name(), core::NAME_None);
    }
  }

  // Renaming one function's arguments leaves another function and the canonical signature's hash and lookup unchanged.
  TEST(IRArgumentTest, KeepsArgumentNamesIndependentFromSharedSignatureIdentity)
  {
    TypePool Types;
    const std::array Parameters{Parameter(Types.getBoolType(), Parameter::Kind::Named)};
    const FunctionType &Signature = Types.getFunctionType(Types.getBoolType(), Parameters);
    const std::size_t OriginalHash = Signature.hash();
    Function First(Signature);
    Function Second(Signature);
    const core::Name FlagName = core::NamePool::instance().intern("flag");
    First.argument(0).setName(FlagName);
    EXPECT_EQ(First.argument(0).name(), FlagName);
    EXPECT_EQ(Second.argument(0).name(), core::NAME_None);
    EXPECT_EQ(Signature.hash(), OriginalHash);
    EXPECT_EQ(&Types.getFunctionType(Types.getBoolType(), Parameters), &Signature);
    EXPECT_EQ(&First.signature(), &Second.signature());
    First.argument(0).setName(core::NAME_None);
    EXPECT_EQ(First.argument(0).name(), core::NAME_None);
    EXPECT_EQ(Signature.hash(), OriginalHash);
  }

  // Function-owned arguments are borrowed directly by arithmetic, memory stores, and returns without copying their identity.
  TEST(IRArgumentTest, UsesArgumentsAsArithmeticMemoryAndReturnOperands)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Int32), Parameter(Int32)};
    Function FunctionValue(Types.getFunctionType(Int32, Parameters));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    AddInstruction *Sum = nullptr;
    StoreInstruction *Store = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Entry);
      AllocaInstruction *Slot = &Builder.createAlloca(Int32);
      Sum = &Builder.createAdd(FunctionValue.argument(0), FunctionValue.argument(1));
      Store = &Builder.createStore(*Slot, FunctionValue.argument(0));
      Return = &Builder.createReturn(FunctionValue.argument(1));
    }
    EXPECT_EQ(&Sum->getLHS(), &FunctionValue.argument(0));
    EXPECT_EQ(&Sum->getRHS(), &FunctionValue.argument(1));
    EXPECT_EQ(&Sum->getType(), &Int32);
    EXPECT_EQ(&Store->getSource(), &FunctionValue.argument(0));
    EXPECT_EQ(Return->getReturnValue(), &FunctionValue.argument(1));
    EXPECT_EQ(Sum->parent(), &Entry);
    EXPECT_EQ(Store->parent(), &Entry);
    EXPECT_EQ(Return->parent(), &Entry);
  }

  // MetaType arguments serve as distinct symbolic type values for pointer, array, and allocation type operands.
  TEST(IRArgumentTest, PreservesMetaTypeArgumentIdentityInCompositeTypesAndAllocation)
  {
    TypePool Types;
    const std::array Parameters{Parameter(Types.getMetaType())};
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType(), Parameters);
    Function First(Signature);
    Function Second(Signature);
    Argument &T = First.argument(0);
    Argument &OtherT = Second.argument(0);
    EXPECT_TRUE(T.isTypeValue());
    EXPECT_EQ(T.asType(), nullptr);
    const PointerType &Pointer = Types.getPointerType(T);
    const ArrayType &Array = Types.getArrayType(T, 4);
    EXPECT_EQ(&Pointer.pointeeType(), &T);
    EXPECT_EQ(&Array.elementType(), &T);
    EXPECT_NE(&Pointer, &Types.getPointerType(OtherT));
    EXPECT_NE(&Array, &Types.getArrayType(OtherT, 4));
    BasicBlock &Entry = First.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    AllocaInstruction *Slot = &Builder.createAlloca(T);
    EXPECT_EQ(&Slot->getAllocatedType(), &T);
    EXPECT_EQ(&Slot->getType(), &Pointer);
  }

  // Const traversal and indexing agree with mutable argument identities after growth of the function's block collection.
  TEST(IRArgumentTest, PreservesArgumentAddressesAndIteratorsWhileBlocksGrow)
  {
    TypePool Types;
    const std::array Parameters{Parameter(Types.getBoolType()), Parameter(Types.getMetaType())};
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), Parameters));
    auto SavedRange = FunctionValue.arguments();
    auto SavedIterator = SavedRange.begin();
    Argument *First = &FunctionValue.argument(0);
    Argument *Second = &FunctionValue.argument(1);
    for (std::size_t Index = 0; Index < 128; ++Index)
    {
      FunctionValue.createBasicBlock();
    }
    EXPECT_EQ(&*SavedIterator, First);
    EXPECT_EQ(&FunctionValue.argument(0), First);
    EXPECT_EQ(&FunctionValue.argument(1), Second);
    ++SavedIterator;
    EXPECT_EQ(&*SavedIterator, Second);
    ++SavedIterator;
    EXPECT_EQ(SavedIterator, SavedRange.end());
    const Function &ConstFunction = FunctionValue;
    std::size_t Index = 0;
    for (const Argument &ArgumentValue : ConstFunction.arguments())
    {
      EXPECT_EQ(&ArgumentValue, &ConstFunction.argument(Index));
      EXPECT_EQ(ArgumentValue.parent(), &ConstFunction);
      EXPECT_EQ(ArgumentValue.index(), Index);
      ++Index;
    }
    EXPECT_EQ(Index, ConstFunction.argumentCount());
  }

  // A parameterless signature creates an empty argument range for both mutable and const function access.
  TEST(IRArgumentTest, ExposesEmptyArgumentsForParameterlessFunctions)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType()));
    const Function &ConstFunction = FunctionValue;
    EXPECT_EQ(FunctionValue.argumentCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(FunctionValue.arguments()));
    EXPECT_TRUE(std::ranges::empty(ConstFunction.arguments()));
  }
} // namespace ink::ir::test
