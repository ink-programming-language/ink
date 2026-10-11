#include "ink/core/singleton.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

namespace ink::core::test
{
  template <unsigned Tag>
  class TestSingleton final : public Singleton<TestSingleton<Tag>>
  {
    public:
      static std::size_t constructionCount() noexcept
      {
        return ConstructionCount;
      }

      unsigned Value = 0;

    private:
      friend class Singleton<TestSingleton<Tag>>;

      TestSingleton()
      {
        ++ConstructionCount;
      }

      ~TestSingleton() = default;

      inline static std::size_t ConstructionCount = 0;
  };

  using LazySingleton = TestSingleton<0>;
  static_assert(!std::is_default_constructible_v<LazySingleton>);
  static_assert(!std::is_destructible_v<LazySingleton>);
  static_assert(!std::is_copy_constructible_v<LazySingleton>);
  static_assert(!std::is_copy_assignable_v<LazySingleton>);
  static_assert(!std::is_move_constructible_v<LazySingleton>);
  static_assert(!std::is_move_assignable_v<LazySingleton>);
  static_assert(std::is_same_v<decltype(LazySingleton::instance()), LazySingleton &>);

  // A private constructor runs on first access only, and subsequent access preserves identity and state.
  TEST(CoreSingletonTest, ConstructsLazilyOnceAndSharesState)
  {
    static const std::size_t InitialCount = LazySingleton::constructionCount();
    EXPECT_EQ(InitialCount, 0U);
    LazySingleton &First = LazySingleton::instance();
    EXPECT_EQ(LazySingleton::constructionCount(), 1U);
    First.Value = 42;
    LazySingleton &Second = LazySingleton::instance();
    EXPECT_EQ(&First, &Second);
    EXPECT_EQ(Second.Value, 42U);
    EXPECT_EQ(LazySingleton::constructionCount(), 1U);
  }

  // Each derived type owns a separate instance with independent construction and mutable state.
  TEST(CoreSingletonTest, DifferentTypesOwnIndependentInstances)
  {
    TestSingleton<1> &First = TestSingleton<1>::instance();
    TestSingleton<2> &Second = TestSingleton<2>::instance();
    EXPECT_NE(static_cast<const void *>(&First), static_cast<const void *>(&Second));
    First.Value = 7;
    Second.Value = 9;
    EXPECT_EQ(TestSingleton<1>::instance().Value, 7U);
    EXPECT_EQ(TestSingleton<2>::instance().Value, 9U);
    EXPECT_EQ(TestSingleton<1>::constructionCount(), 1U);
    EXPECT_EQ(TestSingleton<2>::constructionCount(), 1U);
  }
} // namespace ink::core::test
