#include "ink/core/name/name_pool.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace ink::ir::test
{
  static_assert(!std::is_copy_constructible_v<TypePool>);
  static_assert(!std::is_move_constructible_v<TypePool>);
  static_assert(!std::is_copy_constructible_v<ArrayType>);
  static_assert(!std::is_move_constructible_v<ArrayType>);
  static_assert(!std::is_copy_constructible_v<ReferenceType>);
  static_assert(!std::is_move_constructible_v<ReferenceType>);
  static_assert(std::is_same_v<decltype(std::declval<TypePool &>().createStructType()), StructType &>);
  static_assert(std::is_same_v<decltype(std::declval<TypePool &>().createObjectType()), ObjectType &>);

  struct CollidingTypeHash
  {
    public:
      using is_transparent = void;

      template <typename T>
      std::size_t operator()(const T &) const noexcept
      {
        return 0;
      }
  };

  template <typename T>
  concept PoolAcceptsTemporaryArrayElement = requires(TypePool &Pool)
  {
    Pool.getArrayType(std::declval<T &&>(), 1);
  };

  template <typename T>
  concept PoolAcceptsTemporaryReferenceTarget = requires(TypePool &Pool)
  {
    Pool.getReferenceType(std::declval<T &&>());
  };

  static_assert(!PoolAcceptsTemporaryArrayElement<IntegerType>);
  static_assert(!PoolAcceptsTemporaryArrayElement<const IntegerType>);
  static_assert(!PoolAcceptsTemporaryReferenceTarget<IntegerType>);
  static_assert(!PoolAcceptsTemporaryReferenceTarget<const IntegerType>);
  static_assert(!PoolAcceptsTemporaryArrayElement<Parameter>);
  static_assert(!PoolAcceptsTemporaryArrayElement<const Parameter>);
  static_assert(!PoolAcceptsTemporaryReferenceTarget<Parameter>);
  static_assert(!PoolAcceptsTemporaryReferenceTarget<const Parameter>);
  static_assert(std::is_same_v<decltype(std::declval<const ArrayType &>().elementType()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const ReferenceType &>().referencedType()), const Value &>);

  // Record creation always produces fresh mutable identities, and growing both owned collections preserves existing declarations.
  TEST(IRTypePoolTest, CreatesDistinctMutableRecordsWithStableIdentityWhileGrowing)
  {
    TypePool Pool;
    StructType &FirstStruct = Pool.createStructType();
    StructType &SecondStruct = Pool.createStructType();
    ObjectType &FirstObject = Pool.createObjectType();
    ObjectType &SecondObject = Pool.createObjectType();
    EXPECT_NE(&FirstStruct, &SecondStruct);
    EXPECT_NE(&FirstObject, &SecondObject);
    const core::Name FieldName = core::NamePool::instance().intern("enabled");
    FirstStruct.addField(FieldName, Pool.getBoolType());
    const ReferenceType &FirstReference = Pool.getReferenceType(FirstStruct);
    FirstObject.addField(FieldName, FirstReference);
    std::unordered_set<const StructType *> Structs{&FirstStruct, &SecondStruct};
    std::unordered_set<const ObjectType *> Objects{&FirstObject, &SecondObject};
    for (unsigned Index = 0; Index < 128; ++Index)
    {
      EXPECT_TRUE(Structs.insert(&Pool.createStructType()).second);
      EXPECT_TRUE(Objects.insert(&Pool.createObjectType()).second);
    }
    EXPECT_EQ(Structs.size(), 130U);
    EXPECT_EQ(Objects.size(), 130U);
    ASSERT_EQ(FirstStruct.fields().size(), 1U);
    ASSERT_EQ(FirstObject.fields().size(), 1U);
    EXPECT_EQ(FirstStruct.fields()[0].name(), FieldName);
    EXPECT_EQ(&FirstStruct.fields()[0].type(), &Pool.getBoolType());
    EXPECT_EQ(&FirstObject.fields()[0].type(), &FirstReference);
    EXPECT_EQ(&FirstReference.referencedType(), &FirstStruct);
    EXPECT_EQ(&Pool.getReferenceType(FirstStruct), &FirstReference);
    EXPECT_TRUE(SecondStruct.fields().empty());
    EXPECT_TRUE(SecondObject.fields().empty());
    EXPECT_EQ(FirstStruct.getSemantics(), TypeSemantics::Value);
    EXPECT_EQ(FirstObject.getSemantics(), TypeSemantics::Reference);
  }

  // Mutable record declarations remain identity keys for cached references, arrays, and signatures through edits and cache growth.
  TEST(IRTypePoolTest, KeepsCompositeCachesStableWhenOwnedRecordDeclarationsChange)
  {
    TypePool Pool;
    core::NamePool &Names = core::NamePool::instance();
    StructType &Struct = Pool.createStructType();
    ObjectType &Object = Pool.createObjectType();
    const ReferenceType &StructReference = Pool.getReferenceType(Struct);
    const ReferenceType &ObjectReference = Pool.getReferenceType(Object);
    const ArrayType &StructArray = Pool.getArrayType(Struct, 4);
    const ArrayType &ObjectArray = Pool.getArrayType(Object, 4);
    const std::array Parameters{Parameter(Object, Names.intern("owner"))};
    const FunctionType &Signature = Pool.getFunctionType(Struct, Parameters);
    const auto ReferenceHash = StructReference.hash();
    const auto ArrayHash = StructArray.hash();
    const auto SignatureHash = Signature.hash();
    Struct.addBase(Pool.createStructType());
    Struct.addField(Names.intern("owner"), ObjectReference);
    Object.addField(Names.intern("payload"), Struct);
    Object.addField(Names.intern("next"), ObjectReference);
    Method &OwnedMethod = Object.addMethod(Signature, Names.intern("payload"));
    Function *MethodFunction = &OwnedMethod.function();
    for (std::uint64_t Count = 1; Count <= 128; ++Count)
    {
      StructType &NextStruct = Pool.createStructType();
      ObjectType &NextObject = Pool.createObjectType();
      Pool.getReferenceType(NextStruct);
      Pool.getReferenceType(NextObject);
      Pool.getArrayType(NextStruct, Count);
      Pool.getArrayType(NextObject, Count);
      const std::array NextParameters{Parameter(NextObject)};
      Pool.getFunctionType(NextStruct, NextParameters);
    }
    EXPECT_EQ(&Pool.getReferenceType(Struct), &StructReference);
    EXPECT_EQ(&Pool.getReferenceType(Object), &ObjectReference);
    EXPECT_EQ(&Pool.getArrayType(Struct, 4), &StructArray);
    EXPECT_EQ(&Pool.getArrayType(Object, 4), &ObjectArray);
    EXPECT_EQ(&Pool.getFunctionType(Struct, Parameters), &Signature);
    EXPECT_EQ(StructReference.hash(), ReferenceHash);
    EXPECT_EQ(StructArray.hash(), ArrayHash);
    EXPECT_EQ(Signature.hash(), SignatureHash);
    EXPECT_EQ(&StructArray.elementType(), &Struct);
    EXPECT_EQ(&ObjectArray.elementType(), &Object);
    EXPECT_EQ(&Signature.returnType(), &Struct);
    ASSERT_EQ(Signature.parameters().size(), 1U);
    EXPECT_EQ(&Signature.parameters()[0].type(), &Object);
    ASSERT_EQ(Object.methods().size(), 1U);
    EXPECT_EQ(&Object.methods()[0].function(), MethodFunction);
    EXPECT_EQ(&MethodFunction->signature(), &Signature);
    EXPECT_EQ(Struct.bases().size(), 1U);
    EXPECT_EQ(Struct.fields().size(), 1U);
    EXPECT_EQ(Object.fields().size(), 2U);
  }

  // Growing the pools preserves symbolic operands and keeps equally named generic parameters distinct.
  TEST(IRTypePoolTest, PreservesGenericCompositesWhileThePoolGrows)
  {
    const Parameter T(MetaType::instance());
    const Parameter U(MetaType::instance());
    TypePool Pool;
    const auto *ArrayT = &Pool.getArrayType(T, 1);
    const auto *ArrayU = &Pool.getArrayType(U, 1);
    const auto *ReferenceT = &Pool.getReferenceType(T);
    const auto *ReferenceU = &Pool.getReferenceType(U);
    const auto *Nested = &Pool.getReferenceType(*ArrayT);
    for (std::uint64_t Count = 2; Count < 512; ++Count)
    {
      const auto &Array = Pool.getArrayType(T, Count);
      Pool.getReferenceType(Array);
    }
    EXPECT_EQ(ArrayT, &Pool.getArrayType(T, 1));
    EXPECT_EQ(ArrayU, &Pool.getArrayType(U, 1));
    EXPECT_EQ(ReferenceT, &Pool.getReferenceType(T));
    EXPECT_EQ(ReferenceU, &Pool.getReferenceType(U));
    EXPECT_NE(ArrayT, ArrayU);
    EXPECT_NE(ReferenceT, ReferenceU);
    EXPECT_EQ(&ArrayT->elementType(), &T);
    EXPECT_EQ(&ArrayU->elementType(), &U);
    EXPECT_EQ(&ReferenceT->referencedType(), &T);
    EXPECT_EQ(&ReferenceU->referencedType(), &U);
    EXPECT_EQ(Nested, &Pool.getReferenceType(*ArrayT));
    EXPECT_EQ(&Nested->referencedType(), ArrayT);
  }

  // Transparent lookup and equality use generic parameter identity rather than their common MetaType.
  TEST(IRTypePoolTest, FindsGenericCompositeHashSetEntriesAfterRehash)
  {
    const Parameter T(MetaType::instance());
    const Parameter U(MetaType::instance());
    std::unordered_set<ArrayType, core::Hash<ArrayType>, std::equal_to<>> Arrays;
    const auto *ArrayT = &*Arrays.emplace(T, 4).first;
    const auto *ArrayU = &*Arrays.emplace(U, 4).first;
    Arrays.rehash(Arrays.bucket_count() * 2 + 1);
    const auto FoundArrayT = Arrays.find(ArrayType::Key{&T, 4});
    const auto FoundArrayU = Arrays.find(ArrayType::Key{&U, 4});
    ASSERT_NE(FoundArrayT, Arrays.end());
    ASSERT_NE(FoundArrayU, Arrays.end());
    EXPECT_EQ(&*FoundArrayT, ArrayT);
    EXPECT_EQ(&*FoundArrayU, ArrayU);
    EXPECT_EQ(Arrays.find(ArrayType::Key{&T, 5}), Arrays.end());
    const core::Hash<ArrayType> ArrayHash;
    EXPECT_EQ(ArrayHash(*ArrayT), ArrayHash(ArrayType::Key{&T, 4}));
    EXPECT_EQ(ArrayHash(*ArrayU), ArrayHash(ArrayType::Key{&U, 4}));
    std::unordered_set<ReferenceType, core::Hash<ReferenceType>, std::equal_to<>> References;
    const auto *ReferenceT = &*References.emplace(T).first;
    const auto *ReferenceU = &*References.emplace(U).first;
    References.rehash(References.bucket_count() * 2 + 1);
    const auto FoundReferenceT = References.find(&T);
    const auto FoundReferenceU = References.find(static_cast<const Value *>(&U));
    ASSERT_NE(FoundReferenceT, References.end());
    ASSERT_NE(FoundReferenceU, References.end());
    EXPECT_EQ(&*FoundReferenceT, ReferenceT);
    EXPECT_EQ(&*FoundReferenceU, ReferenceU);
    const core::Hash<ReferenceType> ReferenceHash;
    EXPECT_EQ(ReferenceHash(*ReferenceT), ReferenceHash(&T));
    EXPECT_EQ(ReferenceHash(*ReferenceU), ReferenceHash(static_cast<const Value *>(&U)));
    EXPECT_TRUE(*ReferenceT == &T);
    EXPECT_TRUE(&T == *ReferenceT);
    EXPECT_FALSE(*ReferenceT == &U);
    EXPECT_FALSE(*ReferenceU == &T);
  }

  // A runtime data parameter is not a type value and cannot enter any composite type position.
  TEST(IRTypePoolDeathTest, RejectsNonTypeCompositeOperands)
  {
    TypePool Pool;
    const Parameter Data(Pool.getBoolType());
    EXPECT_DEATH(static_cast<void>(Pool.getArrayType(Data, 1)), "MetaType");
    EXPECT_DEATH(static_cast<void>(Pool.getReferenceType(Data)), "MetaType");
    EXPECT_DEATH(static_cast<void>(Pool.getFunctionType(Data)), "MetaType");
  }

  // A valid return type does not allow a runtime value to be smuggled into a function parameter type.
  TEST(IRTypePoolDeathTest, RejectsNonTypeFunctionParameterTypes)
  {
    TypePool Pool;
    const Parameter Data(Pool.getBoolType());
    const Parameter Parameters[] = {
        Parameter(std::cref(Data)),
    };
    EXPECT_DEATH(static_cast<void>(Pool.getFunctionType(Pool.getVoidType(), Parameters)), "MetaType");
  }

  // Each supported width and sign has one scalar identity within a pool, with independent identities across pools.
  TEST(IRTypePoolTest, UniquifiesScalarTypesByCompleteParameters)
  {
    TypePool Pool;
    TypePool Other;
    const IntegerType::Width Widths[] = {
        IntegerType::Width::Bits8,
        IntegerType::Width::Bits16,
        IntegerType::Width::Bits32,
        IntegerType::Width::Bits64,
    };
    for (const auto Width : Widths)
    {
      const auto &Signed = Pool.getIntegerType(Width, IntegerType::Signedness::Signed);
      const auto &Unsigned = Pool.getIntegerType(Width, IntegerType::Signedness::Unsigned);
      EXPECT_EQ(&Signed, &Pool.getIntegerType(Width, IntegerType::Signedness::Signed));
      EXPECT_EQ(&Unsigned, &Pool.getIntegerType(Width, IntegerType::Signedness::Unsigned));
      EXPECT_NE(&Signed, &Unsigned);
      EXPECT_EQ(Signed.bitWidth(), static_cast<std::uint32_t>(Width));
      EXPECT_EQ(Unsigned.bitWidth(), static_cast<std::uint32_t>(Width));
      EXPECT_TRUE(Signed.isSigned());
      EXPECT_FALSE(Unsigned.isSigned());
      EXPECT_NE(&Signed, &Other.getIntegerType(Width, IntegerType::Signedness::Signed));
    }
    const auto &Float32 = Pool.getFloatType(FloatType::Width::Bits32);
    const auto &Float64 = Pool.getFloatType(FloatType::Width::Bits64);
    EXPECT_EQ(&Float32, &Pool.getFloatType(FloatType::Width::Bits32));
    EXPECT_EQ(&Float64, &Pool.getFloatType(FloatType::Width::Bits64));
    EXPECT_NE(&Float32, &Float64);
    EXPECT_EQ(Float32.bitWidth(), 32U);
    EXPECT_EQ(Float64.bitWidth(), 64U);
    EXPECT_EQ(&Pool.getBoolType(), &Pool.getBoolType());
    EXPECT_EQ(&Pool.getVoidType(), &Pool.getVoidType());
    EXPECT_NE(&Pool.getBoolType(), &Other.getBoolType());
    EXPECT_NE(&Pool.getVoidType(), &Other.getVoidType());
  }

  // Composite identities include target identity and array count, including nested references and zero or maximum lengths.
  TEST(IRTypePoolTest, UniquifiesCompositeTypesByBorrowedIdentityAndParameters)
  {
    const IntegerType External(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    TypePool Pool;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const auto &Array = Pool.getArrayType(Integer, 4);
    const auto &Reference = Pool.getReferenceType(Integer);
    EXPECT_EQ(&Array, &Pool.getArrayType(Integer, 4));
    EXPECT_EQ(&Array.elementType(), &Integer);
    EXPECT_EQ(Array.elementCount(), 4U);
    EXPECT_NE(&Array, &Pool.getArrayType(Integer, 5));
    EXPECT_NE(&Array, &Pool.getArrayType(External, 4));
    EXPECT_EQ(Pool.getArrayType(Integer, 0).elementCount(), 0U);
    EXPECT_EQ(Pool.getArrayType(Integer, std::numeric_limits<std::uint64_t>::max()).elementCount(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(&Reference, &Pool.getReferenceType(Integer));
    EXPECT_EQ(&Reference.referencedType(), &Integer);
    EXPECT_NE(&Reference, &Pool.getReferenceType(External));
    const auto &Nested = Pool.getReferenceType(Reference);
    EXPECT_EQ(&Nested.referencedType(), &Reference);
    EXPECT_EQ(&Pool.getArrayType(Reference, 4).elementType(), &Reference);
    EXPECT_EQ(&Pool.getReferenceType(Array).referencedType(), &Array);
  }

  // Inserting many composite types preserves pointers previously returned for both scalar and composite types.
  TEST(IRTypePoolTest, PreservesReturnedTypesWhileThePoolGrows)
  {
    TypePool Pool;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const auto &Array = Pool.getArrayType(Integer, 1);
    const auto &Reference = Pool.getReferenceType(Array);
    for (std::uint64_t Count = 2; Count < 512; ++Count)
    {
      const auto &Next = Pool.getArrayType(Integer, Count);
      Pool.getReferenceType(Next);
    }
    EXPECT_EQ(&Integer, &Pool.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned));
    EXPECT_EQ(&Array, &Pool.getArrayType(Integer, 1));
    EXPECT_EQ(&Reference, &Pool.getReferenceType(Array));
    EXPECT_EQ(&Reference.referencedType(), &Array);
    EXPECT_EQ(Array.elementCount(), 1U);
  }

  // Array hash sets find exact keys and preserve immovable node addresses after explicit rehashing.
  TEST(IRTypePoolTest, FindsArrayHashSetEntriesWithTransparentKeysAfterRehash)
  {
    const IntegerType First(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType Second(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Missing;
    std::unordered_set<ArrayType, core::Hash<ArrayType>, std::equal_to<>> Arrays;
    const auto [EmptyIterator, InsertedEmpty] = Arrays.emplace(First, 0);
    const auto *Empty = &*EmptyIterator;
    const auto [MaximumIterator, InsertedMaximum] = Arrays.emplace(First, std::numeric_limits<std::uint64_t>::max());
    const auto *Maximum = &*MaximumIterator;
    const auto [OtherIterator, InsertedOther] = Arrays.emplace(Second, 0);
    const auto *Other = &*OtherIterator;
    EXPECT_TRUE(InsertedEmpty);
    EXPECT_TRUE(InsertedMaximum);
    EXPECT_TRUE(InsertedOther);
    EXPECT_EQ(Arrays.size(), 3U);
    EXPECT_FALSE(Arrays.emplace(First, 0).second);
    const auto PreviousBucketCount = Arrays.bucket_count();
    Arrays.rehash(PreviousBucketCount * 2 + 1);
    EXPECT_GT(Arrays.bucket_count(), PreviousBucketCount);
    const auto FoundEmpty = Arrays.find(ArrayType::Key{&First, 0});
    const auto FoundMaximum = Arrays.find(ArrayType::Key{&First, std::numeric_limits<std::uint64_t>::max()});
    const auto FoundOther = Arrays.find(ArrayType::Key{&Second, 0});
    ASSERT_NE(FoundEmpty, Arrays.end());
    ASSERT_NE(FoundMaximum, Arrays.end());
    ASSERT_NE(FoundOther, Arrays.end());
    EXPECT_EQ(&*FoundEmpty, Empty);
    EXPECT_EQ(&*FoundMaximum, Maximum);
    EXPECT_EQ(&*FoundOther, Other);
    EXPECT_EQ(Arrays.find(ArrayType::Key{&First, 1}), Arrays.end());
    EXPECT_EQ(Arrays.find(ArrayType::Key{&Missing, 0}), Arrays.end());
    const core::Hash<ArrayType> Hash;
    const std::equal_to<> Equal;
    EXPECT_EQ(Hash(*Empty), Hash(ArrayType::Key{&First, 0}));
    EXPECT_EQ(Hash(*Maximum), Hash(ArrayType::Key{&First, std::numeric_limits<std::uint64_t>::max()}));
    EXPECT_EQ(Hash(*Other), Hash(ArrayType::Key{&Second, 0}));
    EXPECT_TRUE(Equal(*Empty, *Empty));
    EXPECT_TRUE((*Empty == ArrayType::Key{&First, 0}));
    EXPECT_TRUE((ArrayType::Key{&First, 0} == *Empty));
    EXPECT_FALSE(Equal(*Empty, *Maximum));
    EXPECT_FALSE(Equal(*Empty, ArrayType::Key{&First, 1}));
    EXPECT_FALSE(Equal(ArrayType::Key{&First, 1}, *Empty));
    EXPECT_FALSE(Equal(*Empty, *Other));
    EXPECT_FALSE(Equal(*Empty, ArrayType::Key{&Second, 0}));
    EXPECT_FALSE(Equal(ArrayType::Key{&Second, 0}, *Empty));
  }

  // Reference hash sets retain nested target identity and node addresses across explicit rehashing.
  TEST(IRTypePoolTest, FindsReferenceHashSetEntriesWithTransparentTargetPointersAfterRehash)
  {
    const IntegerType First(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType Second(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Missing;
    std::unordered_set<ReferenceType, core::Hash<ReferenceType>, std::equal_to<>> References;
    const auto [DirectIterator, InsertedDirect] = References.emplace(First);
    const auto *Direct = &*DirectIterator;
    const auto [OtherIterator, InsertedOther] = References.emplace(Second);
    const auto *Other = &*OtherIterator;
    const auto [NestedIterator, InsertedNested] = References.emplace(static_cast<const Type &>(*Direct));
    const auto *Nested = &*NestedIterator;
    EXPECT_TRUE(InsertedDirect);
    EXPECT_TRUE(InsertedOther);
    EXPECT_TRUE(InsertedNested);
    EXPECT_EQ(References.size(), 3U);
    EXPECT_FALSE(References.emplace(First).second);
    const auto PreviousBucketCount = References.bucket_count();
    References.rehash(PreviousBucketCount * 2 + 1);
    EXPECT_GT(References.bucket_count(), PreviousBucketCount);
    const auto FoundDirect = References.find(&First);
    const auto FoundOther = References.find(static_cast<const Type *>(&Second));
    const auto FoundNested = References.find(static_cast<const Type *>(Direct));
    ASSERT_NE(FoundDirect, References.end());
    ASSERT_NE(FoundOther, References.end());
    ASSERT_NE(FoundNested, References.end());
    EXPECT_EQ(&*FoundDirect, Direct);
    EXPECT_EQ(&*FoundOther, Other);
    EXPECT_EQ(&*FoundNested, Nested);
    EXPECT_EQ(References.find(*Direct), FoundDirect);
    EXPECT_EQ(References.find(static_cast<const Type *>(&Missing)), References.end());
    EXPECT_EQ(&Nested->referencedType(), Direct);
    const core::Hash<ReferenceType> Hash;
    const std::equal_to<> Equal;
    EXPECT_EQ(Hash(*Direct), Hash(&First));
    EXPECT_EQ(Hash(*Other), Hash(static_cast<const Type *>(&Second)));
    EXPECT_EQ(Hash(*Nested), Hash(static_cast<const Type *>(Direct)));
    EXPECT_TRUE(Equal(*Direct, *Direct));
    EXPECT_TRUE(*Direct == static_cast<const Type *>(&First));
    EXPECT_TRUE(static_cast<const Type *>(&First) == *Direct);
    EXPECT_FALSE(Equal(*Direct, *Other));
    EXPECT_FALSE(Equal(*Direct, static_cast<const Type *>(&Second)));
    EXPECT_FALSE(Equal(static_cast<const Type *>(&Second), *Direct));
    EXPECT_FALSE(Equal(*Direct, *Nested));
    EXPECT_FALSE(Equal(*Direct, static_cast<const Type *>(Direct)));
    EXPECT_FALSE(Equal(static_cast<const Type *>(&First), *Nested));
  }

  // Equality distinguishes target identity and counts even when every entry shares a hash bucket.
  TEST(IRTypePoolTest, DistinguishesCompositeTypesWhenHashesCollide)
  {
    const IntegerType First(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType Second(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    std::unordered_set<ArrayType, CollidingTypeHash, std::equal_to<>> Arrays;
    const auto *FirstArray = &*Arrays.emplace(First, 0).first;
    const auto *LongerArray = &*Arrays.emplace(First, 1).first;
    const auto *OtherArray = &*Arrays.emplace(Second, 0).first;
    EXPECT_EQ(Arrays.size(), 3U);
    const auto FoundFirstArray = Arrays.find(ArrayType::Key{&First, 0});
    const auto FoundLongerArray = Arrays.find(ArrayType::Key{&First, 1});
    const auto FoundOtherArray = Arrays.find(ArrayType::Key{&Second, 0});
    ASSERT_NE(FoundFirstArray, Arrays.end());
    ASSERT_NE(FoundLongerArray, Arrays.end());
    ASSERT_NE(FoundOtherArray, Arrays.end());
    EXPECT_EQ(&*FoundFirstArray, FirstArray);
    EXPECT_EQ(&*FoundLongerArray, LongerArray);
    EXPECT_EQ(&*FoundOtherArray, OtherArray);
    EXPECT_EQ(Arrays.find(ArrayType::Key{&Second, 1}), Arrays.end());
    EXPECT_FALSE(Arrays.emplace(First, 0).second);
    std::unordered_set<ReferenceType, CollidingTypeHash, std::equal_to<>> References;
    const auto *Direct = &*References.emplace(First).first;
    const auto *Other = &*References.emplace(Second).first;
    const auto *Nested = &*References.emplace(static_cast<const Type &>(*Direct)).first;
    EXPECT_EQ(References.size(), 3U);
    const auto FoundDirect = References.find(static_cast<const Type *>(&First));
    const auto FoundOther = References.find(static_cast<const Type *>(&Second));
    const auto FoundNested = References.find(static_cast<const Type *>(Direct));
    ASSERT_NE(FoundDirect, References.end());
    ASSERT_NE(FoundOther, References.end());
    ASSERT_NE(FoundNested, References.end());
    EXPECT_EQ(&*FoundDirect, Direct);
    EXPECT_EQ(&*FoundOther, Other);
    EXPECT_EQ(&*FoundNested, Nested);
    EXPECT_EQ(References.find(static_cast<const Type *>(Nested)), References.end());
    EXPECT_FALSE(References.emplace(First).second);
  }
} // namespace ink::ir::test
