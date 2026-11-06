#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/parameter.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <functional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, GlobalVariable>);
  static_assert(!std::is_copy_constructible_v<GlobalVariable> && !std::is_copy_assignable_v<GlobalVariable>);
  static_assert(!std::is_move_constructible_v<GlobalVariable> && !std::is_move_assignable_v<GlobalVariable>);
  static_assert(std::is_nothrow_constructible_v<GlobalVariable, const ReferenceType &>);
  static_assert(!std::is_constructible_v<GlobalVariable, ReferenceType &&> && !std::is_constructible_v<GlobalVariable, const ReferenceType &&>);
  static_assert(!std::is_constructible_v<GlobalVariable, ReferenceType &&, core::Name, SymbolVisibility, NativeLinkage>);
  static_assert(!std::is_constructible_v<GlobalVariable, const ReferenceType &&, core::Name, SymbolVisibility, NativeLinkage>);
  static_assert(!std::is_constructible_v<GlobalVariable, const IntegerType &>);
  static_assert(std::is_same_v<decltype(std::declval<const GlobalVariable &>().valueType()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<GlobalVariable &>().parent()), Module *>);
  static_assert(std::is_same_v<decltype(std::declval<const GlobalVariable &>().parent()), const Module *>);

  template <typename GlobalType>
  concept BuilderAcceptsGlobalLoad = requires(InstructionBuilder &Builder, GlobalType &&Global)
  {
    Builder.createLoad(std::forward<GlobalType>(Global));
  };

  template <typename GlobalType>
  concept BuilderAcceptsGlobalStore = requires(InstructionBuilder &Builder, GlobalType &&Global, ValueRef Source)
  {
    Builder.createStore(std::forward<GlobalType>(Global), Source);
  };

  static_assert(BuilderAcceptsGlobalLoad<GlobalVariable &> && BuilderAcceptsGlobalLoad<const GlobalVariable &>);
  static_assert(!BuilderAcceptsGlobalLoad<GlobalVariable> && !BuilderAcceptsGlobalLoad<const GlobalVariable>);
  static_assert(BuilderAcceptsGlobalStore<GlobalVariable &> && BuilderAcceptsGlobalStore<const GlobalVariable &>);
  static_assert(!BuilderAcceptsGlobalStore<GlobalVariable> && !BuilderAcceptsGlobalStore<const GlobalVariable>);

  // Detached globals provide stable address operands after their builder dies and reject stores from a distinct type identity.
  TEST(IRGlobalVariableTest, BuildsTypedMemoryOperationsAndKeepsOperandsAcrossSymbolChanges)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType SeparateInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Value(Int32, 42);
    const IntegerConstant WrongTypeValue(SeparateInt32, 42);
    const core::Name Name = core::NamePool::instance().intern("counter");
    GlobalVariable Global(Types.getReferenceType(Int32), Name);
    Function FunctionValue(Types.getFunctionType(Int32));
    BasicBlock &Block = FunctionValue.createBasicBlock();
    StoreInstruction *Store = nullptr;
    LoadInstruction *Load = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Block);
      EXPECT_EQ(Builder.createStore(Global, WrongTypeValue), nullptr);
      EXPECT_EQ(Block.instructionCount(), 0U);
      Store = Builder.createStore(Global, Value);
      Load = Builder.createLoad(std::cref(Global));
      ASSERT_NE(Store, nullptr);
      ASSERT_NE(Load, nullptr);
      Return = Builder.createReturn(*Load);
      ASSERT_NE(Return, nullptr);
    }
    EXPECT_EQ(Global.parent(), nullptr);
    EXPECT_EQ(FunctionValue.parent(), nullptr);
    EXPECT_EQ(Global.visibility(), SymbolVisibility::Private);
    EXPECT_EQ(Global.nativeLinkage(), NativeLinkage::None);
    Global.setVisibility(SymbolVisibility::Public);
    Global.setNativeLinkage(NativeLinkage::Export);
    EXPECT_EQ(Global.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(Global.nativeLinkage(), NativeLinkage::Export);
    EXPECT_EQ(Global.name(), Name);
    EXPECT_EQ(&Global.getType(), &Types.getReferenceType(Int32));
    EXPECT_EQ(&Global.valueType(), &Int32);
    EXPECT_EQ(&Store->getDestination(), &Global);
    EXPECT_EQ(&Store->getSource(), &Value);
    EXPECT_EQ(&Load->getSource(), &Global);
    EXPECT_EQ(&Load->getType(), &Int32);
    EXPECT_EQ(Return->getReturnValue(), Load);
    ASSERT_EQ(Block.instructionCount(), 3U);
    auto Position = Block.instructions().begin();
    EXPECT_EQ(&*Position++, Store);
    EXPECT_EQ(&*Position++, Load);
    EXPECT_EQ(&*Position++, Return);
    EXPECT_EQ(Position, Block.instructions().end());
  }

  // Reference-valued globals store another global's address, and each load removes exactly one reference layer.
  TEST(IRGlobalVariableTest, LoadsNestedReferenceGlobalsOneLayerAtATime)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ReferenceType &IntegerReference = Types.getReferenceType(Int32);
    const ReferenceType &AddressReference = Types.getReferenceType(IntegerReference);
    const GlobalVariable IntegerGlobal(IntegerReference);
    const GlobalVariable AddressGlobal(AddressReference);
    const IntegerConstant Value(Int32, 42);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    ASSERT_NE(Builder.createStore(IntegerGlobal, Value), nullptr);
    StoreInstruction *AddressStore = Builder.createStore(AddressGlobal, IntegerGlobal);
    ASSERT_NE(AddressStore, nullptr);
    LoadInstruction *LoadedAddress = Builder.createLoad(AddressGlobal);
    ASSERT_NE(LoadedAddress, nullptr);
    LoadInstruction *LoadedInteger = Builder.createLoad(*LoadedAddress);
    ASSERT_NE(LoadedInteger, nullptr);
    EXPECT_EQ(&AddressGlobal.valueType(), &IntegerReference);
    EXPECT_EQ(&AddressGlobal.getType(), &AddressReference);
    EXPECT_EQ(&AddressStore->getDestination(), &AddressGlobal);
    EXPECT_EQ(&AddressStore->getSource(), &IntegerGlobal);
    EXPECT_EQ(&LoadedAddress->getType(), &IntegerReference);
    EXPECT_EQ(&LoadedInteger->getType(), &Int32);
    EXPECT_EQ(&LoadedInteger->getSource(), LoadedAddress);
    EXPECT_EQ(Builder.createStore(AddressGlobal, Value), nullptr);
    EXPECT_EQ(Block.instructionCount(), 4U);
  }

  // Generic globals borrow the exact symbolic type parameter rather than merging parameters that share a name.
  TEST(IRGlobalVariableTest, PreservesSymbolicStorageAndRejectsValuesOfAnotherTypeParameter)
  {
    const core::Name Name = core::NamePool::instance().intern("T");
    const Parameter T(MetaType::instance(), Name);
    const Parameter OtherT(MetaType::instance(), Name);
    const Parameter ValueOfT(std::cref(T));
    const Parameter ValueOfOtherT(std::cref(OtherT));
    TypePool Types;
    const GlobalVariable Global(Types.getReferenceType(T));
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    StoreInstruction *Store = Builder.createStore(Global, ValueOfT);
    LoadInstruction *Load = Builder.createLoad(Global);
    ASSERT_NE(Store, nullptr);
    ASSERT_NE(Load, nullptr);
    EXPECT_EQ(&Global.valueType(), &T);
    EXPECT_EQ(&Load->getType(), &T);
    EXPECT_EQ(&Store->getSource(), &ValueOfT);
    EXPECT_EQ(Builder.createStore(Global, ValueOfOtherT), nullptr);
    ASSERT_NE(Builder.createStore(Global, *Load), nullptr);
    EXPECT_EQ(Block.instructionCount(), 3U);
  }
} // namespace ink::ir::test
