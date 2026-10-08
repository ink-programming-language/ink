#include "ink/core/name.h"
#include "ink/core/name_pool.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace ink::core::test
{
  static_assert(sizeof(Name) == sizeof(std::uint32_t));
  static_assert(std::is_trivially_copyable_v<Name>);
  static_assert(std::is_nothrow_default_constructible_v<Name>);
  static_assert(!std::is_constructible_v<Name, Name::IndexType>);
  static_assert(!Name{}.valid());
  static_assert(Name{}.index() == Name::InvalidIndex);
  static_assert(Name{} == Name{});
  static_assert(!std::is_copy_constructible_v<NamePool>);
  static_assert(!std::is_copy_assignable_v<NamePool>);
  static_assert(!std::is_move_constructible_v<NamePool>);
  static_assert(!std::is_move_assignable_v<NamePool>);

  // Invalid and empty names never occupy a slot, and lookup does not intern misses.
  TEST(CoreNamePoolTest, InvalidNamesAndLookupMissesDoNotAllocate)
  {
    NamePool Pool;
    EXPECT_FALSE(Pool.intern({}).valid());
    EXPECT_FALSE(Pool.find({}).valid());
    EXPECT_FALSE(Pool.find("Missing").valid());
    EXPECT_FALSE(Pool.contains(Name{}));
    EXPECT_TRUE(Pool.text(Name{}).empty());
    EXPECT_EQ(Pool.size(), 0U);
    const Name First = Pool.intern("First");
    EXPECT_NE(First, Name{});
    EXPECT_FALSE(Pool.find("Missing").valid());
    EXPECT_FALSE(Pool.intern({}).valid());
    EXPECT_EQ(Pool.size(), 1U);
    EXPECT_EQ(First.index(), 0U);
  }

  // Equal spellings reuse an index regardless of input ownership and remain valid hash keys.
  TEST(CoreNamePoolTest, InternsOneOwnedSpellingAndSupportsHashKeys)
  {
    NamePool Pool;
    std::string Input = "Variable";
    const Name First = Pool.intern(Input);
    const Name Second = Pool.intern(std::string("Variable"));
    Input.assign("Changed");
    EXPECT_TRUE(First.valid());
    EXPECT_TRUE(Pool.contains(First));
    EXPECT_EQ(First.index(), 0U);
    EXPECT_EQ(First, Second);
    EXPECT_EQ(std::hash<Name>{}(First), std::hash<Name>{}(Second));
    EXPECT_EQ(Pool.find("Variable"), First);
    EXPECT_EQ(Pool.text(First), "Variable");
    EXPECT_EQ(Pool.size(), 1U);
    std::unordered_map<Name, int> Bindings;
    Bindings.emplace(First, 7);
    const auto Found = Bindings.find(Second);
    ASSERT_NE(Found, Bindings.end());
    EXPECT_EQ(Found->second, 7);
    const Name Lowercase = Pool.intern("variable");
    EXPECT_NE(Lowercase, First);
    EXPECT_EQ(Lowercase.index(), 1U);
    EXPECT_EQ(Pool.size(), 2U);
  }

  // Rehashing and index growth preserve handles and text addresses for short and long spellings.
  TEST(CoreNamePoolTest, TextViewsSurvivePoolGrowth)
  {
    NamePool Pool;
    const Name First = Pool.intern("X");
    const std::string LongText(256, 'L');
    const Name Long = Pool.intern(LongText);
    const std::string_view Original = Pool.text(First);
    const std::string_view OriginalLong = Pool.text(Long);
    for (std::size_t Index = 0; Index < 8192; ++Index)
    {
      ASSERT_TRUE(Pool.intern("Name" + std::to_string(Index)).valid());
    }
    EXPECT_EQ(Pool.find("X"), First);
    EXPECT_EQ(Pool.find(LongText), Long);
    EXPECT_EQ(Pool.text(First).data(), Original.data());
    EXPECT_EQ(Pool.text(Long).data(), OriginalLong.data());
    EXPECT_EQ(Original, "X");
    EXPECT_EQ(OriginalLong, LongText);
    EXPECT_EQ(Pool.size(), 8194U);
    EXPECT_EQ(Pool.intern(Original), First);
    EXPECT_EQ(Pool.intern(OriginalLong), Long);
    EXPECT_EQ(Pool.size(), 8194U);
  }

  // Byte comparison preserves distinct Unicode normalization forms and embedded null bytes.
  TEST(CoreNamePoolTest, PreservesUtf8AndLengthDelimitedSpellings)
  {
    NamePool Pool;
    const std::string_view Composed = "\xc3\xa9";
    const std::string_view Decomposed = "e\xcc\x81";
    const Name Unicode = Pool.intern(Composed);
    EXPECT_EQ(Pool.intern(std::string(Composed)), Unicode);
    EXPECT_EQ(Pool.text(Unicode), Composed);
    EXPECT_NE(Pool.intern(Decomposed), Unicode);
    const std::string WithNull("A\0B", 3);
    const Name Bytes = Pool.intern(WithNull);
    EXPECT_EQ(Pool.find(std::string(WithNull)), Bytes);
    EXPECT_EQ(Pool.intern(std::string(WithNull)), Bytes);
    EXPECT_EQ(Pool.text(Bytes), std::string_view(WithNull));
    EXPECT_EQ(Pool.text(Bytes).size(), 3U);
    EXPECT_NE(Bytes, Pool.intern("A"));
    EXPECT_NE(Bytes, Pool.intern(std::string("A\0C", 3)));
    EXPECT_FALSE(Pool.find(std::string_view(WithNull.data(), 2)).valid());
    EXPECT_EQ(Pool.size(), 5U);
  }

  // Handles have pool-local meaning, while indices outside the target pool fail safely.
  TEST(CoreNamePoolTest, IndicesAreLocalToTheirPool)
  {
    NamePool First;
    NamePool Second;
    const Name A = First.intern("A");
    const Name B = Second.intern("B");
    EXPECT_EQ(A.index(), B.index());
    EXPECT_EQ(First.text(A), "A");
    EXPECT_EQ(Second.text(B), "B");
    EXPECT_TRUE(Second.contains(A));
    EXPECT_EQ(Second.text(A), "B");
    const Name OutOfRange = First.intern("C");
    EXPECT_FALSE(Second.contains(OutOfRange));
    EXPECT_TRUE(Second.text(OutOfRange).empty());
    NamePool Empty;
    EXPECT_FALSE(Empty.contains(A));
    EXPECT_TRUE(Empty.text(A).empty());
  }
} // namespace ink::core::test
