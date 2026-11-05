#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

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
    constexpr ReferenceType CompileTimeReference(CompileTimeType);
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
          &CompileTimeReference,
      };
      for (const Type *TypeValue : Types)
      {
        if (&TypeValue->getType() != &MetaType::instance() || &TypeValue->getType().getType() != &MetaType::instance() || &TypeValue->getType().getType().getType() != &MetaType::instance())
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
    static_assert(&CompileTimeArray.elementType() == &CompileTimeType && &CompileTimeReference.referencedType() == &CompileTimeType);
    static_assert(CompileTimeType.asType() == &CompileTimeType);
    static_assert(Type::classof(&CompileTimeType) && IntegerType::classof(&CompileTimeType));
  } // namespace

  static_assert(std::is_base_of_v<Value, Type> && std::is_base_of_v<Value, Parameter>);
  static_assert(std::is_base_of_v<Type, MetaType>);
  static_assert(std::is_same_v<decltype(std::declval<const Value &>().getType()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const Value &>().asType()), const Type *>);
  static_assert(std::is_same_v<decltype(std::declval<const Parameter &>().type()), const Value &>);
  static_assert(std::is_copy_constructible_v<Parameter> && std::is_copy_assignable_v<Parameter>);
  static_assert(!std::is_convertible_v<Parameter &&, ValueRef> && !std::is_convertible_v<const Parameter &&, ValueRef>);

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
    EXPECT_EQ(MetaValue.asType(), &Meta);
    EXPECT_TRUE(MetaValue.isTypeValue());
    EXPECT_TRUE(Type::classof(&MetaValue));
    EXPECT_TRUE(MetaType::classof(&MetaValue));
  }

  // Scalar, record, array, reference, and function types are values with the shared metatype and recover their concrete Type identity.
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
        &Types.getReferenceType(Integer),
        &Types.getFunctionType(Integer),
    };
    for (const Type *TypeValue : TypeValues)
    {
      const Value &Value = *TypeValue;
      EXPECT_EQ(&Value.getType(), &Types.getMetaType());
      EXPECT_EQ(Value.asType(), TypeValue);
      EXPECT_TRUE(Value.isTypeValue());
      EXPECT_TRUE(Type::classof(&Value));
    }
  }

  // Parameter copies preserve descriptions but create new identities, while all explicit reference-wrapper paths borrow the parameter as a type.
  TEST(IRGenericTypeTest, DistinguishesParameterCopiesFromExplicitTypeReferences)
  {
    auto &Names = core::NamePool::instance();
    const Parameter T(MetaType::instance(), Names.intern("T"));
    const Parameter SameName(MetaType::instance(), Names.intern("T"));
    Parameter MutableT(MetaType::instance(), Names.intern("T"));
    const Parameter Copied(T);
    Parameter Assigned(MetaType::instance());
    Assigned = T;
    const Parameter FromConstRef(std::cref(T));
    const Parameter FromRef(std::ref(MutableT));
    const Parameter FromValueRef(ValueRef{T});
    const Parameter Named(std::cref(T), Names.intern("x"), Parameter::Kind::Named);
    TypePool Types;
    EXPECT_EQ(T.name(), SameName.name());
    EXPECT_EQ(Copied.name(), T.name());
    EXPECT_EQ(Assigned.name(), T.name());
    EXPECT_EQ(&T.getType(), &MetaType::instance());
    EXPECT_EQ(&Copied.getType(), &MetaType::instance());
    EXPECT_EQ(&Assigned.getType(), &MetaType::instance());
    EXPECT_TRUE(T.isTypeValue());
    EXPECT_TRUE(Copied.isTypeValue());
    EXPECT_TRUE(Assigned.isTypeValue());
    EXPECT_EQ(T.asType(), nullptr);
    EXPECT_EQ(&FromConstRef.getType(), &T);
    EXPECT_EQ(&FromConstRef.type(), &T);
    EXPECT_EQ(&FromRef.getType(), &MutableT);
    EXPECT_EQ(&FromValueRef.getType(), &T);
    EXPECT_EQ(&Named.getType(), &T);
    EXPECT_EQ(Named.name(), Names.intern("x"));
    EXPECT_EQ(Named.kind(), Parameter::Kind::Named);
    EXPECT_EQ(FromConstRef.name(), core::NAME_None);
    EXPECT_FALSE(FromConstRef.isTypeValue());
    EXPECT_FALSE(FromRef.isTypeValue());
    EXPECT_FALSE(FromValueRef.isTypeValue());
    EXPECT_EQ(FromConstRef.asType(), nullptr);
    EXPECT_NE(&Types.getReferenceType(T), &Types.getReferenceType(SameName));
    EXPECT_NE(&Types.getReferenceType(T), &Types.getReferenceType(Copied));
    EXPECT_NE(&Types.getReferenceType(T), &Types.getReferenceType(Assigned));
  }

  // Copying a signature owns its parameter descriptions without rebinding an x:T type edge to a copied T description.
  TEST(IRGenericTypeTest, PreservesExternalTypeParameterIdentityWhenCopyingFunctionDescriptions)
  {
    auto &Names = core::NamePool::instance();
    const Parameter T(MetaType::instance(), Names.intern("T"));
    TypePool Types;
    const Parameter Parameters[] = {
        T,
        Parameter(std::cref(T), Names.intern("x")),
    };
    const FunctionType &Function = Types.getFunctionType(T, Parameters);
    EXPECT_EQ(&Function.returnType(), &T);
    ASSERT_EQ(Function.parameters().size(), 2U);
    EXPECT_NE(&Function.parameters()[0], &T);
    EXPECT_NE(&Function.parameters()[0], &Parameters[0]);
    EXPECT_EQ(&Function.parameters()[0].getType(), &MetaType::instance());
    EXPECT_EQ(Function.parameters()[0].name(), T.name());
    EXPECT_EQ(&Function.parameters()[1].type(), &T);
    EXPECT_EQ(&Function.parameters()[1].getType(), &T);
    EXPECT_NE(&Function.parameters()[1].getType(), &Function.parameters()[0]);
  }

  // Composite types and record declarations preserve symbolic type operands, and functions accept an erased concrete function type.
  TEST(IRGenericTypeTest, KeepsSymbolicTypesInCompositeTypesAndRecordFields)
  {
    auto &Names = core::NamePool::instance();
    const Parameter T(MetaType::instance(), Names.intern("T"));
    TypePool Types;
    const ReferenceType &Reference = Types.getReferenceType(T);
    const ArrayType &Array = Types.getArrayType(T, 4);
    const ArrayType &NestedArray = Types.getArrayType(Reference, 2);
    const FunctionType &Signature = Types.getFunctionType(T);
    const Value &ErasedSignature = Signature;
    const Function FunctionValue(ErasedSignature);
    StructType Record;
    Record.addBase(T);
    Record.addField(Names.intern("Value"), T);
    EXPECT_EQ(&Reference.referencedType(), &T);
    EXPECT_EQ(&Array.elementType(), &T);
    EXPECT_EQ(Array.elementCount(), 4U);
    EXPECT_EQ(&NestedArray.elementType(), &Reference);
    EXPECT_EQ(&Signature.returnType(), &T);
    EXPECT_EQ(&FunctionValue.getType(), &Signature);
    EXPECT_EQ(&FunctionValue.signature(), &Signature);
    ASSERT_EQ(Record.bases().size(), 1U);
    EXPECT_EQ(Record.bases()[0], &T);
    ASSERT_EQ(Record.fields().size(), 1U);
    EXPECT_EQ(&Record.fields()[0].type(), &T);
    EXPECT_TRUE(Reference.isTypeValue());
    EXPECT_TRUE(Array.isTypeValue());
    EXPECT_TRUE(Record.isTypeValue());
  }

  // asType and every type classifier safely reject parameters and constants even when a parameter itself has MetaType.
  TEST(IRGenericTypeTest, RejectsNonTypeObjectsThroughValuePointers)
  {
    TypePool Types;
    const IntegerType &Integer = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const Parameter T(Types.getMetaType());
    const Parameter X(std::cref(T));
    const Parameter Ordinary(Integer);
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
        ReferenceType::classof,
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
        EXPECT_EQ(NonType->asType(), nullptr);
      }
      for (Classifier Classof : Classifiers)
      {
        EXPECT_FALSE(Classof(NonType));
      }
    }
  }
} // namespace ink::ir::test
