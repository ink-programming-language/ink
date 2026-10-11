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
  static_assert(!NAME_None.valid());
  static_assert(NAME_None.index() == Name::InvalidIndex);
  static_assert(NAME_None == Name{});
  static_assert(!std::is_default_constructible_v<NamePool>);
  static_assert(!std::is_destructible_v<NamePool>);
  static_assert(!std::is_copy_constructible_v<NamePool>);
  static_assert(!std::is_copy_assignable_v<NamePool>);
  static_assert(!std::is_move_constructible_v<NamePool>);
  static_assert(!std::is_move_assignable_v<NamePool>);

  // Invalid and empty names never occupy a slot, and lookup does not intern misses.
  TEST(CoreNamePoolTest, InvalidNamesAndLookupMissesDoNotAllocate)
  {
    NamePool &Pool = NamePool::instance();
    const std::size_t Before = Pool.size();
    const std::string Prefix = "CoreNamePoolTest.Invalid." + std::to_string(Before);
    EXPECT_FALSE(Pool.intern({}).valid());
    EXPECT_FALSE(Pool.find({}).valid());
    EXPECT_FALSE(Pool.find(Prefix + ".Missing").valid());
    EXPECT_FALSE(Pool.contains(Name{}));
    EXPECT_TRUE(Pool.text(Name{}).empty());
    EXPECT_EQ(Pool.size(), Before);
    const Name First = Pool.intern(Prefix + ".First");
    EXPECT_NE(First, Name{});
    EXPECT_FALSE(Pool.find(Prefix + ".Missing").valid());
    EXPECT_FALSE(Pool.intern({}).valid());
    EXPECT_EQ(Pool.size(), Before + 1);
    EXPECT_EQ(First.index(), Before);
  }

  // Equal spellings reuse an index regardless of input ownership and remain valid hash keys.
  TEST(CoreNamePoolTest, InternsOneOwnedSpellingAndSupportsHashKeys)
  {
    NamePool &Pool = NamePool::instance();
    const std::size_t Before = Pool.size();
    const std::string Prefix = "CoreNamePoolTest.Owned." + std::to_string(Before) + ".";
    const std::string Text = Prefix + "Variable";
    std::string Input = Text;
    const Name First = Pool.intern(Input);
    const Name Second = Pool.intern(std::string(Text));
    Input.assign("Changed");
    EXPECT_TRUE(First.valid());
    EXPECT_TRUE(Pool.contains(First));
    EXPECT_EQ(First.index(), Before);
    EXPECT_EQ(First, Second);
    EXPECT_EQ(std::hash<Name>{}(First), std::hash<Name>{}(Second));
    EXPECT_EQ(Pool.find(Text), First);
    EXPECT_EQ(Pool.text(First), Text);
    EXPECT_EQ(Pool.size(), Before + 1);
    std::unordered_map<Name, int> Bindings;
    Bindings.emplace(First, 7);
    const auto Found = Bindings.find(Second);
    ASSERT_NE(Found, Bindings.end());
    EXPECT_EQ(Found->second, 7);
    const Name Lowercase = Pool.intern(Prefix + "variable");
    EXPECT_NE(Lowercase, First);
    EXPECT_EQ(Lowercase.index(), Before + 1);
    EXPECT_EQ(Pool.size(), Before + 2);
  }

  // Rehashing and index growth preserve handles and text addresses for short and long spellings.
  TEST(CoreNamePoolTest, TextViewsSurvivePoolGrowth)
  {
    NamePool &Pool = NamePool::instance();
    const std::size_t Before = Pool.size();
    const std::string Prefix = "CoreNamePoolTest.Growth." + std::to_string(Before) + ".";
    const std::string ShortText = "X" + std::to_string(Before);
    const Name First = Pool.intern(ShortText);
    const std::string LongText = Prefix + std::string(256, 'L');
    const Name Long = Pool.intern(LongText);
    const std::string_view Original = Pool.text(First);
    const std::string_view OriginalLong = Pool.text(Long);
    for (std::size_t Index = 0; Index < 8192; ++Index)
    {
      ASSERT_TRUE(Pool.intern(Prefix + std::to_string(Index)).valid());
    }
    EXPECT_EQ(Pool.find(ShortText), First);
    EXPECT_EQ(Pool.find(LongText), Long);
    EXPECT_EQ(Pool.text(First).data(), Original.data());
    EXPECT_EQ(Pool.text(Long).data(), OriginalLong.data());
    EXPECT_EQ(Original, ShortText);
    EXPECT_EQ(OriginalLong, LongText);
    EXPECT_EQ(Pool.size(), Before + 8194);
    EXPECT_EQ(Pool.intern(Original), First);
    EXPECT_EQ(Pool.intern(OriginalLong), Long);
    EXPECT_EQ(Pool.size(), Before + 8194);
  }

  // Byte comparison preserves distinct Unicode normalization forms and embedded null bytes.
  TEST(CoreNamePoolTest, PreservesUtf8AndLengthDelimitedSpellings)
  {
    NamePool &Pool = NamePool::instance();
    const std::size_t Before = Pool.size();
    const std::string Prefix = "CoreNamePoolTest.Bytes." + std::to_string(Before) + ".";
    const std::string Composed = Prefix + "\xc3\xa9";
    const std::string Decomposed = Prefix + "e\xcc\x81";
    const Name Unicode = Pool.intern(Composed);
    EXPECT_EQ(Pool.intern(std::string(Composed)), Unicode);
    EXPECT_EQ(Pool.text(Unicode), Composed);
    EXPECT_NE(Pool.intern(Decomposed), Unicode);
    const std::string WithNull = Prefix + std::string("A\0B", 3);
    const Name Bytes = Pool.intern(WithNull);
    EXPECT_EQ(Pool.find(std::string(WithNull)), Bytes);
    EXPECT_EQ(Pool.intern(std::string(WithNull)), Bytes);
    EXPECT_EQ(Pool.text(Bytes), std::string_view(WithNull));
    EXPECT_EQ(Pool.text(Bytes).size(), Prefix.size() + 3);
    EXPECT_NE(Bytes, Pool.intern(Prefix + "A"));
    EXPECT_NE(Bytes, Pool.intern(Prefix + std::string("A\0C", 3)));
    EXPECT_FALSE(Pool.find(std::string_view(WithNull.data(), Prefix.size() + 2)).valid());
    EXPECT_EQ(Pool.size(), Before + 5);
  }

  // Repeated access shares one pool and resolves each handle to the same spelling.
  TEST(CoreNamePoolTest, RepeatedAccessSharesNamesAndIdentity)
  {
    NamePool &First = NamePool::instance();
    NamePool &Second = NamePool::instance();
    EXPECT_EQ(&First, &Second);
    const std::size_t Before = First.size();
    const std::string Text = "CoreNamePoolTest.Shared." + std::to_string(Before);
    const Name Value = First.intern(Text);
    EXPECT_EQ(Second.find(Text), Value);
    EXPECT_EQ(Second.intern(Text), Value);
    EXPECT_TRUE(Second.contains(Value));
    EXPECT_EQ(Second.text(Value), Text);
    EXPECT_EQ(First.size(), Before + 1);
    EXPECT_EQ(Second.size(), Before + 1);
  }

  // NAME_None is an invalid sentinel returned for empty spellings and lookup misses without allocation.
  TEST(CoreNamePoolTest, PredefinedNoneRepresentsInvalidIndex)
  {
    NamePool &Pool = NamePool::instance();
    const std::size_t Before = Pool.size();
    const std::string Missing = "CoreNamePoolTest.NoneSentinel.Missing." + std::to_string(Before);
    EXPECT_FALSE(Pool.contains(NAME_None));
    EXPECT_TRUE(Pool.text(NAME_None).empty());
    EXPECT_EQ(Pool.intern({}), NAME_None);
    EXPECT_EQ(Pool.find({}), NAME_None);
    EXPECT_EQ(Pool.find(Missing), NAME_None);
    EXPECT_EQ(NAME_None, Name{});
    EXPECT_EQ(Pool.size(), Before);
  }

  // The ordinary spelling None interns as a valid name distinct from the invalid sentinel.
  TEST(CoreNamePoolTest, NoneSpellingIsAnOrdinaryValidName)
  {
    NamePool &Pool = NamePool::instance();
    const std::size_t Before = Pool.size();
    const Name Existing = Pool.find("None");
    const Name Value = Pool.intern("None");
    EXPECT_TRUE(Value.valid());
    EXPECT_NE(Value, NAME_None);
    EXPECT_TRUE(Pool.contains(Value));
    EXPECT_EQ(Pool.text(Value), "None");
    EXPECT_EQ(Pool.find("None"), Value);
    EXPECT_EQ(Pool.intern("None"), Value);
    EXPECT_EQ(Pool.size(), Before + (Existing.valid() ? 0U : 1U));
    if (Existing.valid())
    {
      EXPECT_EQ(Value, Existing);
    }
  }
} // namespace ink::core::test
