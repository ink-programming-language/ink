#include "ink/semantic/model/context.h"

#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/interface_type.h"

#include <gtest/gtest.h>

#include <limits>
#include <type_traits>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Type, BuiltinType>);
  static_assert(std::is_base_of_v<Type, UserDefinedType>);
  static_assert(std::is_base_of_v<BuiltinType, IntegerType>);
  static_assert(std::is_base_of_v<BuiltinType, FloatType>);
  static_assert(std::is_base_of_v<BuiltinType, ArrayType>);
  static_assert(std::is_base_of_v<BuiltinType, SliceType>);
  static_assert(std::is_base_of_v<BuiltinType, PointerType>);
  static_assert(std::is_base_of_v<BuiltinType, ReferenceType>);
  static_assert(std::is_base_of_v<BuiltinType, FunctionType>);
  static_assert(std::is_base_of_v<UserDefinedType, ClassType>);
  static_assert(std::is_base_of_v<UserDefinedType, EnumType>);
  static_assert(std::is_base_of_v<UserDefinedType, InterfaceType>);

  // All builtin categories use the metatype and safely cast through Value and Type.
  TEST(SemanticTypeTest, BuiltinClassificationMatchesTheInheritanceHierarchy)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    ASSERT_NE(Int32, nullptr);
    const Type *Types[] = {
        &Context.getMetaType(),
        &Context.getVoidType(),
        &Context.getBoolType(),
        &Context.getLabelType(),
        &Context.getModuleType(),
        Int32,
        Context.getFloatType(32),
        Context.getArrayType(*Int32, 4),
        Context.getSliceType(*Int32, AccessKind::ReadOnly),
        Context.getPointerType(*Int32, AccessKind::ReadWrite),
        Context.getReferenceType(*Int32, AccessKind::ReadOnly),
        Context.getFunctionType(*Int32),
    };
    for (const Type *Item : Types)
    {
      ASSERT_NE(Item, nullptr);
      const Value *ValueObject = Item;
      EXPECT_TRUE(BuiltinType::classof(ValueObject));
      EXPECT_FALSE(UserDefinedType::classof(Item));
      EXPECT_EQ(&Item->type(), &Context.getMetaType());
    }
    EXPECT_FALSE(BuiltinType::classof(nullptr));
    EXPECT_FALSE(UserDefinedType::classof(nullptr));
    EXPECT_FALSE(BuiltinType::classof(&Context.getBoolConstant(true)));
    EXPECT_FALSE(UserDefinedType::classof(&Context.getBoolConstant(true)));
  }

  // Floating formats are exact canonical IEEE widths and reject unsupported formats.
  TEST(SemanticTypeTest, FloatTypesAreCanonicalBySupportedWidth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const std::uint32_t Widths[] = {
        16,
        32,
        64,
    };
    for (std::uint32_t Width : Widths)
    {
      const FloatType *Float = Context.getFloatType(Width);
      ASSERT_NE(Float, nullptr);
      EXPECT_EQ(Float->bitWidth(), Width);
      EXPECT_EQ(Context.getFloatType(Width), Float);
      EXPECT_NE(Other.getFloatType(Width), Float);
      EXPECT_TRUE(FloatType::classof(Float));
      EXPECT_FALSE(IntegerType::classof(Float));
      EXPECT_EQ(&Float->type(), &Context.getMetaType());
    }
    EXPECT_NE(Context.getFloatType(32), Context.getFloatType(64));
    EXPECT_EQ(Context.getFloatType(0), nullptr);
    EXPECT_EQ(Context.getFloatType(8), nullptr);
    EXPECT_EQ(Context.getFloatType(80), nullptr);
    EXPECT_EQ(Context.getFloatType(128), nullptr);
    EXPECT_FALSE(FloatType::classof(Context.getIntegerType(32, true)));
  }

  // Array identity preserves exact element type, length, nested dimensions and 64-bit counts.
  TEST(SemanticTypeTest, ArraysAreCanonicalByElementAndFullLength)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const ArrayType *Row = Context.getArrayType(*Int32, 4);
    ASSERT_NE(Row, nullptr);
    EXPECT_EQ(Row, Context.getArrayType(*Int32, 4));
    EXPECT_NE(Row, Context.getArrayType(*Int32, 5));
    EXPECT_NE(Row, Context.getArrayType(*Context.getIntegerType(32, false), 4));
    EXPECT_EQ(&Row->elementType(), Int32);
    EXPECT_EQ(Row->elementCount(), 4U);
    const ArrayType *Matrix = Context.getArrayType(*Row, 3);
    ASSERT_NE(Matrix, nullptr);
    EXPECT_EQ(&Matrix->elementType(), Row);
    EXPECT_EQ(Matrix, Context.getArrayType(*Context.getArrayType(*Int32, 4), 3));
    EXPECT_NE(Matrix, Context.getArrayType(*Int32, 12));
    const ArrayType *Empty = Context.getArrayType(*Int32, 0);
    ASSERT_NE(Empty, nullptr);
    EXPECT_EQ(Empty->elementCount(), 0U);
    EXPECT_NE(Empty, Row);
    const auto MaximumCount = std::numeric_limits<std::uint64_t>::max();
    const ArrayType *Large = Context.getArrayType(*Int32, MaximumCount);
    ASSERT_NE(Large, nullptr);
    EXPECT_EQ(Large->elementCount(), MaximumCount);
    EXPECT_EQ(Large, Context.getArrayType(*Int32, MaximumCount));
    EXPECT_NE(Large, Context.getArrayType(*Int32, std::numeric_limits<std::uint32_t>::max()));
    EXPECT_TRUE(ArrayType::classof(Row));
    EXPECT_FALSE(SliceType::classof(Row));
  }

  // Slice identity includes element access and stays distinct from fixed-size arrays.
  TEST(SemanticTypeTest, SlicesPreserveElementTypeAndAccess)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const SliceType *ReadOnly = Context.getSliceType(*Int32, AccessKind::ReadOnly);
    const SliceType *ReadWrite = Context.getSliceType(*Int32, AccessKind::ReadWrite);
    ASSERT_NE(ReadOnly, nullptr);
    ASSERT_NE(ReadWrite, nullptr);
    EXPECT_EQ(ReadOnly, Context.getSliceType(*Int32, AccessKind::ReadOnly));
    EXPECT_EQ(ReadWrite, Context.getSliceType(*Int32, AccessKind::ReadWrite));
    EXPECT_NE(ReadOnly, ReadWrite);
    EXPECT_EQ(&ReadOnly->elementType(), Int32);
    EXPECT_EQ(ReadOnly->access(), AccessKind::ReadOnly);
    EXPECT_EQ(ReadWrite->access(), AccessKind::ReadWrite);
    EXPECT_NE(ReadOnly, Context.getSliceType(Context.getBoolType(), AccessKind::ReadOnly));
    EXPECT_TRUE(SliceType::classof(ReadOnly));
    EXPECT_FALSE(ArrayType::classof(ReadOnly));
    EXPECT_NE(static_cast<const Type *>(ReadOnly), Context.getArrayType(*Int32, 0));
  }

  // Pointer and reference identities include target/access without sharing the two type kinds.
  TEST(SemanticTypeTest, PointersAndReferencesPreserveTargetAndAccess)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const PointerType *Pointer = Context.getPointerType(*Int32, AccessKind::ReadOnly);
    const ReferenceType *Reference = Context.getReferenceType(*Int32, AccessKind::ReadOnly);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    EXPECT_EQ(Pointer, Context.getPointerType(*Int32, AccessKind::ReadOnly));
    EXPECT_EQ(Reference, Context.getReferenceType(*Int32, AccessKind::ReadOnly));
    EXPECT_NE(Pointer, Context.getPointerType(*Int32, AccessKind::ReadWrite));
    EXPECT_NE(Reference, Context.getReferenceType(*Int32, AccessKind::ReadWrite));
    EXPECT_NE(Pointer, Context.getPointerType(Context.getBoolType(), AccessKind::ReadOnly));
    EXPECT_NE(Reference, Context.getReferenceType(Context.getBoolType(), AccessKind::ReadOnly));
    EXPECT_EQ(Pointer->access(), AccessKind::ReadOnly);
    EXPECT_EQ(Reference->access(), AccessKind::ReadOnly);
    EXPECT_EQ(&Pointer->pointeeType(), Int32);
    EXPECT_EQ(&Reference->referentType(), Int32);
    EXPECT_TRUE(PointerType::classof(Pointer));
    EXPECT_FALSE(ReferenceType::classof(Pointer));
    EXPECT_TRUE(ReferenceType::classof(Reference));
    EXPECT_FALSE(PointerType::classof(Reference));
    EXPECT_NE(static_cast<const Type *>(Pointer), Reference);
    const PointerType *Nested = Context.getPointerType(*Pointer, AccessKind::ReadWrite);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(&Nested->pointeeType(), Pointer);
    Variable *Binding = Context.createVariable(Context.namePool().intern("P"), BindingMutability::Immutable);
    ASSERT_NE(Binding, nullptr);
    ASSERT_TRUE(Binding->setType(*Context.getPointerType(*Int32, AccessKind::ReadWrite)));
    EXPECT_FALSE(Binding->isMutable());
    EXPECT_EQ(Binding->mutability(), BindingMutability::Immutable);
    ASSERT_TRUE(PointerType::classof(Binding->type()));
    EXPECT_EQ(static_cast<const PointerType *>(Binding->type())->access(), AccessKind::ReadWrite);
  }

  // Composite types reject foreign-context components and unknown access flags explicitly.
  TEST(SemanticTypeTest, CompositeFactoriesRejectForeignTypesAndInvalidAccess)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const Type &Foreign = Other.getBoolType();
    EXPECT_EQ(Context.getArrayType(Foreign, 1), nullptr);
    EXPECT_EQ(Context.getSliceType(Foreign, AccessKind::ReadOnly), nullptr);
    EXPECT_EQ(Context.getPointerType(Foreign, AccessKind::ReadWrite), nullptr);
    EXPECT_EQ(Context.getReferenceType(Foreign, AccessKind::ReadOnly), nullptr);
    const auto InvalidAccess = static_cast<AccessKind>(255);
    EXPECT_EQ(Context.getSliceType(Context.getBoolType(), InvalidAccess), nullptr);
    EXPECT_EQ(Context.getPointerType(Context.getBoolType(), InvalidAccess), nullptr);
    EXPECT_EQ(Context.getReferenceType(Context.getBoolType(), InvalidAccess), nullptr);
  }

  // Resolved nominal types have distinct identities and specific kinds without allocating generic declarations.
  TEST(SemanticTypeTest, UserDefinedTypesHaveIndependentIdentityAndSpecificKinds)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Name NameValue = Context.namePool().intern("Point");
    const ClassType *Class = Context.createClassType(NameValue);
    const ClassType *Second = Context.createClassType(NameValue);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(Class, Second);
    EXPECT_EQ(Class->name(), Second->name());
    EXPECT_EQ(&Class->context(), &Context);
    EXPECT_EQ(Context.namePool().size(), 1U);
    const EnumType *Enum = Context.createEnumType(NameValue);
    const InterfaceType *Interface = Context.createInterfaceType(NameValue);
    ASSERT_NE(Enum, nullptr);
    ASSERT_NE(Interface, nullptr);
    EXPECT_TRUE(ClassType::classof(Class));
    EXPECT_FALSE(ClassType::classof(Enum));
    EXPECT_TRUE(EnumType::classof(Enum));
    EXPECT_TRUE(InterfaceType::classof(Interface));
    const UserDefinedType *Types[] = {
        Class,
        Enum,
        Interface,
    };
    for (const Type *Item : Types)
    {
      const Value *ValueObject = Item;
      EXPECT_TRUE(UserDefinedType::classof(ValueObject));
      EXPECT_FALSE(BuiltinType::classof(Item));
      EXPECT_EQ(&Item->type(), &Context.getMetaType());
    }
    EXPECT_EQ(Context.createClassType(Name{}), nullptr);
    EXPECT_EQ(Context.createEnumType(Name{}), nullptr);
    EXPECT_EQ(Context.createInterfaceType(Name{}), nullptr);
    SemanticContext Other(Compilation);
    EXPECT_EQ(Other.createClassType(NameValue), nullptr);
    EXPECT_EQ(Other.createEnumType(NameValue), nullptr);
    EXPECT_EQ(Other.createInterfaceType(NameValue), nullptr);
    EXPECT_EQ(Other.getPointerType(*Class, AccessKind::ReadOnly), nullptr);
  }

  // Builtin type constructors remain builtin when their component is a nominal user type.
  TEST(SemanticTypeTest, UserDefinedElementsDoNotChangeCompositeClassification)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const ClassType *Node = Context.createClassType(Context.namePool().intern("Node"));
    ASSERT_NE(Node, nullptr);
    const ArrayType *Array = Context.getArrayType(*Node, 8);
    const SliceType *Slice = Context.getSliceType(*Node, AccessKind::ReadOnly);
    const PointerType *Pointer = Context.getPointerType(*Node, AccessKind::ReadWrite);
    const ReferenceType *Reference = Context.getReferenceType(*Node, AccessKind::ReadOnly);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(Slice, nullptr);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    const Type *Types[] = {
        Array,
        Slice,
        Pointer,
        Reference,
    };
    for (const Type *Item : Types)
    {
      EXPECT_TRUE(BuiltinType::classof(Item));
      EXPECT_FALSE(UserDefinedType::classof(Item));
    }
    EXPECT_EQ(&Array->elementType(), Node);
    EXPECT_EQ(&Slice->elementType(), Node);
    EXPECT_EQ(&Pointer->pointeeType(), Node);
    EXPECT_EQ(&Reference->referentType(), Node);
  }

  // Rehashing every derived-type store keeps nominal and recursive-component references stable.
  TEST(SemanticTypeTest, DerivedTypeIdentitiesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Name NameValue = Context.namePool().intern("Node");
    const ClassType *Node = Context.createClassType(NameValue);
    ASSERT_NE(Node, nullptr);
    const ArrayType *Array = Context.getArrayType(*Node, 3);
    const SliceType *Slice = Context.getSliceType(*Node, AccessKind::ReadWrite);
    const PointerType *Pointer = Context.getPointerType(*Node, AccessKind::ReadWrite);
    const ReferenceType *Reference = Context.getReferenceType(*Node, AccessKind::ReadOnly);
    for (std::uint32_t Index = 1; Index <= 512; ++Index)
    {
      const Type *Item = Context.getIntegerType(Index, true);
      ASSERT_NE(Context.getArrayType(*Item, Index), nullptr);
      ASSERT_NE(Context.getSliceType(*Item, AccessKind::ReadWrite), nullptr);
      ASSERT_NE(Context.getPointerType(*Item, AccessKind::ReadWrite), nullptr);
      ASSERT_NE(Context.getReferenceType(*Item, AccessKind::ReadOnly), nullptr);
      ASSERT_NE(Context.createClassType(NameValue), nullptr);
    }
    EXPECT_EQ(Context.getArrayType(*Node, 3), Array);
    EXPECT_EQ(Context.getSliceType(*Node, AccessKind::ReadWrite), Slice);
    EXPECT_EQ(Context.getPointerType(*Node, AccessKind::ReadWrite), Pointer);
    EXPECT_EQ(Context.getReferenceType(*Node, AccessKind::ReadOnly), Reference);
    EXPECT_EQ(&Pointer->pointeeType(), Node);
    EXPECT_EQ(Node->name(), NameValue);
    EXPECT_EQ(&Node->context(), &Context);
  }
} // namespace ink::semantic::test
