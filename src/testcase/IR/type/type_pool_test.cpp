#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

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
