#include "ink/semantic/model/name/name_pool.h"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <unordered_map>

namespace ink::semantic::test
{
  static_assert(sizeof(Name) == sizeof(std::uint32_t));
  static_assert(std::is_trivially_copyable_v<Name>);
  static_assert(!std::is_copy_constructible_v<NamePool>);
  static_assert(!std::is_move_constructible_v<NamePool>);

  // Invalid and empty names never occupy a slot, and lookup does not intern misses.
  TEST(SemanticNamePoolTest, InvalidNamesAndLookupMissesDoNotAllocate)
  {
    NamePool Pool;
    EXPECT_FALSE(Name{}.valid());
    EXPECT_FALSE(Pool.intern({}).valid());
    EXPECT_FALSE(Pool.find({}).valid());
    EXPECT_FALSE(Pool.find("Missing").valid());
    EXPECT_FALSE(Pool.contains(Name{}));
    EXPECT_TRUE(Pool.text(Name{}).empty());
    EXPECT_EQ(Pool.size(), 0U);
  }

  // Equal spellings reuse an index regardless of the lifetime or address of input.
  TEST(SemanticNamePoolTest, InternsOneOwnedSpellingAndSupportsHashKeys)
  {
    NamePool Pool;
    std::string Input = "Variable";
    const Name First = Pool.intern(Input);
    const Name Second = Pool.intern(std::string("Variable"));
    Input.assign("Changed");
    EXPECT_TRUE(First.valid());
    EXPECT_EQ(First.index(), 0U);
    EXPECT_EQ(First, Second);
    EXPECT_EQ(Pool.find("Variable"), First);
    EXPECT_EQ(Pool.text(First), "Variable");
    EXPECT_EQ(Pool.size(), 1U);
    std::unordered_map<Name, int> Bindings;
    Bindings.emplace(First, 7);
    EXPECT_EQ(Bindings.find(Second)->second, 7);
    EXPECT_NE(Pool.intern("variable"), First);
    EXPECT_EQ(Pool.size(), 2U);
  }

  // Rehashing both indices preserves existing handles and the actual text addresses.
  TEST(SemanticNamePoolTest, TextViewsSurvivePoolGrowth)
  {
    NamePool Pool;
    const Name First = Pool.intern("X");
    const std::string_view Original = Pool.text(First);
    for (std::size_t Index = 0; Index < 8192; ++Index)
    {
      ASSERT_TRUE(Pool.intern("Name" + std::to_string(Index)).valid());
    }
    EXPECT_EQ(Pool.find("X"), First);
    EXPECT_EQ(Pool.text(First).data(), Original.data());
    EXPECT_EQ(Original, "X");
    EXPECT_EQ(Pool.size(), 8193U);
    EXPECT_EQ(Pool.intern(Original), First);
  }

  // The pool compares complete byte sequences; normalization belongs to tokenization.
  TEST(SemanticNamePoolTest, PreservesUtf8AndLengthDelimitedSpellings)
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
    EXPECT_EQ(Pool.text(Bytes), std::string_view(WithNull));
    EXPECT_NE(Bytes, Pool.intern("A"));
  }

  // A compact handle has pool-local meaning, and out-of-range lookups fail safely.
  TEST(SemanticNamePoolTest, IndicesAreLocalToTheirPool)
  {
    NamePool First;
    NamePool Second;
    const Name A = First.intern("A");
    const Name B = Second.intern("B");
    EXPECT_EQ(A.index(), B.index());
    EXPECT_EQ(First.text(A), "A");
    EXPECT_EQ(Second.text(B), "B");
    const Name OutOfRange = First.intern("C");
    EXPECT_FALSE(Second.contains(OutOfRange));
    EXPECT_TRUE(Second.text(OutOfRange).empty());
  }
} // namespace ink::semantic::test
