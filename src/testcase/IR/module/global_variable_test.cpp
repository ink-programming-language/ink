#include "ink/core/support/core_define.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/argument.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/module/global_variable.h"
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
  static_assert(std::is_base_of_v<Value, GlobalVariable>);
  static_assert(!std::is_copy_constructible_v<GlobalVariable> && !std::is_copy_assignable_v<GlobalVariable>);
  static_assert(!std::is_move_constructible_v<GlobalVariable> && !std::is_move_assignable_v<GlobalVariable>);
  static_assert(std::is_nothrow_constructible_v<GlobalVariable, const PointerType &>);
  static_assert(!std::is_constructible_v<GlobalVariable, PointerType &&> && !std::is_constructible_v<GlobalVariable, const PointerType &&>);
  static_assert(!std::is_constructible_v<GlobalVariable, PointerType &&, core::Name, SymbolVisibility, NativeLinkage>);
  static_assert(!std::is_constructible_v<GlobalVariable, const PointerType &&, core::Name, SymbolVisibility, NativeLinkage>);
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
    const IntegerConstant Value(Int32, 42);
#if INK_ENABLE_ASSERTIONS
    const IntegerType SeparateInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant WrongTypeValue(SeparateInt32, 42);
#endif
    const core::Name Name = core::NamePool::instance().intern("counter");
    GlobalVariable Global(Types.getPointerType(Int32), Name);
    Function FunctionValue(Types.getFunctionType(Int32));
    BasicBlock &Block = FunctionValue.createBasicBlock();
    StoreInstruction *Store = nullptr;
    LoadInstruction *Load = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Block);
#if INK_ENABLE_ASSERTIONS
      EXPECT_DEATH(static_cast<void>(Builder.createStore(Global, WrongTypeValue)), "Store source type must match the destination pointee type");
#endif
      EXPECT_EQ(Block.instructionCount(), 0U);
      Store = &Builder.createStore(Global, Value);
      Load = &Builder.createLoad(std::cref(Global));
      Return = &Builder.createReturn(*Load);
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
    EXPECT_EQ(&Global.getType(), &Types.getPointerType(Int32));
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

  // Pointer-valued globals store another global's address, and each load removes exactly one pointer layer.
  TEST(IRGlobalVariableTest, LoadsNestedPointerGlobalsOneLayerAtATime)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const PointerType &IntegerPointer = Types.getPointerType(Int32);
    const PointerType &AddressPointer = Types.getPointerType(IntegerPointer);
    const GlobalVariable IntegerGlobal(IntegerPointer);
    const GlobalVariable AddressGlobal(AddressPointer);
    const IntegerConstant Value(Int32, 42);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    static_cast<void>(Builder.createStore(IntegerGlobal, Value));
    StoreInstruction *AddressStore = &Builder.createStore(AddressGlobal, IntegerGlobal);
    LoadInstruction *LoadedAddress = &Builder.createLoad(AddressGlobal);
    LoadInstruction *LoadedInteger = &Builder.createLoad(*LoadedAddress);
    EXPECT_EQ(&AddressGlobal.valueType(), &IntegerPointer);
    EXPECT_EQ(&AddressGlobal.getType(), &AddressPointer);
    EXPECT_EQ(&AddressStore->getDestination(), &AddressGlobal);
    EXPECT_EQ(&AddressStore->getSource(), &IntegerGlobal);
    EXPECT_EQ(&LoadedAddress->getType(), &IntegerPointer);
    EXPECT_EQ(&LoadedInteger->getType(), &Int32);
    EXPECT_EQ(&LoadedInteger->getSource(), LoadedAddress);
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createStore(AddressGlobal, Value)), "Store source type must match the destination pointee type");
#endif
    EXPECT_EQ(Block.instructionCount(), 4U);
  }

  // Generic globals borrow the exact symbolic type argument rather than merging arguments that share a name.
  TEST(IRGlobalVariableTest, PreservesSymbolicStorageAndRejectsValuesOfAnotherTypeArgument)
  {
    const core::Name Name = core::NamePool::instance().intern("T");
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType()), Parameter(Types.getMetaType())}));
    Argument &T = Owner.argument(0);
    Argument &OtherT = Owner.argument(1);
    T.setName(Name);
    OtherT.setName(Name);
    Function Values(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(T), Parameter(OtherT)}));
    const Argument &ValueOfT = Values.argument(0);
    const Argument &ValueOfOtherT = Values.argument(1);
    EXPECT_EQ(T.name(), OtherT.name());
    EXPECT_NE(&ValueOfT.getType(), &ValueOfOtherT.getType());
    const GlobalVariable Global(Types.getPointerType(T));
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    StoreInstruction *Store = &Builder.createStore(Global, ValueOfT);
    LoadInstruction *Load = &Builder.createLoad(Global);
    EXPECT_EQ(&Global.valueType(), &T);
    EXPECT_EQ(&Load->getType(), &T);
    EXPECT_EQ(&Store->getSource(), &ValueOfT);
#if INK_ENABLE_ASSERTIONS
    EXPECT_DEATH(static_cast<void>(Builder.createStore(Global, ValueOfOtherT)), "Store source type must match the destination pointee type");
#endif
    static_cast<void>(Builder.createStore(Global, *Load));
    EXPECT_EQ(Block.instructionCount(), 3U);
  }
} // namespace ink::ir::test
