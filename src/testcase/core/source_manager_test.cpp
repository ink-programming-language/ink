#include "ink/core/source_manager.h"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace ink::core
{
  namespace
  {
    // Verifies that SourceId reserves zero for the invalid state and preserves strongly typed identity comparisons.
    TEST(SourceIdTest, RepresentsValidAndInvalidIdentities)
    {
      constexpr SourceId Invalid;
      constexpr SourceId First(1);
      constexpr SourceId FirstCopy(1);
      constexpr SourceId Second(2);

      EXPECT_FALSE(Invalid.valid());
      EXPECT_TRUE(First.valid());
      EXPECT_EQ(First.value(), 1U);
      EXPECT_EQ(First, FirstCopy);
      EXPECT_NE(First, Second);
    }

    // Verifies that registered source identities resolve to their stable names, text, and physical-line metadata.
    TEST(SourceManagerTest, RegistersAndFindsNamedSources)
    {
      SourceManager Manager;
      const SourceId FirstId = Manager.addSource("first.ink", "first\nsecond\n");
      const SourceId SecondId = Manager.addSource("<stdin>", "value");
      const std::shared_ptr<const SourceBuffer> First = Manager.findSource(FirstId);
      const std::shared_ptr<const SourceBuffer> Second = Manager.findSource(SecondId);

      ASSERT_NE(First, nullptr);
      ASSERT_NE(Second, nullptr);
      EXPECT_TRUE(FirstId.valid());
      EXPECT_TRUE(SecondId.valid());
      EXPECT_NE(FirstId, SecondId);
      EXPECT_EQ(Manager.sourceCount(), 2U);
      EXPECT_EQ(First->id(), FirstId);
      EXPECT_EQ(First->name(), "first.ink");
      EXPECT_EQ(First->text(), "first\nsecond\n");
      EXPECT_EQ(First->lineStarts(), (std::vector<std::size_t>{0, 6, 13}));
      EXPECT_EQ(First->lineNumber(0), 1U);
      EXPECT_EQ(First->lineNumber(6), 2U);
      EXPECT_EQ(First->lineNumber(First->text().size()), 3U);
      EXPECT_EQ(First->lineNumber(std::numeric_limits<std::size_t>::max()), 3U);
      EXPECT_EQ(Second->id(), SecondId);
      EXPECT_EQ(Second->name(), "<stdin>");
      EXPECT_EQ(Second->text(), "value");
    }

    // Verifies that invalid and out-of-range identities fail lookup without aliasing an existing source.
    TEST(SourceManagerTest, RejectsUnknownSourceIds)
    {
      SourceManager Manager;
      const SourceId Existing = Manager.addSource("known.ink", "known");

      ASSERT_NE(Manager.findSource(Existing), nullptr);
      EXPECT_EQ(Manager.findSource(SourceId{}), nullptr);
      EXPECT_EQ(Manager.findSource(SourceId(Existing.value() + 1)), nullptr);
    }

    // Verifies SourceIds remain globally distinct and cannot resolve to an unrelated source in another manager.
    TEST(SourceManagerTest, RejectsSourceIdsFromAnotherManager)
    {
      SourceManager FirstManager;
      SourceManager SecondManager;
      const SourceId FirstId = FirstManager.addSource("first.ink", "first");
      const SourceId SecondId = SecondManager.addSource("second.ink", "second");

      EXPECT_NE(FirstId, SecondId);
      EXPECT_EQ(FirstManager.findSource(SecondId), nullptr);
      EXPECT_EQ(SecondManager.findSource(FirstId), nullptr);
      ASSERT_NE(FirstManager.findSource(FirstId), nullptr);
      ASSERT_NE(SecondManager.findSource(SecondId), nullptr);
      EXPECT_EQ(FirstManager.findSource(FirstId)->name(), "first.ink");
      EXPECT_EQ(SecondManager.findSource(SecondId)->name(), "second.ink");
    }

    // Verifies that a resolved source handle keeps immutable source storage alive after its manager is destroyed.
    TEST(SourceManagerTest, ResolvedSourceHandleOwnsItsStorage)
    {
      std::shared_ptr<const SourceBuffer> Source;
      {
        SourceManager Manager;
        const SourceId Id = Manager.addSource("temporary.ink", "persistent text");
        Source = Manager.findSource(Id);
      }

      ASSERT_NE(Source, nullptr);
      EXPECT_EQ(Source->name(), "temporary.ink");
      EXPECT_EQ(Source->text(), "persistent text");
    }

    // Verifies that concurrent source registration publishes distinct identities and complete immutable buffers.
    TEST(SourceManagerTest, RegistersSourcesConcurrently)
    {
      constexpr std::size_t ThreadCount = 8;
      SourceManager Manager;
      std::array<SourceId, ThreadCount> Ids;
      std::vector<std::thread> Threads;
      Threads.reserve(ThreadCount);
      for (std::size_t Index = 0; Index < ThreadCount; ++Index)
      {
        Threads.emplace_back([&Manager, &Ids, Index]()
                             {
                               const std::string Suffix = std::to_string(Index);
                               Ids[Index] = Manager.addSource("source-" + Suffix + ".ink", "text-" + Suffix);
                             });
      }
      for (std::thread &Thread : Threads)
      {
        Thread.join();
      }

      std::unordered_set<std::uint64_t> UniqueIds;
      for (std::size_t Index = 0; Index < ThreadCount; ++Index)
      {
        const std::shared_ptr<const SourceBuffer> Source = Manager.findSource(Ids[Index]);
        ASSERT_NE(Source, nullptr);
        EXPECT_TRUE(UniqueIds.insert(Ids[Index].value()).second);
        EXPECT_EQ(Source->name(), "source-" + std::to_string(Index) + ".ink");
        EXPECT_EQ(Source->text(), "text-" + std::to_string(Index));
      }
      EXPECT_EQ(Manager.sourceCount(), ThreadCount);
    }
  } // namespace
} // namespace ink::core
