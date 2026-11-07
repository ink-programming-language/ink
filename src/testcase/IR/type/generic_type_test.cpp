#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/argument.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr IntegerType CompileTimeType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    constexpr FloatType CompileTimeFloat(FloatType::Width::Bits64);
    constexpr BoolType CompileTimeBool;
    constexpr VoidType CompileTimeVoid;
    constexpr ArrayType CompileTimeArray(CompileTimeType, 4);
    constexpr PointerType CompileTimePointer(CompileTimeType);
    constexpr IntegerConstant CompileTimeValue(CompileTimeType, 42);
    constexpr bool haveCanonicalTypeEdges() noexcept
    {
      // Debug-mode record vectors may allocate proxies, which must be released within this constant evaluation.
      const StructType Struct;
      const ObjectType Object;
      const Type *Types[] = {
          &MetaType::instance(),
          &CompileTimeType,
          &CompileTimeFloat,
          &CompileTimeBool,
          &CompileTimeVoid,
          &Struct,
          &Object,
          &CompileTimeArray,
          &CompileTimePointer,
      };
      for (const Type *TypeValue : Types)
      {
        const Value *Erased = TypeValue;
        if (Erased->valueKind() != ValueKind::Type || !Type::classof(Erased) || static_cast<const Type *>(Erased) != TypeValue || &TypeValue->getType() != &MetaType::instance() || &TypeValue->getType().getType() != &MetaType::instance() || &TypeValue->getType().getType().getType() != &MetaType::instance())
        {
          return false;
        }
      }
      return true;
    }

    static_assert(haveCanonicalTypeEdges());
    static_assert(&MetaType::instance().getType() == &MetaType::instance());
    static_assert(&MetaType::instance().getType().getType().getType() == &MetaType::instance());
    static_assert(&CompileTimeValue.getType() == &CompileTimeType);
    static_assert(&CompileTimeValue.getType().getType() == &MetaType::instance());
    static_assert(&CompileTimeValue.getType().getType().getType() == &MetaType::instance());
    static_assert(&CompileTimeArray.elementType() == &CompileTimeType && &CompileTimePointer.pointeeType() == &CompileTimeType);
    constexpr const Value *CompileTimeErasedType = &CompileTimeType;
    static_assert(CompileTimeErasedType->valueKind() == ValueKind::Type);
    static_assert(static_cast<const Type *>(CompileTimeErasedType) == &CompileTimeType);
    static_assert(!Type::classof(nullptr));
    static_assert(Type::classof(&CompileTimeType) && IntegerType::classof(&CompileTimeType));
  } // namespace

  static_assert(std::is_base_of_v<Value, Type> && std::is_base_of_v<Value, Argument>);
  static_assert(!std::is_base_of_v<Value, Parameter>);
  static_assert(std::is_base_of_v<Type, MetaType>);
  static_assert(std::is_same_v<decltype(std::declval<const Value &>().getType()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const Value &>().valueKind()), ValueKind>);
  static_assert(std::is_same_v<decltype(std::declval<const Parameter &>().type()), const Value &>);
  static_assert(std::is_copy_constructible_v<Parameter> && std::is_copy_assignable_v<Parameter>);
  static_assert(!std::is_convertible_v<Parameter &, ValueRef> && !std::is_convertible_v<const Parameter &, ValueRef>);
  static_assert(!std::is_convertible_v<Argument &&, ValueRef> && !std::is_convertible_v<const Argument &&, ValueRef>);
  static_assert(!std::is_copy_constructible_v<Argument> && !std::is_copy_assignable_v<Argument>);
  static_assert(!std::is_move_constructible_v<Argument> && !std::is_move_assignable_v<Argument>);

  // Every pool exposes the same metatype, and querying that singleton through Value returns itself as its type.
  TEST(IRGenericTypeTest, SharesOneSelfTypedMetatypeAcrossPools)
  {
    TypePool First;
    TypePool Second;
    const MetaType &Meta = MetaType::instance();
    const Value &MetaValue = Meta;
    EXPECT_EQ(&First.getMetaType(), &Meta);
    EXPECT_EQ(&Second.getMetaType(), &Meta);
    EXPECT_EQ(&Meta.getType(), &Meta);
    EXPECT_EQ(&MetaValue.getType(), &MetaValue);
    EXPECT_EQ(MetaValue.valueKind(), ValueKind::Type);
    EXPECT_EQ(static_cast<const Type *>(&MetaValue), &Meta);
    EXPECT_TRUE(MetaValue.isTypeValue());
    EXPECT_TRUE(Type::classof(&MetaValue));
    EXPECT_TRUE(MetaType::classof(&MetaValue));
  }

  // Scalar, record, array, pointer, and function types are values with the shared metatype and recover their concrete Type identity.
  TEST(IRGenericTypeTest, RepresentsEveryConcreteTypeAsAMetatypeValue)
  {
    TypePool Types;
    const IntegerType &Integer = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const StructType Struct;
    const ObjectType Object;
    const Type *TypeValues[] = {
        &Types.getMetaType(),
        &Integer,
        &Types.getFloatType(FloatType::Width::Bits32),
        &Types.getBoolType(),
        &Types.getVoidType(),
        &Struct,
        &Object,
        &Types.getArrayType(Integer, 4),
        &Types.getPointerType(Integer),
        &Types.getFunctionType(Integer),
    };
    for (const Type *TypeValue : TypeValues)
    {
      const Value &Value = *TypeValue;
      EXPECT_EQ(&Value.getType(), &Types.getMetaType());
      EXPECT_EQ(Value.valueKind(), ValueKind::Type);
      EXPECT_EQ(static_cast<const Type *>(&Value), TypeValue);
      EXPECT_TRUE(Value.isTypeValue());
      EXPECT_TRUE(Type::classof(&Value));
    }
  }

  // Arguments in one function or in separate functions keep distinct identities despite sharing a signature and name.
  TEST(IRGenericTypeTest, DistinguishesArgumentIdentitiesFromCopiedParameterDescriptions)
  {
    auto &Names = core::NamePool::instance();
    TypePool Types;
    const std::array Parameters{Parameter(Types.getMetaType()), Parameter(Types.getMetaType())};
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType(), Parameters);
    Function First(Signature);
    Function Second(Signature);
    Argument &T = First.argument(0);
    Argument &SameName = First.argument(1);
    Argument &OtherOwner = Second.argument(0);
    T.setName(Names.intern("T"));
    SameName.setName(T.name());
    OtherOwner.setName(T.name());
    const Parameter FromConstRef(std::cref(T));
    const Parameter FromRef(std::ref(SameName));
    const Parameter FromValueRef(ValueRef{T});
    const Parameter Named(std::cref(T), Parameter::Kind::Named);
    const Parameter Copied(Named);
    Parameter Assigned(Types.getMetaType());
    Assigned = Named;
    EXPECT_EQ(T.name(), SameName.name());
    EXPECT_EQ(T.name(), OtherOwner.name());
    EXPECT_EQ(T.parent(), &First);
    EXPECT_EQ(SameName.parent(), &First);
    EXPECT_EQ(OtherOwner.parent(), &Second);
    EXPECT_EQ(T.index(), 0U);
    EXPECT_EQ(SameName.index(), 1U);
    EXPECT_EQ(&T.getType(), &MetaType::instance());
    EXPECT_TRUE(T.isTypeValue());
    EXPECT_EQ(T.valueKind(), ValueKind::Argument);
    EXPECT_FALSE(Type::classof(&T));
    EXPECT_EQ(&FromConstRef.type(), &T);
    EXPECT_EQ(&FromRef.type(), &SameName);
    EXPECT_EQ(&FromValueRef.type(), &T);
    EXPECT_EQ(&Named.type(), &T);
    EXPECT_EQ(Named.kind(), Parameter::Kind::Named);
    EXPECT_EQ(&Copied.type(), &T);
    EXPECT_EQ(Copied.kind(), Parameter::Kind::Named);
    EXPECT_EQ(&Assigned.type(), &T);
    EXPECT_EQ(Assigned.kind(), Parameter::Kind::Named);
    EXPECT_EQ(FromConstRef.kind(), Parameter::Kind::Positional);
    const PointerType &Pointer = Types.getPointerType(T);
    EXPECT_NE(&Pointer, &Types.getPointerType(SameName));
    EXPECT_NE(&Pointer, &Types.getPointerType(OtherOwner));
    T.setName(Names.intern("RenamedT"));
    EXPECT_EQ(&Pointer, &Types.getPointerType(T));
    EXPECT_EQ(&Pointer.pointeeType(), &T);
  }

  // Copying signature descriptions retains x:T edges to the original argument instead of rebinding them to a new function's argument.
  TEST(IRGenericTypeTest, PreservesExternalArgumentIdentityWhenCopyingFunctionDescriptions)
  {
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    Argument &T = Owner.argument(0);
    T.setName(core::NamePool::instance().intern("T"));
    const Parameter Parameters[] = {
        Parameter(Types.getMetaType()),
        Parameter(std::cref(T)),
    };
    const FunctionType &Signature = Types.getFunctionType(T, Parameters);
    const std::array CopiedParameters{Parameters[0], Parameters[1]};
    Function Dependent(Signature);
    EXPECT_EQ(&Types.getFunctionType(T, CopiedParameters), &Signature);
    EXPECT_EQ(&Signature.returnType(), &T);
    ASSERT_EQ(Signature.parameters().size(), 2U);
    EXPECT_NE(&Signature.parameters()[0], &Parameters[0]);
    EXPECT_EQ(&Signature.parameters()[0].type(), &MetaType::instance());
    EXPECT_EQ(&Signature.parameters()[1].type(), &T);
    EXPECT_EQ(&Dependent.argument(0).getType(), &MetaType::instance());
    EXPECT_EQ(&Dependent.argument(1).getType(), &T);
    EXPECT_NE(&Dependent.argument(1).getType(), &Dependent.argument(0));
    EXPECT_EQ(&Dependent.signature().returnType(), &T);
  }

  // Composite types and record declarations preserve symbolic type operands, and functions accept an erased concrete function type.
  TEST(IRGenericTypeTest, KeepsSymbolicTypesInCompositeTypesAndRecordFields)
  {
    auto &Names = core::NamePool::instance();
    TypePool Types;
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType())}));
    Argument &T = Owner.argument(0);
    T.setName(Names.intern("T"));
    const PointerType &Pointer = Types.getPointerType(T);
    const ArrayType &Array = Types.getArrayType(T, 4);
    const ArrayType &NestedArray = Types.getArrayType(Pointer, 2);
    const FunctionType &Signature = Types.getFunctionType(T);
    const Value &ErasedSignature = Signature;
    const Function FunctionValue(ErasedSignature);
    StructType Record;
    Record.addBase(T);
    Record.addField(Names.intern("Value"), T);
    EXPECT_EQ(&Pointer.pointeeType(), &T);
    EXPECT_EQ(&Array.elementType(), &T);
    EXPECT_EQ(Array.elementCount(), 4U);
    EXPECT_EQ(&NestedArray.elementType(), &Pointer);
    EXPECT_EQ(&Signature.returnType(), &T);
    EXPECT_EQ(&FunctionValue.getType(), &Signature);
    EXPECT_EQ(&FunctionValue.signature(), &Signature);
    ASSERT_EQ(Record.bases().size(), 1U);
    EXPECT_EQ(Record.bases()[0], &T);
    ASSERT_EQ(Record.fields().size(), 1U);
    EXPECT_EQ(&Record.fields()[0].type(), &T);
    EXPECT_TRUE(Pointer.isTypeValue());
    EXPECT_TRUE(Array.isTypeValue());
    EXPECT_TRUE(Record.isTypeValue());
  }

  // ValueKind and every type classifier distinguish concrete types from arguments and constants, including MetaType arguments.
  TEST(IRGenericTypeTest, RejectsNonTypeObjectsThroughValuePointers)
  {
    TypePool Types;
    const IntegerType &Integer = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getMetaType()), Parameter(Integer)}));
    const Argument &T = Owner.argument(0);
    const Argument &Ordinary = Owner.argument(1);
    Function Dependent(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(T)}));
    const Argument &X = Dependent.argument(0);
    const IntegerConstant Constant(Integer, 42);
    const Value *NonTypes[] = {
        &T,
        &X,
        &Ordinary,
        &Constant,
        nullptr,
    };
    using Classifier = bool (*)(const Value *) noexcept;
    const Classifier Classifiers[] = {
        Type::classof,
        MetaType::classof,
        IntegerType::classof,
        FloatType::classof,
        BoolType::classof,
        VoidType::classof,
        ArrayType::classof,
        PointerType::classof,
        FunctionType::classof,
        RecordType::classof,
        StructType::classof,
        ObjectType::classof,
    };
    EXPECT_TRUE(T.isTypeValue());
    EXPECT_FALSE(X.isTypeValue());
    EXPECT_FALSE(Ordinary.isTypeValue());
    EXPECT_FALSE(Constant.isTypeValue());
    for (const Value *NonType : NonTypes)
    {
      if (NonType)
      {
        EXPECT_NE(NonType->valueKind(), ValueKind::Type);
      }
      for (Classifier Classof : Classifiers)
      {
        EXPECT_FALSE(Classof(NonType));
      }
    }
  }
} // namespace ink::ir::test
