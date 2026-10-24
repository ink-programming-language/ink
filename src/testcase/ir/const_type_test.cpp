#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <cstdint>
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
        Types.get<ink::ir::BoolType>(),
        Types.get<ink::ir::IntegerType>(8, true),
        Types.get<ink::ir::IntegerType>(8, false),
        Types.get<ink::ir::IntegerType>(16, true),
        Types.get<ink::ir::IntegerType>(16, false),
        Types.get<ink::ir::IntegerType>(32, true),
        Types.get<ink::ir::IntegerType>(32, false),
        Types.get<ink::ir::IntegerType>(64, true),
        Types.get<ink::ir::IntegerType>(64, false),
        Types.get<ink::ir::FloatType>(32),
        Types.get<ink::ir::FloatType>(64),
        Types.get<ink::ir::VoidType>(),
        Types.get<ink::ir::NullPointerType>(),
    };
    for (const auto *Mutable : Builtins)
    {
      SCOPED_TRACE(Mutable->toString());
      const auto *Immutable = Types.get(Mutable, false);
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
      EXPECT_EQ(Types.get(Mutable, true), Mutable);
      EXPECT_EQ(Types.get(Mutable, false), Immutable);
      EXPECT_EQ(Types.get(Immutable, false), Immutable);
      EXPECT_EQ(Types.get(Immutable, true), Mutable);
      EXPECT_EQ(Immutable->unqualifiedType(), Mutable);
      EXPECT_EQ(Mutable->unqualifiedType(), Mutable);
    }
  }

  // Pointer interning distinguishes both pointer mutability and the pointee's qualified identity without changing existing descriptors.
  TEST(IRConstTypeTest, InternsAllFourPointerQualifierCombinations)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    const std::array<const PointerType *, 4> Pointers = {
        Types.get<ink::ir::PointerType>(Integer, true),
        Types.get<ink::ir::PointerType>(Integer, false),
        Types.get<ink::ir::PointerType>(ConstInteger, true),
        Types.get<ink::ir::PointerType>(ConstInteger, false),
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
      EXPECT_EQ(Pointer, Types.get<ink::ir::PointerType>(Pointee, Mutable));
      EXPECT_EQ(Types.get(Pointer, Mutable), Pointer);
      EXPECT_EQ(Types.get(Pointer, !Mutable), Pointers[Index ^ 1]);
      EXPECT_EQ(Pointer->size(), Target.pointerByteWidth());
    }
    for (std::uint64_t Count = 0; Count != 256; ++Count)
    {
      const auto *Array = Types.get<ink::ir::ArrayType>(Integer, Count);
      ASSERT_NE(Types.get<ink::ir::PointerType>(Array, true), nullptr);
      ASSERT_NE(Types.get<ink::ir::PointerType>(Array, false), nullptr);
    }
    EXPECT_EQ(Types.get<ink::ir::PointerType>(Integer), Pointers[0]);
    EXPECT_EQ(Types.get<ink::ir::PointerType>(Integer, false), Pointers[1]);
    EXPECT_EQ(Types.get<ink::ir::PointerType>(ConstInteger), Pointers[2]);
    EXPECT_EQ(Types.get<ink::ir::PointerType>(ConstInteger, false), Pointers[3]);
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
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    const auto *ReadonlyPointer = Types.get<ink::ir::PointerType>(ConstInteger);
    const auto *Array = Types.get<ink::ir::ArrayType>(ReadonlyPointer, 3);
    const auto *Reference = Types.get<ink::ir::ReferenceType>(ReadonlyPointer);
    const auto *Signature = Types.get<ink::ir::FunctionType>(ReadonlyPointer, {Integer, {ReadonlyPointer, true}});
    const Type *Structural[] = {
        Array,
        Reference,
        Signature,
    };
    for (const auto *Mutable : Structural)
    {
      const auto *Immutable = Types.get(Mutable, false);
      ASSERT_NE(Immutable, nullptr);
      EXPECT_NE(Immutable, Mutable);
      EXPECT_EQ(Immutable->kind(), Mutable->kind());
      EXPECT_FALSE(Immutable->isMutable());
      EXPECT_EQ(Immutable->size(), Mutable->size());
      EXPECT_EQ(Immutable->alignment(), Mutable->alignment());
      EXPECT_EQ(Types.get(Immutable, true), Mutable);
      EXPECT_EQ(Types.get(Mutable, false), Immutable);
    }
    const auto *ConstArray = static_cast<const ArrayType *>(Types.get(Array, false));
    EXPECT_EQ(ConstArray->elementType(), ReadonlyPointer);
    EXPECT_EQ(ConstArray->elementCount(), 3U);
    EXPECT_NE(ConstArray, Types.get<ink::ir::ArrayType>(Types.get(ReadonlyPointer, false), 3));
    const auto *ConstReference = static_cast<const ReferenceType *>(Types.get(Reference, false));
    EXPECT_EQ(ConstReference->referencedType(), ReadonlyPointer);
    EXPECT_EQ(ConstReference, Types.get<ink::ir::ReferenceType>(ReadonlyPointer, false));
    EXPECT_NE(ConstReference, Types.get<ink::ir::ReferenceType>(Types.get(ReadonlyPointer, false)));
    const auto *ConstSignature = static_cast<const FunctionType *>(Types.get(Signature, false));
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
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    const auto *MutableSignature = Types.get<ink::ir::FunctionType>(Integer, {Integer});
    const auto *ConstParameterSignature = Types.get<ink::ir::FunctionType>(Integer, {ConstInteger});
    const auto *ConstResultSignature = Types.get<ink::ir::FunctionType>(ConstInteger, {Integer});
    EXPECT_NE(MutableSignature, ConstParameterSignature);
    EXPECT_NE(MutableSignature, ConstResultSignature);
    EXPECT_EQ(ConstParameterSignature, Types.get<ink::ir::FunctionType>(Integer, {ConstInteger}));
    EXPECT_EQ(ConstResultSignature, Types.get<ink::ir::FunctionType>(ConstInteger, {Integer}));
    const Type *ForeignTypes[] = {
        OtherTypes.get<ink::ir::IntegerType>(32, true),
        OtherTypes.get(OtherTypes.get<ink::ir::IntegerType>(32, true), false),
        OtherTypes.get<ink::ir::PointerType>(OtherTypes.get<ink::ir::IntegerType>(32, true), false),
    };
    for (const auto *ForeignType : ForeignTypes)
    {
      ASSERT_NE(ForeignType, nullptr);
      EXPECT_EQ(Types.get(ForeignType, true), nullptr);
      EXPECT_EQ(Types.get(ForeignType, false), nullptr);
      EXPECT_EQ(Types.get<ink::ir::PointerType>(ForeignType, false), nullptr);
      EXPECT_EQ(Types.get<ink::ir::ReferenceType>(ForeignType, false), nullptr);
      EXPECT_FALSE(Types.owns(ForeignType));
    }
    EXPECT_EQ(Types.get(nullptr, true), nullptr);
    EXPECT_EQ(Types.get(nullptr, false), nullptr);
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
    const auto *Qualified = Types.get(Declaration, false);
    ASSERT_TRUE(StructType::classof(Qualified));
    const auto *ConstDeclaration = static_cast<const StructType *>(Qualified);
    EXPECT_FALSE(ConstDeclaration->isMutable());
    EXPECT_EQ(ConstDeclaration->unqualifiedType(), Declaration);
    EXPECT_EQ(ConstDeclaration->declarationType(), Declaration);
    EXPECT_EQ(Types.get(ConstDeclaration, true), Declaration);
    EXPECT_NE(ConstDeclaration, Types.get(OtherDeclaration, false));
    EXPECT_TRUE(Context.containsType(ConstDeclaration));
    EXPECT_TRUE(Owner.ownsType(ConstDeclaration));
    Module Peer(Context);
    EXPECT_FALSE(Peer.ownsType(ConstDeclaration));
    EXPECT_EQ(Foreign.typePool().get(ConstDeclaration, true), nullptr);
    EXPECT_EQ(Foreign.typePool().get(Declaration, false), nullptr);
    EXPECT_EQ(ConstDeclaration->fieldCount(), 0U);
    EXPECT_EQ(ConstDeclaration->methodCount(), 0U);
    const auto FieldName = core::NamePool::instance().intern("IRConstTypeTest.Value");
    const auto MethodName = core::NamePool::instance().intern("IRConstTypeTest.Method");
    ASSERT_TRUE(Declaration->addField(Field(Types.get<ink::ir::IntegerType>(32, true), FieldName, 0)));
    ASSERT_TRUE(Declaration->addMethod(Method(std::make_unique<Function>(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()), MethodName))));
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
      View = Types.get(Declaration, false);
      ASSERT_NE(View, nullptr);
      EXPECT_TRUE(Context.containsType(View));
      EXPECT_EQ(Types.get(View, true), Declaration);
    }
    EXPECT_FALSE(Context.containsType(View));
    EXPECT_FALSE(Types.owns(View));
    EXPECT_EQ(Types.get(View, true), nullptr);
    EXPECT_EQ(Types.get(View, false), nullptr);
    EXPECT_EQ(Types.get<ink::ir::PointerType>(View), nullptr);
    Module Next(Context);
    const auto *NextDeclaration = Next.createStructType(8, 8);
    ASSERT_NE(NextDeclaration, nullptr);
    const auto *NextView = Types.get(NextDeclaration, false);
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
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    ASSERT_NE(Seven, nullptr);
    EXPECT_TRUE(Seven->canInitialize(ConstInteger));
    EXPECT_FALSE(Seven->canInitialize(Foreign.typePool().get(Foreign.typePool().get<ink::ir::IntegerType>(32, true), false)));
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *ReadonlyPointer = Types.get<ink::ir::PointerType>(ConstInteger);
    const auto *Array = Types.get<ink::ir::ArrayType>(Pointer, 1);
    const auto *ReadonlyArray = Types.get<ink::ir::ArrayType>(ReadonlyPointer, 1);
    const auto *Value = Context.constantPool().get<ink::ir::ArrayConstant>(Array, {Context.constantPool().get<ink::ir::NullPointerConstant>()});
    ASSERT_NE(Value, nullptr);
    EXPECT_TRUE(Value->canInitialize(Types.get(Array, false)));
    EXPECT_FALSE(Value->canInitialize(ReadonlyArray));
    EXPECT_FALSE(Value->canInitialize(Types.get(ReadonlyArray, false)));
  }

  // Negation consumes a const-qualified numeric parameter by value and gives the computed result its mutable canonical type.
  TEST(IRConstTypeTest, NumericResultsDiscardTopLevelConst)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    Function Owner(*Types.get<ink::ir::FunctionType>(Integer, {ConstInteger}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Negation = Builder.createNeg(*Owner.parameter(0));
    ASSERT_NE(Negation, nullptr);
    EXPECT_EQ(Negation->type(), Integer);
    EXPECT_NE(Builder.createReturn(*Negation), nullptr);
  }
} // namespace ink::ir::test
