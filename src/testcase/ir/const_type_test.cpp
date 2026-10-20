#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <unordered_set>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Const primitive variants retain their concrete kind and layout while their identity and represented mutability differ.
  TEST(IRConstTypeTest, CanonicalizesPrimitiveQualifiersWithoutChangingRepresentation)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const Type *Builtins[] = {
        Types.getBoolType(),
        Types.getInt8Type(),
        Types.getUInt8Type(),
        Types.getInt16Type(),
        Types.getUInt16Type(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
        Types.getFloat32Type(),
        Types.getFloat64Type(),
        Types.getVoidType(),
        Types.getNullPointerType(),
    };
    for (const auto *Mutable : Builtins)
    {
      SCOPED_TRACE(Mutable->toString());
      const auto *Immutable = Types.getQualifiedType(Mutable, false);
      ASSERT_NE(Immutable, nullptr);
      EXPECT_NE(Immutable, Mutable);
      EXPECT_TRUE(Mutable->isMutable());
      EXPECT_FALSE(Immutable->isMutable());
      EXPECT_EQ(Immutable->kind(), Mutable->kind());
      EXPECT_EQ(Immutable->type(), Mutable->type());
      EXPECT_EQ(Immutable->hasLayout(), Mutable->hasLayout());
      EXPECT_EQ(Immutable->size(), Mutable->size());
      EXPECT_EQ(Immutable->alignment(), Mutable->alignment());
      EXPECT_TRUE(Types.owns(Immutable));
      EXPECT_TRUE(Context.containsType(Immutable));
      EXPECT_EQ(Types.getQualifiedType(Mutable, true), Mutable);
      EXPECT_EQ(Types.getQualifiedType(Mutable, false), Immutable);
      EXPECT_EQ(Types.getQualifiedType(Immutable, false), Immutable);
      EXPECT_EQ(Types.getQualifiedType(Immutable, true), Mutable);
      EXPECT_EQ(Immutable->unqualifiedType(), Mutable);
      EXPECT_EQ(Mutable->unqualifiedType(), Mutable);
    }
  }

  // Pointer interning distinguishes both pointer mutability and the pointee's qualified identity without changing existing descriptors.
  TEST(IRConstTypeTest, InternsAllFourPointerQualifierCombinations)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *ConstInteger = Types.getQualifiedType(Integer, false);
    const std::array<const PointerType *, 4> Pointers = {
        Types.getPointerType(Integer, true),
        Types.getPointerType(Integer, false),
        Types.getPointerType(ConstInteger, true),
        Types.getPointerType(ConstInteger, false),
    };
    const std::unordered_set<const PointerType *> Unique(Pointers.begin(), Pointers.end());
    EXPECT_EQ(Unique.size(), Pointers.size());
    for (std::size_t Index = 0; Index != Pointers.size(); ++Index)
    {
      const auto *Pointer = Pointers[Index];
      ASSERT_NE(Pointer, nullptr);
      const bool Mutable = Index % 2 == 0;
      const auto *Pointee = Index < 2 ? Integer : ConstInteger;
      EXPECT_EQ(Pointer->isMutable(), Mutable);
      EXPECT_EQ(Pointer->pointeeType(), Pointee);
      EXPECT_EQ(Pointer, Types.getPointerType(Pointee, Mutable));
      EXPECT_EQ(Types.getQualifiedType(Pointer, Mutable), Pointer);
      EXPECT_EQ(Types.getQualifiedType(Pointer, !Mutable), Pointers[Index ^ 1]);
      EXPECT_EQ(Pointer->size(), Target.pointerByteWidth());
    }
    for (std::uint64_t Count = 0; Count != 256; ++Count)
    {
      const auto *Array = Types.getArrayType(Integer, Count);
      ASSERT_NE(Types.getPointerType(Array, true), nullptr);
      ASSERT_NE(Types.getPointerType(Array, false), nullptr);
    }
    EXPECT_EQ(Types.getPointerType(Integer), Pointers[0]);
    EXPECT_EQ(Types.getPointerType(Integer, false), Pointers[1]);
    EXPECT_EQ(Types.getPointerType(ConstInteger), Pointers[2]);
    EXPECT_EQ(Types.getPointerType(ConstInteger, false), Pointers[3]);
    EXPECT_TRUE(Integer->isMutable());
    EXPECT_FALSE(ConstInteger->isMutable());
    EXPECT_EQ(ConstInteger->toString(), "const i32");
    EXPECT_EQ(Pointers[0]->toString(), "*i32");
    EXPECT_EQ(Pointers[1]->toString(), "const *i32");
    EXPECT_EQ(Pointers[2]->toString(), "*const i32");
    EXPECT_EQ(Pointers[3]->toString(), "const *const i32");
  }

  // Structural qualification changes only the outer descriptor while preserving nested array, reference and function components.
  TEST(IRConstTypeTest, QualifiesStructuralTypesOneLayerAtATime)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *ConstInteger = Types.getQualifiedType(Integer, false);
    const auto *ReadonlyPointer = Types.getPointerType(ConstInteger);
    const auto *Array = Types.getArrayType(ReadonlyPointer, 3);
    const auto *Reference = Types.getReferenceType(ReadonlyPointer);
    const auto *Signature = Types.getFunctionType(ReadonlyPointer, {Integer, {ReadonlyPointer, true}});
    const Type *Structural[] = {
        Array,
        Reference,
        Signature,
    };
    for (const auto *Mutable : Structural)
    {
      const auto *Immutable = Types.getQualifiedType(Mutable, false);
      ASSERT_NE(Immutable, nullptr);
      EXPECT_NE(Immutable, Mutable);
      EXPECT_EQ(Immutable->kind(), Mutable->kind());
      EXPECT_FALSE(Immutable->isMutable());
      EXPECT_EQ(Immutable->size(), Mutable->size());
      EXPECT_EQ(Immutable->alignment(), Mutable->alignment());
      EXPECT_EQ(Types.getQualifiedType(Immutable, true), Mutable);
      EXPECT_EQ(Types.getQualifiedType(Mutable, false), Immutable);
    }
    const auto *ConstArray = static_cast<const ArrayType *>(Types.getQualifiedType(Array, false));
    EXPECT_EQ(ConstArray->elementType(), ReadonlyPointer);
    EXPECT_EQ(ConstArray->elementCount(), 3U);
    EXPECT_NE(ConstArray, Types.getArrayType(Types.getQualifiedType(ReadonlyPointer, false), 3));
    const auto *ConstReference = static_cast<const ReferenceType *>(Types.getQualifiedType(Reference, false));
    EXPECT_EQ(ConstReference->referencedType(), ReadonlyPointer);
    EXPECT_EQ(ConstReference, Types.getReferenceType(ReadonlyPointer, false));
    EXPECT_NE(ConstReference, Types.getReferenceType(Types.getQualifiedType(ReadonlyPointer, false)));
    const auto *ConstSignature = static_cast<const FunctionType *>(Types.getQualifiedType(Signature, false));
    EXPECT_EQ(ConstSignature->returnType(), ReadonlyPointer);
    ASSERT_EQ(ConstSignature->parameterCount(), 2U);
    EXPECT_EQ(ConstSignature->parameters()[0].ParameterType, Integer);
    EXPECT_EQ(ConstSignature->parameters()[1].ParameterType, ReadonlyPointer);
    EXPECT_TRUE(ConstSignature->parameters()[1].Variadic);
    EXPECT_TRUE(ReadonlyPointer->isMutable());
    EXPECT_FALSE(ReadonlyPointer->pointeeType()->isMutable());
  }

  // Qualified signature components retain distinct function identities, and qualifier factories reject all foreign-context inputs.
  TEST(IRConstTypeTest, PreservesQualifiedSignatureIdentityAndContextOwnership)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    auto &OtherTypes = Foreign.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *ConstInteger = Types.getQualifiedType(Integer, false);
    const auto *MutableSignature = Types.getFunctionType(Integer, {Integer});
    const auto *ConstParameterSignature = Types.getFunctionType(Integer, {ConstInteger});
    const auto *ConstResultSignature = Types.getFunctionType(ConstInteger, {Integer});
    EXPECT_NE(MutableSignature, ConstParameterSignature);
    EXPECT_NE(MutableSignature, ConstResultSignature);
    EXPECT_EQ(ConstParameterSignature, Types.getFunctionType(Integer, {ConstInteger}));
    EXPECT_EQ(ConstResultSignature, Types.getFunctionType(ConstInteger, {Integer}));
    const Type *ForeignTypes[] = {
        OtherTypes.getInt32Type(),
        OtherTypes.getQualifiedType(OtherTypes.getInt32Type(), false),
        OtherTypes.getPointerType(OtherTypes.getInt32Type(), false),
    };
    for (const auto *ForeignType : ForeignTypes)
    {
      ASSERT_NE(ForeignType, nullptr);
      EXPECT_EQ(Types.getQualifiedType(ForeignType, true), nullptr);
      EXPECT_EQ(Types.getQualifiedType(ForeignType, false), nullptr);
      EXPECT_EQ(Types.getPointerType(ForeignType, false), nullptr);
      EXPECT_EQ(Types.getReferenceType(ForeignType, false), nullptr);
      EXPECT_FALSE(Types.owns(ForeignType));
    }
    EXPECT_EQ(Types.getQualifiedType(nullptr, true), nullptr);
    EXPECT_EQ(Types.getQualifiedType(nullptr, false), nullptr);
  }

  // A qualified nominal view keeps its declaration's evolving fields and method identities instead of copying member metadata.
  TEST(IRConstTypeTest, NominalQualifierViewsShareDeclarationMetadata)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    Module Owner(Context);
    auto &Types = Context.typePool();
    auto *Declaration = Owner.createStructType(8, 8);
    auto *OtherDeclaration = Owner.createStructType(8, 8);
    ASSERT_NE(Declaration, nullptr);
    ASSERT_NE(OtherDeclaration, nullptr);
    const auto *Qualified = Types.getQualifiedType(Declaration, false);
    ASSERT_TRUE(StructType::classof(Qualified));
    const auto *ConstDeclaration = static_cast<const StructType *>(Qualified);
    EXPECT_FALSE(ConstDeclaration->isMutable());
    EXPECT_EQ(ConstDeclaration->unqualifiedType(), Declaration);
    EXPECT_EQ(ConstDeclaration->declarationType(), Declaration);
    EXPECT_EQ(Types.getQualifiedType(ConstDeclaration, true), Declaration);
    EXPECT_NE(ConstDeclaration, Types.getQualifiedType(OtherDeclaration, false));
    EXPECT_TRUE(Context.containsType(ConstDeclaration));
    EXPECT_TRUE(Owner.ownsType(ConstDeclaration));
    Module Peer(Context);
    EXPECT_FALSE(Peer.ownsType(ConstDeclaration));
    EXPECT_EQ(Foreign.typePool().getQualifiedType(ConstDeclaration, true), nullptr);
    EXPECT_EQ(Foreign.typePool().getQualifiedType(Declaration, false), nullptr);
    EXPECT_EQ(ConstDeclaration->fieldCount(), 0U);
    EXPECT_EQ(ConstDeclaration->methodCount(), 0U);
    const auto FieldName = core::NamePool::instance().intern("IRConstTypeTest.Value");
    const auto MethodName = core::NamePool::instance().intern("IRConstTypeTest.Method");
    ASSERT_TRUE(Declaration->addField(Field(Types.getInt32Type(), FieldName, 0)));
    ASSERT_TRUE(Declaration->addMethod(Method(std::make_unique<Function>(*Types.getFunctionType(Types.getVoidType()), MethodName))));
    EXPECT_EQ(ConstDeclaration->fieldCount(), 1U);
    EXPECT_EQ(ConstDeclaration->methodCount(), 1U);
    EXPECT_EQ(ConstDeclaration->field(0), Declaration->field(0));
    EXPECT_EQ(ConstDeclaration->method(0), Declaration->method(0));
    ASSERT_NE(ConstDeclaration->method(0), nullptr);
    EXPECT_EQ(ConstDeclaration->method(0)->function(), Declaration->method(0)->function());
    EXPECT_EQ(ConstDeclaration->findFields(FieldName), Declaration->findFields(FieldName));
    EXPECT_EQ(ConstDeclaration->findMethods(MethodName), Declaration->findMethods(MethodName));
    Declaration->freeze();
    EXPECT_TRUE(ConstDeclaration->isFrozen());
  }

  // Retiring a nominal declaration invalidates its pooled qualifier view before any later lookup can follow the retired declaration.
  TEST(IRConstTypeTest, RejectsNominalQualifierViewsAfterDeclarationDestruction)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const Type *View = nullptr;
    {
      Module Owner(Context);
      const auto *Declaration = Owner.createStructType(8, 8);
      ASSERT_NE(Declaration, nullptr);
      View = Types.getQualifiedType(Declaration, false);
      ASSERT_NE(View, nullptr);
      EXPECT_TRUE(Context.containsType(View));
      EXPECT_EQ(Types.getQualifiedType(View, true), Declaration);
    }
    EXPECT_FALSE(Context.containsType(View));
    EXPECT_FALSE(Types.owns(View));
    EXPECT_EQ(Types.getQualifiedType(View, true), nullptr);
    EXPECT_EQ(Types.getQualifiedType(View, false), nullptr);
    EXPECT_EQ(Types.getPointerType(View), nullptr);
    Module Next(Context);
    const auto *NextDeclaration = Next.createStructType(8, 8);
    ASSERT_NE(NextDeclaration, nullptr);
    const auto *NextView = Types.getQualifiedType(NextDeclaration, false);
    ASSERT_NE(NextView, nullptr);
    EXPECT_NE(NextView, View);
    EXPECT_EQ(NextView->unqualifiedType(), NextDeclaration);
    EXPECT_TRUE(Context.containsType(NextView));
  }

  // Constant initialization ignores only the destination's outer qualifier and preserves context and nested pointee distinctions.
  TEST(IRConstTypeTest, ConstantInitializationRemovesOnlyTopLevelConst)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *ConstInteger = Types.getQualifiedType(Integer, false);
    const auto *Seven = Context.constantPool().getSignedIntegerConstant(Integer, 7);
    ASSERT_NE(Seven, nullptr);
    EXPECT_TRUE(Seven->canInitialize(ConstInteger));
    EXPECT_FALSE(Seven->canInitialize(Foreign.typePool().getQualifiedType(Foreign.typePool().getInt32Type(), false)));
    const auto *Pointer = Types.getPointerType(Integer);
    const auto *ReadonlyPointer = Types.getPointerType(ConstInteger);
    const auto *Array = Types.getArrayType(Pointer, 1);
    const auto *ReadonlyArray = Types.getArrayType(ReadonlyPointer, 1);
    const auto *Value = Context.constantPool().getArrayConstant(Array, {Context.constantPool().getNullPointerConstant()});
    ASSERT_NE(Value, nullptr);
    EXPECT_TRUE(Value->canInitialize(Types.getQualifiedType(Array, false)));
    EXPECT_FALSE(Value->canInitialize(ReadonlyArray));
    EXPECT_FALSE(Value->canInitialize(Types.getQualifiedType(ReadonlyArray, false)));
  }

  // Negation consumes a const-qualified numeric parameter by value and gives the computed result its mutable canonical type.
  TEST(IRConstTypeTest, NumericResultsDiscardTopLevelConst)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *ConstInteger = Types.getQualifiedType(Integer, false);
    Function Owner(*Types.getFunctionType(Integer, {ConstInteger}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Negation = Builder.createNeg(*Owner.parameter(0));
    ASSERT_NE(Negation, nullptr);
    EXPECT_EQ(Negation->type(), Integer);
    EXPECT_NE(Builder.createReturn(*Negation), nullptr);
  }
} // namespace ink::ir::test
