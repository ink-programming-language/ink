#include "ink/core/support/core_define.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <string>
#include <string_view>
#include <thread>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace ink::core::test
{
  namespace
  {
    static_assert(INK_BUILD_DEBUG + INK_BUILD_DEVELOPMENT + INK_BUILD_SHIPPING == 1);
    static_assert(INK_ENABLE_ASSERTIONS == !INK_BUILD_SHIPPING);
    static_assert(INK_PLATFORM_POINTER_SIZE == sizeof(void *));

#if defined(_WIN32)
    static_assert(INK_PLATFORM_WINDOWS == 1);
    static_assert(INK_PLATFORM_LINUX + INK_PLATFORM_MACOS + INK_PLATFORM_ANDROID + INK_PLATFORM_IOS == 0);
#elif defined(__ANDROID__)
    static_assert(INK_PLATFORM_ANDROID == 1);
    static_assert(INK_PLATFORM_WINDOWS + INK_PLATFORM_LINUX + INK_PLATFORM_MACOS + INK_PLATFORM_IOS == 0);
#elif defined(__linux__)
    static_assert(INK_PLATFORM_LINUX == 1);
    static_assert(INK_PLATFORM_WINDOWS + INK_PLATFORM_MACOS + INK_PLATFORM_ANDROID + INK_PLATFORM_IOS == 0);
#elif defined(__APPLE__) && TARGET_OS_IOS
    static_assert(INK_PLATFORM_IOS == 1);
    static_assert(INK_PLATFORM_WINDOWS + INK_PLATFORM_LINUX + INK_PLATFORM_MACOS + INK_PLATFORM_ANDROID == 0);
#elif defined(__APPLE__) && TARGET_OS_OSX
    static_assert(INK_PLATFORM_MACOS == 1);
    static_assert(INK_PLATFORM_WINDOWS + INK_PLATFORM_LINUX + INK_PLATFORM_ANDROID + INK_PLATFORM_IOS == 0);
#endif

#if defined(__clang__)
    static_assert(INK_COMPILER_CLANG == 1 && INK_COMPILER_MSVC == 0 && INK_COMPILER_GCC == 0);
#elif defined(_MSC_VER)
    static_assert(INK_COMPILER_MSVC == 1 && INK_COMPILER_CLANG == 0 && INK_COMPILER_GCC == 0);
#elif defined(__GNUC__)
    static_assert(INK_COMPILER_GCC == 1 && INK_COMPILER_MSVC == 0 && INK_COMPILER_CLANG == 0);
#endif

#if defined(__apple_build_version__)
    static_assert(INK_COMPILER_APPLE_CLANG == 1);
#else
    static_assert(INK_COMPILER_APPLE_CLANG == 0);
#endif

#if defined(_MSC_VER)
    static_assert(INK_COMPILER_MSVC_ABI == 1);
#else
    static_assert(INK_COMPILER_MSVC_ABI == 0);
#endif

    struct ExplicitCondition
    {
      public:
        bool Value;
        int *Conversions;

        explicit operator bool() const noexcept
        {
          ++*Conversions;
          return Value;
        }
    };

    std::size_t countOccurrences(std::string_view Text, std::string_view Needle)
    {
      std::size_t Count = 0;
      std::size_t Position = 0;
      while ((Position = Text.find(Needle, Position)) != std::string_view::npos)
      {
        ++Count;
        Position += Needle.size();
      }
      return Count;
    }

    constexpr bool evaluateSuccessfulAssertions()
    {
      check(true);
      checkMsgf(true, "check {}", 1);
      checkAlways(true);
      checkAlwaysMsgf(true, "always check {}", 2);
      return ensure(true) && ensureMsgf(true, "ensure {}", 3) && ensureAlways(true) && ensureAlwaysMsgf(true, "always ensure {}", 4);
    }

    template <unsigned Tag>
    bool failTemplateEnsure()
    {
      return ensureMsgf(false, "template {}", Tag);
    }

    static_assert(ensure(true));
    static_assert(ensureAlways(true));
    static_assert(ensureAlwaysMsgf(true, "always ensure"));
    static_assert(evaluateSuccessfulAssertions());
  } // namespace

  // Generated configuration identifies the selected build mode and the actual platform and compiler metadata.
  TEST(CoreDefineTest, GeneratedConfigurationProvidesBuildAndToolchainMetadata)
  {
#if INK_BUILD_DEBUG
    EXPECT_STREQ(INK_BUILD_MODE_NAME, "debug");
#elif INK_BUILD_DEVELOPMENT
    EXPECT_STREQ(INK_BUILD_MODE_NAME, "development");
#else
    EXPECT_STREQ(INK_BUILD_MODE_NAME, "shipping");
#endif
    EXPECT_FALSE(std::string_view(INK_PLATFORM_NAME).empty());
    EXPECT_FALSE(std::string_view(INK_COMPILER_ID).empty());
    EXPECT_FALSE(std::string_view(INK_COMPILER_VERSION).empty());
  }

  // All check variants evaluate successful conditions once when enabled and skip formatting arguments.
  TEST(CoreDefineTest, ChecksEvaluateConditionsOnlyWhenEnabled)
  {
    int Conditions = 0;
    int Arguments = 0;
    check(++Conditions > 0);
    checkMsgf(++Conditions > 0, "unused {}", ++Arguments);
    checkAlways(++Conditions > 0);
    checkAlwaysMsgf(++Conditions > 0, "unused {}", ++Arguments);
    EXPECT_EQ(Conditions, INK_ENABLE_ASSERTIONS ? 4 : 0);
    EXPECT_EQ(Arguments, 0);
  }

  // Conditions with an explicit bool conversion are accepted and converted at most once per evaluated macro.
  TEST(CoreDefineTest, AcceptsExplicitBoolConditions)
  {
    int Conversions = 0;
    ExplicitCondition Condition{true, &Conversions};
    check(Condition);
    checkMsgf(Condition, "check");
    checkAlways(Condition);
    checkAlwaysMsgf(Condition, "always check");
    EXPECT_TRUE(ensure(Condition));
    EXPECT_TRUE(ensureMsgf(Condition, "ensure"));
    EXPECT_TRUE(ensureAlways(Condition));
    EXPECT_TRUE(ensureAlwaysMsgf(Condition, "always ensure"));
    EXPECT_EQ(Conversions, 4 + (INK_ENABLE_ASSERTIONS ? 4 : 0));
  }

  // Statement macros remain a single statement when used directly in an if/else branch.
  TEST(CoreDefineTest, StatementMacrosDoNotCaptureOuterElse)
  {
    int UnexpectedBranches = 0;
    if (true)
      check(true);
    else
      ++UnexpectedBranches;
    if (true)
      checkMsgf(true, "check");
    else
      ++UnexpectedBranches;
    if (true)
      checkAlways(true);
    else
      ++UnexpectedBranches;
    if (true)
      checkAlwaysMsgf(true, "always check");
    else
      ++UnexpectedBranches;
    EXPECT_EQ(UnexpectedBranches, 0);
  }

#if !INK_ENABLE_ASSERTIONS
  // Disabled checks ignore false expressions and their messages without evaluating side effects or reporting failures.
  TEST(CoreDefineTest, DisabledChecksDoNotEvaluateConditionsOrReportFailures)
  {
    int Conditions = 0;
    int Arguments = 0;
    testing::internal::CaptureStderr();
    check(++Conditions < 0);
    checkMsgf(++Conditions < 0, "ignored {}", ++Arguments);
    checkAlways(++Conditions < 0);
    checkAlwaysMsgf(++Conditions < 0, "ignored {}", ++Arguments);
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(Conditions, 0);
    EXPECT_EQ(Arguments, 0);
    EXPECT_TRUE(Output.empty());
  }
#endif

  // Successful ensure expressions return true once and never evaluate formatting arguments or write a report.
  TEST(CoreDefineTest, SuccessfulEnsuresSkipReportingAndFormatting)
  {
    int Conditions = 0;
    int Arguments = 0;
    testing::internal::CaptureStderr();
    const bool First = ensure(++Conditions > 0);
    const bool Second = ensureMsgf(++Conditions > 0, "unused {}", ++Arguments);
    const bool Third = ensureAlways(++Conditions > 0);
    const bool Fourth = ensureAlwaysMsgf(++Conditions > 0, "unused {}", ++Arguments);
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(First);
    EXPECT_TRUE(Second);
    EXPECT_TRUE(Third);
    EXPECT_TRUE(Fourth);
    EXPECT_EQ(Conditions, 4);
    EXPECT_EQ(Arguments, 0);
    EXPECT_TRUE(Output.empty());
  }

  // Each ordinary ensure call site reports its first failure only and returns false for every later failure.
  TEST(CoreDefineTest, OrdinaryEnsureReportsOncePerCallSite)
  {
    int Conditions = 0;
    int Arguments = 0;
    int Failures = 0;
    testing::internal::CaptureStderr();
    for (int Iteration = 0; Iteration < 4; ++Iteration)
    {
      if (!ensure(++Conditions < 0))
      {
        ++Failures;
      }
      if (!ensureMsgf(++Conditions < 0, "first failure {}", ++Arguments))
      {
        ++Failures;
      }
    }
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(Conditions, 8);
    EXPECT_EQ(Failures, 8);
    EXPECT_EQ(Arguments, INK_ENABLE_ASSERTIONS ? 1 : 0);
    EXPECT_EQ(countOccurrences(Output, "ensure failed:"), INK_ENABLE_ASSERTIONS ? 2U : 0U);
#if INK_ENABLE_ASSERTIONS
    EXPECT_NE(Output.find("first failure 1"), std::string::npos);
#else
    EXPECT_TRUE(Output.empty());
#endif
  }

  // A successful first evaluation does not consume the later first-failure report for the same ensure call site.
  TEST(CoreDefineTest, SuccessDoesNotConsumeOrdinaryEnsureReport)
  {
    int Arguments = 0;
    std::array<bool, 3> Results{};
    testing::internal::CaptureStderr();
    for (std::size_t Index = 0; Index < Results.size(); ++Index)
    {
      Results[Index] = ensureMsgf(Index == 0, "failure {}", ++Arguments);
    }
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(Results[0]);
    EXPECT_FALSE(Results[1]);
    EXPECT_FALSE(Results[2]);
    EXPECT_EQ(Arguments, INK_ENABLE_ASSERTIONS ? 1 : 0);
    EXPECT_EQ(countOccurrences(Output, "ensure failed:"), INK_ENABLE_ASSERTIONS ? 1U : 0U);
  }

  // Separate template specializations each own one first-failure report even though they share the same source line.
  TEST(CoreDefineTest, TemplateSpecializationsKeepIndependentEnsureReports)
  {
    int Failures = 0;
    testing::internal::CaptureStderr();
    for (int Iteration = 0; Iteration < 3; ++Iteration)
    {
      if (!failTemplateEnsure<1>())
      {
        ++Failures;
      }
      if (!failTemplateEnsure<2>())
      {
        ++Failures;
      }
    }
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(Failures, 6);
    EXPECT_EQ(countOccurrences(Output, "ensure failed:"), INK_ENABLE_ASSERTIONS ? 2U : 0U);
#if INK_ENABLE_ASSERTIONS
    EXPECT_NE(Output.find("template 1"), std::string::npos);
    EXPECT_NE(Output.find("template 2"), std::string::npos);
#endif
  }

  // Always ensure returns false and reports each failure, while disabled ensuring still evaluates every condition.
  TEST(CoreDefineTest, AlwaysEnsureReportsEveryFailure)
  {
    int Conditions = 0;
    int Arguments = 0;
    int Failures = 0;
    testing::internal::CaptureStderr();
    for (int Iteration = 0; Iteration < 3; ++Iteration)
    {
      if (!ensureAlways(++Conditions < 0))
      {
        ++Failures;
      }
      if (!ensureAlwaysMsgf(++Conditions < 0, "failure {}", ++Arguments))
      {
        ++Failures;
      }
    }
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(Conditions, 6);
    EXPECT_EQ(Failures, 6);
    EXPECT_EQ(Arguments, INK_ENABLE_ASSERTIONS ? 3 : 0);
    EXPECT_EQ(countOccurrences(Output, "ensure failed:"), INK_ENABLE_ASSERTIONS ? 6U : 0U);
#if INK_ENABLE_ASSERTIONS
    EXPECT_NE(Output.find("failure 3"), std::string::npos);
#else
    EXPECT_TRUE(Output.empty());
#endif
  }

  // False conditions with explicit bool conversion are evaluated exactly once even after their report is suppressed.
  TEST(CoreDefineTest, FailedEnsureConvertsExplicitBoolOncePerEvaluation)
  {
    int Conversions = 0;
    ExplicitCondition Condition{false, &Conversions};
    int Failures = 0;
    testing::internal::CaptureStderr();
    for (int Iteration = 0; Iteration < 3; ++Iteration)
    {
      if (!ensure(Condition))
      {
        ++Failures;
      }
    }
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(Conversions, 3);
    EXPECT_EQ(Failures, 3);
    EXPECT_EQ(countOccurrences(Output, "ensure failed:"), INK_ENABLE_ASSERTIONS ? 1U : 0U);
  }

  // Concurrent failures at one ensure call site produce one report and evaluate the message arguments only once.
  TEST(CoreDefineTest, OrdinaryEnsureReportsOnceAcrossThreads)
  {
    std::atomic<bool> Start{false};
    std::atomic<int> Conditions{0};
    std::atomic<int> Arguments{0};
    std::atomic<int> Failures{0};
    std::array<std::thread, 8> Workers;
    testing::internal::CaptureStderr();
    for (std::thread &Worker : Workers)
    {
      Worker = std::thread([&]()
      {
        while (!Start.load(std::memory_order_acquire))
        {
          std::this_thread::yield();
        }
        for (int Iteration = 0; Iteration < 16; ++Iteration)
        {
          if (!ensureMsgf(Conditions.fetch_add(1, std::memory_order_relaxed) < 0, "concurrent {}", Arguments.fetch_add(1, std::memory_order_relaxed)))
          {
            Failures.fetch_add(1, std::memory_order_relaxed);
          }
        }
      });
    }
    Start.store(true, std::memory_order_release);
    for (std::thread &Worker : Workers)
    {
      Worker.join();
    }
    const std::string Output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(Conditions.load(), 128);
    EXPECT_EQ(Failures.load(), 128);
    EXPECT_EQ(Arguments.load(), INK_ENABLE_ASSERTIONS ? 1 : 0);
    EXPECT_EQ(countOccurrences(Output, "ensure failed:"), INK_ENABLE_ASSERTIONS ? 1U : 0U);
  }

#if INK_ENABLE_ASSERTIONS
  // Ensure reports include the original expression, formatted braces, and the exact source line even with logging disabled.
  TEST(CoreDefineTest, EnsureReportsExpressionMessageAndLocationWithLoggingDisabled)
  {
    const auto PreviousLevel = spdlog::get_level();
    spdlog::set_level(spdlog::level::off);
    testing::internal::CaptureStderr();
    const int ExpectedLine = __LINE__ + 1;
    const bool Result = ensureMsgf(2 + 2 == 5, "value {} {{ready}}", 42);
    const std::string Output = testing::internal::GetCapturedStderr();
    spdlog::set_level(PreviousLevel);
    EXPECT_FALSE(Result);
    EXPECT_NE(Output.find("ensure failed: 2 + 2 == 5"), std::string::npos);
    EXPECT_NE(Output.find("value 42 {ready}"), std::string::npos);
    EXPECT_NE(Output.find("ensure at " + std::string(__FILE__) + ":" + std::to_string(ExpectedLine)), std::string::npos);
  }
#endif

#if INK_ENABLE_ASSERTIONS
  // Failed checks terminate after reporting their original expression and formatted message.
  TEST(CoreDefineDeathTest, ChecksTerminateWithExpressionAndMessage)
  {
    EXPECT_DEATH(check(2 + 2 == 5), "check failed: 2 \\+ 2 == 5");
    EXPECT_DEATH(checkMsgf(false, "value {} {{ready}}", 42), "value 42 \\{ready\\}");
    EXPECT_DEATH(checkMsgf(false, "message without arguments"), "message without arguments");
    EXPECT_DEATH(checkAlways(2 + 2 == 5), "check failed: 2 \\+ 2 == 5");
    EXPECT_DEATH(checkAlwaysMsgf(false, "always check {}", 17), "always check 17");
    EXPECT_DEATH(checkAlwaysMsgf(false, "always message without arguments"), "always message without arguments");
  }

  // Check diagnostics retain the call-site source line and cannot be hidden by the global logger level.
  TEST(CoreDefineDeathTest, ChecksReportLocationWithLoggingDisabled)
  {
    const auto Fail = []()
    {
      spdlog::set_level(spdlog::level::off);
      checkMsgf(false, "logging disabled {}", 23);
    };
    EXPECT_DEATH(Fail(), "logging disabled 23");
    const std::string ExpectedLocation = "panic at .*core_define_test.cpp:" + std::to_string(__LINE__ + 1);
    EXPECT_DEATH(check(false), ExpectedLocation);
  }
#endif
} // namespace ink::core::test
