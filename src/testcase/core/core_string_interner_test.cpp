#include "ink/core/string_interner.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace ink::core
{
  namespace
  {
    static_assert(!std::is_convertible<InternedStringId, InternedStringId::ValueType>::value, "InternedStringId must not implicitly convert to its storage type");
    static_assert(!std::is_convertible<InternedStringId::ValueType, InternedStringId>::value, "InternedStringId must not implicitly construct from its storage type");

    // Verifies that the default interned string ID is invalid and non-sentinel values are explicit valid IDs.
    TEST(InternedStringIdTest, DistinguishesInvalidAndValidValues)
    {
      constexpr InternedStringId Invalid;
      constexpr InternedStringId First = InternedStringId::fromValue(0);
      constexpr InternedStringId Later = InternedStringId::fromValue(9);

      EXPECT_FALSE(Invalid.isValid());
      EXPECT_EQ(Invalid.value(), InternedStringId::InvalidValue);
      EXPECT_TRUE(First.isValid());
      EXPECT_TRUE(static_cast<bool>(First));
      EXPECT_LT(First, Later);
      EXPECT_EQ(std::hash<InternedStringId>{}(Later), std::hash<InternedStringId>{}(InternedStringId::fromValue(9)));
    }

    // Verifies every InternedStringId relational operator agrees at equal, lower, higher, and invalid-sentinel boundaries.
    TEST(InternedStringIdTest, ImplementsCompleteOrderingSemantics)
    {
      constexpr InternedStringId Lower = InternedStringId::fromValue(2);
      constexpr InternedStringId Equal = InternedStringId::fromValue(2);
      constexpr InternedStringId Higher = InternedStringId::fromValue(3);
      constexpr InternedStringId Invalid;

      EXPECT_EQ(Lower, Equal);
      EXPECT_FALSE(Lower != Equal);
      EXPECT_LT(Lower, Higher);
      EXPECT_LE(Lower, Equal);
      EXPECT_LE(Lower, Higher);
      EXPECT_GT(Higher, Lower);
      EXPECT_GE(Higher, Lower);
      EXPECT_GE(Equal, Lower);
      EXPECT_LT(Higher, Invalid);
    }

    // Verifies a string interner reports its exact empty-to-nonempty state transition even when the first interned value is empty.
    TEST(StringInternerTest, ReportsEmptyStateTransition)
    {
      StringInterner Strings;

      EXPECT_TRUE(Strings.empty());
      EXPECT_EQ(Strings.size(), 0U);
      const InternedStringId Empty = Strings.intern("");
      EXPECT_FALSE(Strings.empty());
      EXPECT_EQ(Strings.size(), 1U);
      EXPECT_TRUE(Strings.contains(Empty));
      EXPECT_TRUE(Strings.string(Empty).empty());
    }

    // Verifies that equal strings, including the empty string, reuse a single insertion-order identity.
    TEST(StringInternerTest, ReusesIdsForEqualStrings)
    {
      StringInterner Strings;
      const InternedStringId Empty = Strings.intern("");
      const InternedStringId First = Strings.intern("identifier");
      const InternedStringId Equal = Strings.intern(std::string("identifier"));
      const InternedStringId Second = Strings.intern("other");

      EXPECT_EQ(Empty.value(), 0U);
      EXPECT_EQ(First.value(), 1U);
      EXPECT_EQ(Equal, First);
      EXPECT_EQ(Second.value(), 2U);
      EXPECT_EQ(Strings.size(), 3U);
      EXPECT_EQ(Strings.string(Empty), "");
      EXPECT_EQ(Strings.string(First), "identifier");
    }

    // Verifies that interning compares and preserves complete byte sequences rather than stopping at an embedded null byte.
    TEST(StringInternerTest, PreservesEmbeddedNullBytes)
    {
      StringInterner Strings;
      const std::string FirstValue("a\0b", 3);
      const std::string EqualValue("a\0b", 3);
      const InternedStringId First = Strings.intern(FirstValue);
      const InternedStringId Equal = Strings.intern(EqualValue);
      const std::string_view Resolved = Strings.string(First);

      EXPECT_EQ(First, Equal);
      ASSERT_EQ(Resolved.size(), 3U);
      EXPECT_EQ(Resolved[0], 'a');
      EXPECT_EQ(Resolved[1], '\0');
      EXPECT_EQ(Resolved[2], 'b');
    }

    // Verifies that the interner owns input bytes and keeps resolved string views stable across later insertions.
    TEST(StringInternerTest, OwnsStringsAndKeepsViewsStable)
    {
      StringInterner Strings;
      std::string Input = "owned";
      const InternedStringId Id = Strings.intern(Input);
      const std::string_view View = Strings.string(Id);
      const char *Data = View.data();
      Input = "changed";
      for (std::uint32_t Index = 0; Index < 128; ++Index)
      {
        Strings.intern("generated-" + std::to_string(Index));
      }

      EXPECT_EQ(Strings.string(Id), "owned");
      EXPECT_EQ(Strings.string(Id).data(), Data);
      EXPECT_EQ(View, "owned");
    }

    // Verifies that invalid and out-of-range IDs are rejected instead of resolving to an unrelated string.
    TEST(StringInternerTest, RejectsIdsOutsideTheInterner)
    {
      StringInterner Strings;
      const InternedStringId Existing = Strings.intern("only");
      const InternedStringId OutOfRange = InternedStringId::fromValue(1);

      EXPECT_TRUE(Strings.contains(Existing));
      EXPECT_FALSE(Strings.contains(InternedStringId{}));
      EXPECT_FALSE(Strings.contains(OutOfRange));
      EXPECT_THROW(Strings.string(InternedStringId{}), std::out_of_range);
      EXPECT_THROW(Strings.string(OutOfRange), std::out_of_range);
    }
  } // namespace
} // namespace ink::core
