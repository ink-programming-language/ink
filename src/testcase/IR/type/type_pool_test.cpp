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
  static_assert(!std::is_copy_constructible_v<PointerType>);
  static_assert(!std::is_move_constructible_v<PointerType>);
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
  concept PoolAcceptsTemporaryPointerTarget = requires(TypePool &Pool)
  {
    Pool.getPointerType(std::declval<T &&>());
  };

  static_assert(!PoolAcceptsTemporaryArrayElement<IntegerType>);
  static_assert(!PoolAcceptsTemporaryArrayElement<const IntegerType>);
  static_assert(!PoolAcceptsTemporaryPointerTarget<IntegerType>);
  static_assert(!PoolAcceptsTemporaryPointerTarget<const IntegerType>);
  static_assert(!PoolAcceptsTemporaryArrayElement<Parameter>);
  static_assert(!PoolAcceptsTemporaryArrayElement<const Parameter>);
  static_assert(!PoolAcceptsTemporaryPointerTarget<Parameter>);
  static_assert(!PoolAcceptsTemporaryPointerTarget<const Parameter>);
  static_assert(std::is_same_v<decltype(std::declval<const ArrayType &>().elementType()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const PointerType &>().pointeeType()), const Value &>);

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
    const PointerType &FirstPointer = Pool.getPointerType(FirstStruct);
    FirstObject.addField(FieldName, FirstPointer);
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
    EXPECT_EQ(&FirstObject.fields()[0].type(), &FirstPointer);
    EXPECT_EQ(&FirstPointer.pointeeType(), &FirstStruct);
    EXPECT_EQ(&Pool.getPointerType(FirstStruct), &FirstPointer);
    EXPECT_TRUE(SecondStruct.fields().empty());
    EXPECT_TRUE(SecondObject.fields().empty());
    EXPECT_EQ(FirstStruct.getSemantics(), TypeSemantics::Value);
    EXPECT_EQ(FirstObject.getSemantics(), TypeSemantics::Reference);
  }

  // Mutable record declarations remain identity keys for cached pointers, arrays, and signatures through edits and cache growth.
  TEST(IRTypePoolTest, KeepsCompositeCachesStableWhenOwnedRecordDeclarationsChange)
  {
    TypePool Pool;
    core::NamePool &Names = core::NamePool::instance();
    StructType &Struct = Pool.createStructType();
    ObjectType &Object = Pool.createObjectType();
    const PointerType &StructPointer = Pool.getPointerType(Struct);
    const PointerType &ObjectPointer = Pool.getPointerType(Object);
    EXPECT_EQ(StructPointer.getSemantics(), TypeSemantics::Value);
    EXPECT_EQ(ObjectPointer.getSemantics(), TypeSemantics::Value);
    const ArrayType &StructArray = Pool.getArrayType(Struct, 4);
    const ArrayType &ObjectArray = Pool.getArrayType(Object, 4);
    const std::array Parameters{Parameter(Object, Names.intern("owner"))};
    const FunctionType &Signature = Pool.getFunctionType(Struct, Parameters);
    const auto PointerHash = StructPointer.hash();
    const auto ArrayHash = StructArray.hash();
    const auto SignatureHash = Signature.hash();
    Struct.addBase(Pool.createStructType());
    Struct.addField(Names.intern("owner"), ObjectPointer);
    Object.addField(Names.intern("payload"), Struct);
    Object.addField(Names.intern("next"), ObjectPointer);
    Method &OwnedMethod = Object.addMethod(Signature, Names.intern("payload"));
    Function *MethodFunction = &OwnedMethod.function();
    for (std::uint64_t Count = 1; Count <= 128; ++Count)
    {
      StructType &NextStruct = Pool.createStructType();
      ObjectType &NextObject = Pool.createObjectType();
      Pool.getPointerType(NextStruct);
      Pool.getPointerType(NextObject);
      Pool.getArrayType(NextStruct, Count);
      Pool.getArrayType(NextObject, Count);
      const std::array NextParameters{Parameter(NextObject)};
      Pool.getFunctionType(NextStruct, NextParameters);
    }
    EXPECT_EQ(&Pool.getPointerType(Struct), &StructPointer);
    EXPECT_EQ(&Pool.getPointerType(Object), &ObjectPointer);
    EXPECT_EQ(&Pool.getArrayType(Struct, 4), &StructArray);
    EXPECT_EQ(&Pool.getArrayType(Object, 4), &ObjectArray);
    EXPECT_EQ(&Pool.getFunctionType(Struct, Parameters), &Signature);
    EXPECT_EQ(StructPointer.hash(), PointerHash);
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
    const auto *PointerT = &Pool.getPointerType(T);
    const auto *PointerU = &Pool.getPointerType(U);
    const auto *Nested = &Pool.getPointerType(*ArrayT);
    for (std::uint64_t Count = 2; Count < 512; ++Count)
    {
      const auto &Array = Pool.getArrayType(T, Count);
      Pool.getPointerType(Array);
    }
    EXPECT_EQ(ArrayT, &Pool.getArrayType(T, 1));
    EXPECT_EQ(ArrayU, &Pool.getArrayType(U, 1));
    EXPECT_EQ(PointerT, &Pool.getPointerType(T));
    EXPECT_EQ(PointerU, &Pool.getPointerType(U));
    EXPECT_NE(ArrayT, ArrayU);
    EXPECT_NE(PointerT, PointerU);
    EXPECT_EQ(&ArrayT->elementType(), &T);
    EXPECT_EQ(&ArrayU->elementType(), &U);
    EXPECT_EQ(&PointerT->pointeeType(), &T);
    EXPECT_EQ(&PointerU->pointeeType(), &U);
    EXPECT_EQ(Nested, &Pool.getPointerType(*ArrayT));
    EXPECT_EQ(&Nested->pointeeType(), ArrayT);
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
    std::unordered_set<PointerType, core::Hash<PointerType>, std::equal_to<>> Pointers;
    const auto *PointerT = &*Pointers.emplace(T).first;
    const auto *PointerU = &*Pointers.emplace(U).first;
    Pointers.rehash(Pointers.bucket_count() * 2 + 1);
    const auto FoundPointerT = Pointers.find(&T);
    const auto FoundPointerU = Pointers.find(static_cast<const Value *>(&U));
    ASSERT_NE(FoundPointerT, Pointers.end());
    ASSERT_NE(FoundPointerU, Pointers.end());
    EXPECT_EQ(&*FoundPointerT, PointerT);
    EXPECT_EQ(&*FoundPointerU, PointerU);
    const core::Hash<PointerType> PointerHash;
    EXPECT_EQ(PointerHash(*PointerT), PointerHash(&T));
    EXPECT_EQ(PointerHash(*PointerU), PointerHash(static_cast<const Value *>(&U)));
    EXPECT_TRUE(*PointerT == &T);
    EXPECT_TRUE(&T == *PointerT);
    EXPECT_FALSE(*PointerT == &U);
    EXPECT_FALSE(*PointerU == &T);
  }

  // A runtime data parameter is not a type value and cannot enter any composite type position.
  TEST(IRTypePoolDeathTest, RejectsNonTypeCompositeOperands)
  {
    TypePool Pool;
    const Parameter Data(Pool.getBoolType());
    EXPECT_DEATH(static_cast<void>(Pool.getArrayType(Data, 1)), "MetaType");
    EXPECT_DEATH(static_cast<void>(Pool.getPointerType(Data)), "MetaType");
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

  // Composite identities include target identity and array count, including nested pointers and zero or maximum lengths.
  TEST(IRTypePoolTest, UniquifiesCompositeTypesByBorrowedIdentityAndParameters)
  {
    const IntegerType External(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    TypePool Pool;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const auto &Array = Pool.getArrayType(Integer, 4);
    const auto &Pointer = Pool.getPointerType(Integer);
    EXPECT_EQ(&Array, &Pool.getArrayType(Integer, 4));
    EXPECT_EQ(&Array.elementType(), &Integer);
    EXPECT_EQ(Array.elementCount(), 4U);
    EXPECT_NE(&Array, &Pool.getArrayType(Integer, 5));
    EXPECT_NE(&Array, &Pool.getArrayType(External, 4));
    EXPECT_EQ(Pool.getArrayType(Integer, 0).elementCount(), 0U);
    EXPECT_EQ(Pool.getArrayType(Integer, std::numeric_limits<std::uint64_t>::max()).elementCount(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(&Pointer, &Pool.getPointerType(Integer));
    EXPECT_EQ(&Pointer.pointeeType(), &Integer);
    EXPECT_NE(&Pointer, &Pool.getPointerType(External));
    const auto &Nested = Pool.getPointerType(Pointer);
    EXPECT_EQ(&Nested.pointeeType(), &Pointer);
    EXPECT_EQ(&Pool.getArrayType(Pointer, 4).elementType(), &Pointer);
    EXPECT_EQ(&Pool.getPointerType(Array).pointeeType(), &Array);
  }

  // Inserting many composite types preserves pointers previously returned for both scalar and composite types.
  TEST(IRTypePoolTest, PreservesReturnedTypesWhileThePoolGrows)
  {
    TypePool Pool;
    const auto &Integer = Pool.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const auto &Array = Pool.getArrayType(Integer, 1);
    const auto &Pointer = Pool.getPointerType(Array);
    for (std::uint64_t Count = 2; Count < 512; ++Count)
    {
      const auto &Next = Pool.getArrayType(Integer, Count);
      Pool.getPointerType(Next);
    }
    EXPECT_EQ(&Integer, &Pool.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned));
    EXPECT_EQ(&Array, &Pool.getArrayType(Integer, 1));
    EXPECT_EQ(&Pointer, &Pool.getPointerType(Array));
    EXPECT_EQ(&Pointer.pointeeType(), &Array);
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

  // Pointer hash sets retain nested target identity and node addresses across explicit rehashing.
  TEST(IRTypePoolTest, FindsPointerHashSetEntriesWithTransparentTargetPointersAfterRehash)
  {
    const IntegerType First(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType Second(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Missing;
    std::unordered_set<PointerType, core::Hash<PointerType>, std::equal_to<>> Pointers;
    const auto [DirectIterator, InsertedDirect] = Pointers.emplace(First);
    const auto *Direct = &*DirectIterator;
    const auto [OtherIterator, InsertedOther] = Pointers.emplace(Second);
    const auto *Other = &*OtherIterator;
    const auto [NestedIterator, InsertedNested] = Pointers.emplace(static_cast<const Type &>(*Direct));
    const auto *Nested = &*NestedIterator;
    EXPECT_TRUE(InsertedDirect);
    EXPECT_TRUE(InsertedOther);
    EXPECT_TRUE(InsertedNested);
    EXPECT_EQ(Pointers.size(), 3U);
    EXPECT_FALSE(Pointers.emplace(First).second);
    const auto PreviousBucketCount = Pointers.bucket_count();
    Pointers.rehash(PreviousBucketCount * 2 + 1);
    EXPECT_GT(Pointers.bucket_count(), PreviousBucketCount);
    const auto FoundDirect = Pointers.find(&First);
    const auto FoundOther = Pointers.find(static_cast<const Type *>(&Second));
    const auto FoundNested = Pointers.find(static_cast<const Type *>(Direct));
    ASSERT_NE(FoundDirect, Pointers.end());
    ASSERT_NE(FoundOther, Pointers.end());
    ASSERT_NE(FoundNested, Pointers.end());
    EXPECT_EQ(&*FoundDirect, Direct);
    EXPECT_EQ(&*FoundOther, Other);
    EXPECT_EQ(&*FoundNested, Nested);
    EXPECT_EQ(Pointers.find(*Direct), FoundDirect);
    EXPECT_EQ(Pointers.find(static_cast<const Type *>(&Missing)), Pointers.end());
    EXPECT_EQ(&Nested->pointeeType(), Direct);
    const core::Hash<PointerType> Hash;
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
    std::unordered_set<PointerType, CollidingTypeHash, std::equal_to<>> Pointers;
    const auto *Direct = &*Pointers.emplace(First).first;
    const auto *Other = &*Pointers.emplace(Second).first;
    const auto *Nested = &*Pointers.emplace(static_cast<const Type &>(*Direct)).first;
    EXPECT_EQ(Pointers.size(), 3U);
    const auto FoundDirect = Pointers.find(static_cast<const Type *>(&First));
    const auto FoundOther = Pointers.find(static_cast<const Type *>(&Second));
    const auto FoundNested = Pointers.find(static_cast<const Type *>(Direct));
    ASSERT_NE(FoundDirect, Pointers.end());
    ASSERT_NE(FoundOther, Pointers.end());
    ASSERT_NE(FoundNested, Pointers.end());
    EXPECT_EQ(&*FoundDirect, Direct);
    EXPECT_EQ(&*FoundOther, Other);
    EXPECT_EQ(&*FoundNested, Nested);
    EXPECT_EQ(Pointers.find(static_cast<const Type *>(Nested)), Pointers.end());
    EXPECT_FALSE(Pointers.emplace(First).second);
  }
} // namespace ink::ir::test
