#include "ink/core/source_manager.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace ink::core
{
  namespace
  {
    static_assert(!std::is_convertible<SourceFileId, SourceFileId::ValueType>::value, "SourceFileId must not implicitly convert to its storage type");
    static_assert(!std::is_convertible<SourceFileId::ValueType, SourceFileId>::value, "SourceFileId must not implicitly construct from its storage type");

    // Verifies that the default source file ID is invalid and explicitly created non-sentinel values are valid.
    TEST(SourceFileIdTest, DistinguishesInvalidAndValidValues)
    {
      constexpr SourceFileId Invalid;
      constexpr SourceFileId First = SourceFileId::fromValue(0);
      constexpr SourceFileId Later = SourceFileId::fromValue(17);
      constexpr SourceFileId Sentinel = SourceFileId::fromValue(SourceFileId::InvalidValue);

      EXPECT_FALSE(Invalid.isValid());
      EXPECT_FALSE(static_cast<bool>(Invalid));
      EXPECT_EQ(Invalid.value(), SourceFileId::InvalidValue);
      EXPECT_TRUE(First.isValid());
      EXPECT_TRUE(static_cast<bool>(First));
      EXPECT_EQ(First.value(), 0U);
      EXPECT_LT(First, Later);
      EXPECT_EQ(Invalid, Sentinel);
    }

    // Verifies that SourceFileId can be used as an associative-container key without weakening its type safety.
    TEST(SourceFileIdTest, ProvidesStableValueHashing)
    {
      const SourceFileId First = SourceFileId::fromValue(3);
      const SourceFileId Equal = SourceFileId::fromValue(3);
      const SourceFileId Different = SourceFileId::fromValue(4);

      EXPECT_EQ(std::hash<SourceFileId>{}(First), std::hash<SourceFileId>{}(Equal));
      EXPECT_NE(First, Different);
    }

    // Verifies every SourceFileId relational operator agrees at equal, lower, higher, and invalid-sentinel boundaries.
    TEST(SourceFileIdTest, ImplementsCompleteOrderingSemantics)
    {
      constexpr SourceFileId Lower = SourceFileId::fromValue(2);
      constexpr SourceFileId Equal = SourceFileId::fromValue(2);
      constexpr SourceFileId Higher = SourceFileId::fromValue(3);
      constexpr SourceFileId Invalid;

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

    // Verifies a source manager reports its exact empty-to-nonempty state transition when the first source is added.
    TEST(SourceManagerTest, ReportsEmptyStateTransition)
    {
      SourceManager Sources;

      EXPECT_TRUE(Sources.empty());
      EXPECT_EQ(Sources.size(), 0U);
      const SourceFileId Id = Sources.addSourceFile("", "");
      EXPECT_FALSE(Sources.empty());
      EXPECT_EQ(Sources.size(), 1U);
      EXPECT_TRUE(Sources.contains(Id));
      EXPECT_TRUE(Sources.sourceFile(Id).Path.empty());
      EXPECT_TRUE(Sources.sourceFile(Id).Contents.empty());
    }

    // Verifies that source files receive deterministic insertion-order IDs and retain their owned path and contents.
    TEST(SourceManagerTest, AddsOwnedSourceFilesWithStableIds)
    {
      SourceManager Sources;
      std::string Path = "first.ink";
      std::string Contents = "fn first() {}";
      const SourceFileId FirstId = Sources.addSourceFile(Path, Contents);
      Path = "changed.ink";
      Contents = "changed";
      const SourceFileId SecondId = Sources.addSourceFile("second.ink", "fn second() {}");

      EXPECT_EQ(FirstId.value(), 0U);
      EXPECT_EQ(SecondId.value(), 1U);
      EXPECT_EQ(Sources.size(), 2U);
      EXPECT_FALSE(Sources.empty());
      EXPECT_EQ(Sources.sourceFile(FirstId).Id, FirstId);
      EXPECT_EQ(Sources.sourceFile(FirstId).Path, "first.ink");
      EXPECT_EQ(Sources.sourceFile(FirstId).Contents, "fn first() {}");
      EXPECT_EQ(Sources.sourceFile(SecondId).Path, "second.ink");
    }

    // Verifies that adding source files does not invalidate a previously retrieved source-file record or its contents.
    TEST(SourceManagerTest, KeepsExistingSourceFileReferencesStable)
    {
      SourceManager Sources;
      const SourceFileId FirstId = Sources.addSourceFile("first.ink", "first");
      const SourceFile &First = Sources.sourceFile(FirstId);
      const std::string *FirstContents = &First.Contents;
      for (std::uint32_t Index = 0; Index < 128; ++Index)
      {
        Sources.addSourceFile("generated-" + std::to_string(Index) + ".ink", std::to_string(Index));
      }

      EXPECT_EQ(&Sources.sourceFile(FirstId), &First);
      EXPECT_EQ(&Sources.sourceFile(FirstId).Contents, FirstContents);
      EXPECT_EQ(*FirstContents, "first");
    }

    // Verifies source storage owns complete byte sequences, including embedded null bytes, without treating them as C strings.
    TEST(SourceManagerTest, PreservesBinarySourceContents)
    {
      SourceManager Sources;
      std::string Contents("a\0b", 3);
      const SourceFileId Id = Sources.addSourceFile("binary.ink", Contents);
      Contents.assign("changed");

      const std::string &Stored = Sources.sourceFile(Id).Contents;
      ASSERT_EQ(Stored.size(), 3U);
      EXPECT_EQ(Stored[0], 'a');
      EXPECT_EQ(Stored[1], '\0');
      EXPECT_EQ(Stored[2], 'b');
    }

    // Verifies that invalid and out-of-range IDs are rejected instead of aliasing an existing source file.
    TEST(SourceManagerTest, RejectsIdsOutsideTheManager)
    {
      SourceManager Sources;
      const SourceFileId Existing = Sources.addSourceFile("only.ink", "only");
      const SourceFileId OutOfRange = SourceFileId::fromValue(1);

      EXPECT_TRUE(Sources.contains(Existing));
      EXPECT_FALSE(Sources.contains(SourceFileId{}));
      EXPECT_FALSE(Sources.contains(OutOfRange));
      EXPECT_THROW(Sources.sourceFile(SourceFileId{}), std::out_of_range);
      EXPECT_THROW(Sources.sourceFile(OutOfRange), std::out_of_range);
    }
  } // namespace
} // namespace ink::core
