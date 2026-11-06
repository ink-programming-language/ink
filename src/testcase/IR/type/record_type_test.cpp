#include "ink/core/name/name_pool.h"
#include "ink/ir/enums.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/field.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/method.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/record_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <span>
#include <string>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  // Empty records retain their existing constant-expression construction and semantics.
  static_assert(StructType().getKind() == TypeKind::Struct);
  static_assert(StructType().getSemantics() == TypeSemantics::Value);
  static_assert(ObjectType().getKind() == TypeKind::Object);
  static_assert(ObjectType().getSemantics() == TypeSemantics::Reference);
  static_assert(std::is_base_of_v<RecordType, StructType>);
  static_assert(std::is_base_of_v<RecordType, ObjectType>);
  static_assert(!std::is_destructible_v<RecordType>);
  static_assert(std::is_same_v<decltype(std::declval<const RecordType &>().bases()), std::span<const Value *const>>);
  static_assert(std::is_same_v<decltype(std::declval<const RecordType &>().fields()), std::span<const Field>>);
  static_assert(std::is_same_v<decltype(std::declval<const RecordType &>().methods()), std::span<const Method>>);
  static_assert(std::is_same_v<decltype(std::declval<RecordType &>().bases()), std::span<const Value *const>>);
  static_assert(std::is_same_v<decltype(std::declval<RecordType &>().fields()), std::span<const Field>>);
  static_assert(std::is_same_v<decltype(std::declval<RecordType &>().methods()), std::span<Method>>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().type()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().kind()), MemberKind>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().visibility()), SymbolVisibility>);
  static_assert(std::is_same_v<decltype(std::declval<const Method &>().function()), const Function &>);
  static_assert(std::is_same_v<decltype(std::declval<Method &>().function()), Function &>);
  static_assert(std::is_same_v<decltype(std::declval<RecordType &>().addMethod(std::declval<const FunctionType &>())), Method &>);
  static_assert(std::is_same_v<decltype(std::declval<const Method &>().kind()), MemberKind>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().name()), core::Name>);
  static_assert(std::is_same_v<decltype(std::declval<const Method &>().name()), core::Name>);
  static_assert(!std::is_constructible_v<Field, core::Name, IntegerType &&>);
  static_assert(!std::is_constructible_v<Field, core::Name, const IntegerType &&>);
  static_assert(!std::is_constructible_v<Field, core::Name, IntegerType &&, MemberKind, SymbolVisibility>);
  static_assert(!std::is_constructible_v<Field, core::Name, const IntegerType &&, MemberKind, SymbolVisibility>);
  static_assert(!std::is_copy_constructible_v<Method> && !std::is_copy_assignable_v<Method>);
  static_assert(std::is_move_constructible_v<Method> && std::is_move_assignable_v<Method>);
  static_assert(!std::is_constructible_v<Method, Function &> && !std::is_constructible_v<Method, const Function &>);
  static_assert(!std::is_constructible_v<Method, Function &&> && !std::is_constructible_v<Method, const Function &&>);
  static_assert(std::is_constructible_v<Method, const FunctionType &>);
  static_assert(!std::is_constructible_v<Method, FunctionType &&> && !std::is_constructible_v<Method, const FunctionType &&>);
  static_assert(!std::is_constructible_v<Method, FunctionType &&, core::Name, MemberKind, SymbolVisibility, NativeLinkage>);
  static_assert(!std::is_constructible_v<Method, const FunctionType &&, core::Name, MemberKind, SymbolVisibility, NativeLinkage>);

  template <typename Record>
  concept RecordAcceptsTemporaryBase = requires(Record &Value)
  {
    Value.addBase(StructType{});
  };

  template <typename Record>
  concept RecordAcceptsTemporaryFieldType = requires(Record &Value)
  {
    Value.addField(core::NAME_None, IntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed));
  };

  template <typename Record>
  concept RecordAcceptsTemporaryModifiedFieldType = requires(Record &Value)
  {
    Value.addField(core::NAME_None, IntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), MemberKind::Static, SymbolVisibility::Private);
  };

  template <typename Source>
  concept RecordAcceptsMethodSource = requires(RecordType &Value, Source &&SourceValue)
  {
    Value.addMethod(std::forward<Source>(SourceValue));
  };

  template <typename Source>
  concept RecordAcceptsModifiedMethodSource = requires(RecordType &Value, Source &&SourceValue)
  {
    Value.addMethod(std::forward<Source>(SourceValue), core::NAME_None, MemberKind::Static, SymbolVisibility::Public, NativeLinkage::Export);
  };

  static_assert(!RecordAcceptsTemporaryBase<StructType>);
  static_assert(!RecordAcceptsTemporaryBase<ObjectType>);
  static_assert(!RecordAcceptsTemporaryFieldType<StructType>);
  static_assert(!RecordAcceptsTemporaryFieldType<ObjectType>);
  static_assert(!RecordAcceptsTemporaryModifiedFieldType<StructType>);
  static_assert(!RecordAcceptsTemporaryModifiedFieldType<ObjectType>);
  static_assert(RecordAcceptsMethodSource<FunctionType &> && RecordAcceptsMethodSource<const FunctionType &>);
  static_assert(!RecordAcceptsMethodSource<FunctionType> && !RecordAcceptsMethodSource<const FunctionType>);
  static_assert(!RecordAcceptsModifiedMethodSource<FunctionType> && !RecordAcceptsModifiedMethodSource<const FunctionType>);
  static_assert(!RecordAcceptsMethodSource<Function &> && !RecordAcceptsMethodSource<const Function &>);
  static_assert(!RecordAcceptsMethodSource<Function> && !RecordAcceptsMethodSource<const Function>);

  // A struct retains its distinct kind and spelling while exposing value semantics through Type.
  TEST(IRTypeTest, IdentifiesStructWithValueSemanticsThroughBasePointer)
  {
    const StructType Struct;
    const Type *Base = &Struct;
    EXPECT_EQ(Base->getKind(), TypeKind::Struct);
    EXPECT_EQ(Base->getName(), "struct");
    EXPECT_EQ(typeKindName(TypeKind::Struct), "struct");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Value);
  }

  // An object retains its distinct kind and spelling while overriding value semantics through Type.
  TEST(IRTypeTest, IdentifiesObjectWithReferenceSemanticsThroughBasePointer)
  {
    const ObjectType Object;
    const Type *Base = &Object;
    EXPECT_EQ(Base->getKind(), TypeKind::Object);
    EXPECT_EQ(Base->getName(), "object");
    EXPECT_EQ(typeKindName(TypeKind::Object), "object");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Reference);
  }

  // Record kind checks recognize both record kinds and reject null, scalar, array, and function types.
  TEST(IRTypeTest, DistinguishesStructAndObjectFromOtherKinds)
  {
    const StructType Struct;
    const ObjectType Object;
    const IntegerType Int32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const ArrayType Array(Int32, 2);
    const FunctionType Function(Int32);
    EXPECT_TRUE(RecordType::classof(&Struct));
    EXPECT_TRUE(RecordType::classof(&Object));
    EXPECT_TRUE(StructType::classof(&Struct));
    EXPECT_TRUE(ObjectType::classof(&Object));
    EXPECT_FALSE(StructType::classof(&Object));
    EXPECT_FALSE(ObjectType::classof(&Struct));
    const Type *OtherTypes[] = {
        nullptr,
        &Int32,
        &Bool,
        &Array,
        &Function,
    };
    for (const Type *Other : OtherTypes)
    {
      EXPECT_FALSE(RecordType::classof(Other));
      EXPECT_FALSE(StructType::classof(Other));
      EXPECT_FALSE(ObjectType::classof(Other));
    }
    EXPECT_FALSE(IntegerType::classof(&Struct));
    EXPECT_FALSE(IntegerType::classof(&Object));
    EXPECT_FALSE(ArrayType::classof(&Struct));
    EXPECT_FALSE(ArrayType::classof(&Object));
  }

  // Both concrete record kinds start with empty collections through the shared record interface.
  TEST(IRTypeTest, StartsRecordsWithNoDeclaredBasesFieldsOrMethods)
  {
    const StructType Struct;
    const ObjectType Object;
    const RecordType *Records[] = {
        &Struct,
        &Object,
    };
    for (const RecordType *Record : Records)
    {
      EXPECT_TRUE(Record->bases().empty());
      EXPECT_TRUE(Record->fields().empty());
      EXPECT_TRUE(Record->methods().empty());
    }
  }

  // Both record kinds retain ordered declarations and field modifiers without flattening inherited members or combining overloads.
  TEST(IRTypeTest, SharesOrderedBaseFieldAndMethodStorageBetweenRecordKinds)
  {
    core::NamePool &Names = core::NamePool::instance();
    const core::Name InheritedName = Names.intern("inherited");
    const core::Name CountName = Names.intern("count");
    const core::Name EnabledName = Names.intern("enabled");
    const core::Name ConvertName = Names.intern("convert");
    const IntegerType Int32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const FunctionType FirstSignature(Int32);
    const FunctionType SecondSignature(Bool);
    StructType StructBase;
    const ObjectType ObjectBase;
    StructBase.addField(InheritedName, Bool);
    StructType Struct;
    ObjectType Object;
    RecordType *Records[] = {
        &Struct,
        &Object,
    };
    for (RecordType *Record : Records)
    {
      Record->addBase(StructBase);
      Record->addBase(ObjectBase);
      Record->addField(CountName, Int32);
      Record->addField(EnabledName, Bool, MemberKind::Static, SymbolVisibility::Private);
      Record->addMethod(FirstSignature, ConvertName);
      Record->addMethod(SecondSignature, ConvertName);
      ASSERT_EQ(Record->bases().size(), 2u);
      EXPECT_EQ(Record->bases()[0], &StructBase);
      EXPECT_EQ(Record->bases()[1], &ObjectBase);
      ASSERT_EQ(Record->fields().size(), 2u);
      EXPECT_EQ(Record->fields()[0].name(), CountName);
      EXPECT_EQ(&Record->fields()[0].type(), &Int32);
      EXPECT_EQ(Record->fields()[0].kind(), MemberKind::Instance);
      EXPECT_FALSE(Record->fields()[0].isStatic());
      EXPECT_EQ(Record->fields()[0].visibility(), SymbolVisibility::Public);
      EXPECT_EQ(Record->fields()[1].name(), EnabledName);
      EXPECT_EQ(&Record->fields()[1].type(), &Bool);
      EXPECT_EQ(Record->fields()[1].kind(), MemberKind::Static);
      EXPECT_TRUE(Record->fields()[1].isStatic());
      EXPECT_EQ(Record->fields()[1].visibility(), SymbolVisibility::Private);
      ASSERT_EQ(Record->methods().size(), 2u);
      EXPECT_EQ(Record->methods()[0].name(), ConvertName);
      EXPECT_EQ(&Record->methods()[0].function().signature(), &FirstSignature);
      EXPECT_EQ(Record->methods()[0].kind(), MemberKind::Instance);
      EXPECT_FALSE(Record->methods()[0].isStatic());
      EXPECT_EQ(Record->methods()[1].name(), ConvertName);
      EXPECT_NE(&Record->methods()[0].function(), &Record->methods()[1].function());
      EXPECT_EQ(&Record->methods()[1].function().signature(), &SecondSignature);
      EXPECT_EQ(Record->methods()[1].kind(), MemberKind::Instance);
      EXPECT_FALSE(Record->methods()[1].isStatic());
    }
    EXPECT_EQ(Struct.getSemantics(), TypeSemantics::Value);
    EXPECT_EQ(Object.getSemantics(), TypeSemantics::Reference);
  }

  // Methods own distinct functions with forwarded symbol attributes, and moving a method transfers its function without relocation.
  TEST(IRTypeTest, OwnsIndependentFunctionsAndTransfersTheirIdentityWithMethodMoves)
  {
    const BoolType Bool;
    const FunctionType Signature(Bool);
    const core::Name FunctionName = core::NamePool::instance().intern("shared_method");
    Method DefaultMethod(Signature);
    Method StaticMethod(Signature, FunctionName, MemberKind::Static, SymbolVisibility::Public, NativeLinkage::Export);
    EXPECT_EQ(DefaultMethod.kind(), MemberKind::Instance);
    EXPECT_FALSE(DefaultMethod.isStatic());
    EXPECT_EQ(StaticMethod.kind(), MemberKind::Static);
    EXPECT_TRUE(StaticMethod.isStatic());
    EXPECT_EQ(DefaultMethod.name(), core::NAME_None);
    EXPECT_EQ(DefaultMethod.function().visibility(), SymbolVisibility::Private);
    EXPECT_EQ(DefaultMethod.function().nativeLinkage(), NativeLinkage::None);
    EXPECT_EQ(StaticMethod.name(), FunctionName);
    EXPECT_EQ(StaticMethod.function().visibility(), SymbolVisibility::Public);
    EXPECT_EQ(StaticMethod.function().nativeLinkage(), NativeLinkage::Export);
    EXPECT_NE(&DefaultMethod.function(), &StaticMethod.function());
    Function *MovedFunction = &StaticMethod.function();
    Method Moved(std::move(StaticMethod));
    EXPECT_EQ(&Moved.function(), MovedFunction);
    DefaultMethod = std::move(Moved);
    EXPECT_EQ(&DefaultMethod.function(), MovedFunction);
    EXPECT_EQ(DefaultMethod.kind(), MemberKind::Static);
    EXPECT_EQ(DefaultMethod.name(), FunctionName);
    StructType Struct;
    ObjectType Object;
    RecordType *Records[] = {
        &Struct,
        &Object,
    };
    for (RecordType *Record : Records)
    {
      Function *FirstFunction = &Record->addMethod(Signature, FunctionName).function();
      Method &StaticRecordMethod = Record->addMethod(Signature, FunctionName, MemberKind::Static, SymbolVisibility::Public, NativeLinkage::Export);
      ASSERT_EQ(Record->methods().size(), 2U);
      EXPECT_EQ(Record->methods()[0].kind(), MemberKind::Instance);
      EXPECT_FALSE(Record->methods()[0].isStatic());
      EXPECT_EQ(Record->methods()[1].kind(), MemberKind::Static);
      EXPECT_TRUE(Record->methods()[1].isStatic());
      EXPECT_EQ(&Record->methods()[0].function(), FirstFunction);
      EXPECT_NE(FirstFunction, &StaticRecordMethod.function());
      EXPECT_EQ(&FirstFunction->signature(), &Signature);
      EXPECT_EQ(&StaticRecordMethod.function().signature(), &Signature);
      EXPECT_EQ(FirstFunction->visibility(), SymbolVisibility::Private);
      EXPECT_EQ(FirstFunction->nativeLinkage(), NativeLinkage::None);
      EXPECT_EQ(StaticRecordMethod.function().visibility(), SymbolVisibility::Public);
      EXPECT_EQ(StaticRecordMethod.function().nativeLinkage(), NativeLinkage::Export);
    }
  }

  // Fields and independently owned method functions retain interned name handles after the original spellings change.
  TEST(IRTypeTest, UsesInternedFieldAndFunctionNamesForMethods)
  {
    core::NamePool &Names = core::NamePool::instance();
    const IntegerType Int32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType Signature(Int32);
    std::string FieldSpelling = "stored_field";
    std::string FunctionSpelling = "stored_method";
    const core::Name FieldName = Names.intern(FieldSpelling);
    const core::Name FunctionName = Names.intern(FunctionSpelling);
    const Field FieldValue(FieldName, Int32, MemberKind::Static);
    const Method MethodValue(Signature, FunctionName);
    StructType Struct;
    Struct.addField(FieldName, Int32);
    Struct.addMethod(Signature, FunctionName);
    FieldSpelling.assign("changed_field");
    FunctionSpelling.assign("changed_method");
    EXPECT_EQ(FieldValue.name(), FieldName);
    EXPECT_EQ(FieldValue.name(), Names.intern("stored_field"));
    EXPECT_EQ(Names.text(FieldValue.name()), "stored_field");
    EXPECT_EQ(MethodValue.name(), FunctionName);
    EXPECT_EQ(MethodValue.name(), MethodValue.function().name());
    EXPECT_EQ(&FieldValue.type(), &Int32);
    EXPECT_EQ(FieldValue.kind(), MemberKind::Static);
    EXPECT_TRUE(FieldValue.isStatic());
    EXPECT_EQ(FieldValue.visibility(), SymbolVisibility::Public);
    EXPECT_EQ(&MethodValue.function().signature(), &Signature);
    ASSERT_EQ(Struct.fields().size(), 1u);
    ASSERT_EQ(Struct.methods().size(), 1u);
    EXPECT_EQ(Struct.fields()[0].name(), FieldName);
    EXPECT_EQ(Struct.methods()[0].name(), FunctionName);
    EXPECT_EQ(Struct.methods()[0].name(), MethodValue.function().name());
    EXPECT_EQ(&Struct.fields()[0].type(), &Int32);
    EXPECT_NE(&Struct.methods()[0].function(), &MethodValue.function());
    EXPECT_EQ(&Struct.methods()[0].function().signature(), &Signature);
  }

  // Growing method storage moves wrappers while each owned function, block, instruction, and their iterators keep stable identities.
  TEST(IRTypeTest, KeepsOwnedFunctionBodiesIndependentWhileMethodStorageGrows)
  {
    core::NamePool &Names = core::NamePool::instance();
    TypePool Types;
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    ObjectType Object;
    Function &First = Object.addMethod(Signature, Names.intern("first")).function();
    Function &Second = Object.addMethod(Signature, Names.intern("second")).function();
    BasicBlock &Entry = First.createBasicBlock(Names.intern("entry"));
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Entry);
      Return = &Builder.createReturn();
    }
    auto BlockPosition = First.basicBlocks().begin();
    auto InstructionPosition = Entry.instructions().begin();
    ASSERT_EQ(Object.methods().size(), 2U);
    EXPECT_EQ(&Object.methods()[0].function(), &First);
    EXPECT_EQ(&Object.methods()[1].function(), &Second);
    EXPECT_EQ(&Object.methods()[0].function().signature(), &Object.methods()[1].function().signature());
    ASSERT_EQ(Object.methods()[0].function().basicBlockCount(), 1U);
    EXPECT_EQ(&*Object.methods()[0].function().basicBlocks().begin(), &Entry);
    EXPECT_EQ(Entry.name(), Names.intern("entry"));
    EXPECT_EQ(Entry.parent(), &First);
    EXPECT_TRUE(Object.methods()[1].function().basicBlocks().empty());
    for (unsigned Index = 0; Index < 128; ++Index)
    {
      Object.addMethod(Signature);
    }
    ASSERT_EQ(Object.methods().size(), 130U);
    EXPECT_EQ(&Object.methods()[0].function(), &First);
    EXPECT_EQ(&Object.methods()[1].function(), &Second);
    EXPECT_NE(&First, &Second);
    EXPECT_EQ(&*BlockPosition, &Entry);
    EXPECT_EQ(&*InstructionPosition, Return);
    EXPECT_EQ(&*First.basicBlocks().begin(), &Entry);
    EXPECT_EQ(&*Entry.instructions().begin(), Return);
    EXPECT_EQ(Entry.parent(), &First);
    EXPECT_EQ(Return->parent(), &Entry);
    EXPECT_EQ(First.basicBlockCount(), 1U);
    EXPECT_EQ(Second.basicBlockCount(), 0U);
    Second.createBasicBlock(Names.intern("other_entry"));
    EXPECT_EQ(First.basicBlockCount(), 1U);
    EXPECT_EQ(Second.basicBlockCount(), 1U);
  }

  // A record can be declared before a pointer to itself is added as a field.
  TEST(IRTypeTest, SupportsSelfReferencesThroughBorrowedPointerTypes)
  {
    const core::Name NextName = core::NamePool::instance().intern("next");
    StructType Node;
    const PointerType Next(Node);
    Node.addField(NextName, Next);
    ASSERT_EQ(Node.fields().size(), 1u);
    EXPECT_EQ(Node.fields()[0].name(), NextName);
    EXPECT_EQ(&Node.fields()[0].type(), &Next);
    EXPECT_TRUE(PointerType::classof(&Node.fields()[0].type()));
    const auto &NextType = static_cast<const PointerType &>(Node.fields()[0].type());
    EXPECT_EQ(&NextType.pointeeType(), &Node);
  }
} // namespace ink::ir::test
