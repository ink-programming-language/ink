#include "../../lib/ir/serialization/load_transaction.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/context.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // A failed load removes every newly interned type and constant while retaining preexisting cache identities.
  TEST(IRSerializationTransactionTest, RollsBackAllPoolCaches)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    auto &Constants = Context.constantPool();
    const auto *OldInteger = Types.get<IntegerType>(32, true);
    const auto *OldFloat = Types.get<FloatType>(32);
    const auto *OldPointer = Types.get<PointerType>(OldInteger);
    const auto *OldReference = Types.get<ReferenceType>(OldInteger);
    const auto *OldArray = Types.get<ArrayType>(OldInteger, 2);
    const auto *OldSignature = Types.get<FunctionType>(OldInteger, {OldInteger});
    const auto *OldNumber = Constants.get<IntegerConstant>(OldInteger, 1);
    const auto *OldReal = Constants.get<FloatConstant>(1.0f);
    const auto *OldElements = Constants.get<ArrayConstant>(OldArray, {OldNumber, OldNumber});
    const auto *OldText = Constants.get<StringConstant>("existing spelling");
    std::vector<const Type *> AddedTypes;
    std::vector<const Constant *> AddedConstants;
    {
      detail::IRLoadTransaction Transaction(Context);
      const auto *Integer = Types.get<IntegerType>(24, true, false);
      const auto *Float = Types.get<FloatType>(64, false);
      const auto *Pointer = Types.get<PointerType>(Integer, false);
      const auto *Reference = Types.get<ReferenceType>(Integer, false);
      const auto *Array = Types.get<ArrayType>(Integer, 2, false);
      const auto *Signature = Types.get<FunctionType>(Integer, {Pointer, Reference}, false);
      AddedTypes = {Integer, Float, Pointer, Reference, Array, Signature, Types.get(Integer, true), Types.get(Float, true), Types.get(Pointer, true), Types.get(Reference, true), Types.get(Array, true), Types.get(Signature, true)};
      const auto *Number = Constants.get<IntegerConstant>(Integer, 2);
      const auto *Real = Constants.get<FloatConstant>(2.0);
      const auto *Elements = Constants.get<ArrayConstant>(Array, {Number, Number});
      const auto *Text = Constants.get<StringConstant>("new spelling after load");
      ASSERT_NE(Text, nullptr);
      AddedTypes.push_back(Text->type());
      AddedConstants = {Number, Real, Elements, Text, Constants.get<IntegerConstant>(OldInteger, 2), Constants.get<FloatConstant>(2.0f), Constants.get<ArrayConstant>(OldArray, {OldNumber, Constants.get<IntegerConstant>(OldInteger, 2)})};
      for (const Type *TypeValue : AddedTypes)
      {
        ASSERT_NE(TypeValue, nullptr);
        EXPECT_TRUE(Types.owns(TypeValue));
      }
      for (const Constant *Value : AddedConstants)
      {
        ASSERT_NE(Value, nullptr);
        EXPECT_TRUE(Constants.owns(Value));
      }
    }
    for (const Type *TypeValue : AddedTypes)
    {
      EXPECT_FALSE(Types.owns(TypeValue));
    }
    for (const Constant *Value : AddedConstants)
    {
      EXPECT_FALSE(Constants.owns(Value));
    }
    EXPECT_EQ(Types.get<IntegerType>(32, true), OldInteger);
    EXPECT_EQ(Types.get<FloatType>(32), OldFloat);
    EXPECT_EQ(Types.get<PointerType>(OldInteger), OldPointer);
    EXPECT_EQ(Types.get<ReferenceType>(OldInteger), OldReference);
    EXPECT_EQ(Types.get<ArrayType>(OldInteger, 2), OldArray);
    EXPECT_EQ(Types.get<FunctionType>(OldInteger, {OldInteger}), OldSignature);
    EXPECT_EQ(Constants.get<IntegerConstant>(OldInteger, 1), OldNumber);
    EXPECT_EQ(Constants.get<FloatConstant>(1.0f), OldReal);
    EXPECT_EQ(Constants.get<ArrayConstant>(OldArray, {OldNumber, OldNumber}), OldElements);
    EXPECT_EQ(Constants.get<StringConstant>("existing spelling"), OldText);

    const auto *Integer = Types.get<IntegerType>(24, true, false);
    ASSERT_NE(Integer, nullptr);
    EXPECT_TRUE(Types.owns(Integer));
    EXPECT_TRUE(Types.owns(Types.get<FloatType>(64, false)));
    EXPECT_TRUE(Types.owns(Types.get<PointerType>(Integer, false)));
    EXPECT_TRUE(Types.owns(Types.get<ReferenceType>(Integer, false)));
    const auto *Array = Types.get<ArrayType>(Integer, 2, false);
    EXPECT_TRUE(Types.owns(Array));
    EXPECT_TRUE(Types.owns(Types.get<FunctionType>(Integer, {Types.get<PointerType>(Integer, false), Types.get<ReferenceType>(Integer, false)}, false)));
    const auto *Number = Constants.get<IntegerConstant>(Integer, 2);
    EXPECT_TRUE(Constants.owns(Number));
    EXPECT_TRUE(Constants.owns(Constants.get<FloatConstant>(2.0)));
    EXPECT_TRUE(Constants.owns(Constants.get<ArrayConstant>(Array, {Number, Number})));
    EXPECT_TRUE(Constants.owns(Constants.get<IntegerConstant>(OldInteger, 2)));
    EXPECT_TRUE(Constants.owns(Constants.get<FloatConstant>(2.0f)));
    const auto *Text = Constants.get<StringConstant>("new spelling after load");
    ASSERT_NE(Text, nullptr);
    EXPECT_TRUE(Constants.owns(Text));
    EXPECT_EQ(Text->value(), "new spelling after load");
  }

  // Rollback removes qualified nominal views and derived pools before safely destroying an unpublished module's bodies and declarations.
  TEST(IRSerializationTransactionTest, DestroysRejectedNominalModuleAfterRollback)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    Module Existing(Context);
    auto *ExistingClass = Existing.createClassType(8, 8);
    auto *ExistingStruct = Existing.createStructType(8, 8);
    ASSERT_NE(ExistingClass, nullptr);
    ASSERT_NE(ExistingStruct, nullptr);
    const auto *ExistingView = Types.get(ExistingClass, false);
    const auto *ExistingPointer = Types.get<PointerType>(ExistingView);
    std::unique_ptr<Module> Pending;
    const Type *Declaration = nullptr;
    std::vector<const Type *> AddedTypes;
    {
      detail::IRLoadTransaction Transaction(Context);
      Pending = std::make_unique<Module>(Context);
      auto *Class = Pending->createClassType(8, 8);
      auto *Struct = Pending->createStructType(8, 8);
      auto *Interface = Pending->createInterfaceType(8, 8);
      ASSERT_NE(Class, nullptr);
      ASSERT_NE(Struct, nullptr);
      ASSERT_NE(Interface, nullptr);
      Declaration = Class;
      const auto *View = Types.get(Class, false);
      const auto *Pointer = Types.get<PointerType>(View);
      const auto *Reference = Types.get<ReferenceType>(View);
      const auto *Array = Types.get<ArrayType>(View, 2);
      const auto *Signature = Types.get<FunctionType>(Types.get<VoidType>(), {Pointer});
      AddedTypes = {View, Types.get(Struct, false), Types.get(Interface, false), Types.get(ExistingStruct, false), Pointer, Reference, Array, Signature};
      for (const Type *TypeValue : AddedTypes)
      {
        ASSERT_NE(TypeValue, nullptr);
      }
      ASSERT_TRUE(Class->addField(Field(Pointer, core::NamePool::instance().intern("next"))));
      auto Callable = std::make_unique<Function>(*Signature, core::NamePool::instance().intern("method"));
      IRBuilder Builder(Context);
      ASSERT_TRUE(Builder.setInsertPoint(*Callable->createBlock()));
      ASSERT_NE(Builder.createReturn(), nullptr);
      ASSERT_TRUE(Class->addMethod(Method(std::move(Callable))));
      auto *FunctionValue = Pending->createGlobalFunction(*Signature);
      ASSERT_NE(FunctionValue, nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*FunctionValue->createBlock()));
      ASSERT_NE(Builder.createAlloca(*Array), nullptr);
      ASSERT_NE(Builder.createReturn(), nullptr);
      auto *Variable = Pending->createGlobalVariable(*Pointer);
      ASSERT_NE(Variable, nullptr);
      ASSERT_TRUE(Variable->setInitializer(Context.constantPool().get<NullPointerConstant>()));
      const auto *Text = Context.constantPool().get<StringConstant>("pending module text");
      ASSERT_NE(Text, nullptr);
      ASSERT_NE(Pending->getOrCreateStringStorage(*Text), nullptr);
    }
    for (const Type *TypeValue : AddedTypes)
    {
      EXPECT_FALSE(Types.owns(TypeValue));
    }
    Pending.reset();
    EXPECT_FALSE(Context.containsType(Declaration));
    EXPECT_TRUE(Context.containsType(ExistingClass));
    EXPECT_EQ(Types.get(ExistingClass, false), ExistingView);
    EXPECT_EQ(Types.get<PointerType>(ExistingView), ExistingPointer);
    EXPECT_TRUE(Types.owns(Types.get(ExistingStruct, false)));
    // Repeated failed loads exercise cache keys when allocators recycle nominal declaration addresses.
    for (unsigned Index = 0; Index < 8; ++Index)
    {
      std::unique_ptr<Module> Retry;
      detail::IRLoadTransaction Transaction(Context);
      Retry = std::make_unique<Module>(Context);
      auto *Class = Retry->createClassType(16, 8);
      ASSERT_NE(Class, nullptr);
      const auto *View = Types.get(Class, false);
      ASSERT_NE(View, nullptr);
      EXPECT_EQ(View->size(), 16U);
      const auto *Pointer = Types.get<PointerType>(View);
      ASSERT_NE(Pointer, nullptr);
      EXPECT_EQ(Pointer->pointeeType(), View);
    }
  }

  // Committing publishes the loaded module's canonical pool values without changing preexisting identities.
  TEST(IRSerializationTransactionTest, CommitPreservesCreatedPoolObjects)
  {
    IRContext Context(Target);
    std::unique_ptr<Module> Pending;
    const Type *View = nullptr;
    const PointerType *Pointer = nullptr;
    const StringConstant *Text = nullptr;
    {
      detail::IRLoadTransaction Transaction(Context);
      Pending = std::make_unique<Module>(Context);
      auto *Class = Pending->createClassType(8, 8);
      ASSERT_NE(Class, nullptr);
      View = Context.typePool().get(Class, false);
      Pointer = Context.typePool().get<PointerType>(View);
      Text = Context.constantPool().get<StringConstant>("committed spelling");
      ASSERT_NE(View, nullptr);
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(Text, nullptr);
      Transaction.commit();
    }
    EXPECT_TRUE(Context.containsType(View));
    EXPECT_TRUE(Context.containsType(Pointer));
    EXPECT_TRUE(Context.constantPool().owns(Text));
    EXPECT_EQ(Context.typePool().get(Pending->classType(0), false), View);
    EXPECT_EQ(Context.typePool().get<PointerType>(View), Pointer);
    EXPECT_EQ(Context.constantPool().get<StringConstant>("committed spelling"), Text);
  }
} // namespace ink::ir::test
