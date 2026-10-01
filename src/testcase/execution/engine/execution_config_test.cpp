#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/context.h"
#include "../../core/environment_test_support.h"

#include <gtest/gtest.h>

namespace ink::execution::test
{
  class ExecutionConfigTest : public ::testing::Test
  {
    protected:
      ExecutionConfigTest()
          : Steps("INK_EXECUTION_MAX_STEPS"),
            Objects("INK_EXECUTION_MAX_OBJECTS"),
            CallDepth("INK_EXECUTION_MAX_CALL_DEPTH"),
            EvaluationDepth("INK_EXECUTION_MAX_EVALUATION_DEPTH")
      {
      }

      void SetUp() override
      {
        ASSERT_TRUE(Steps.set(nullptr));
        ASSERT_TRUE(Objects.set(nullptr));
        ASSERT_TRUE(CallDepth.set(nullptr));
        ASSERT_TRUE(EvaluationDepth.set(nullptr));
      }

      core::test::ScopedEnvironmentVariable Steps;
      core::test::ScopedEnvironmentVariable Objects;
      core::test::ScopedEnvironmentVariable CallDepth;
      core::test::ScopedEnvironmentVariable EvaluationDepth;
  };

  // Unset execution overrides retain the documented defaults from the central configuration registry.
  TEST_F(ExecutionConfigTest, UsesRegisteredDefaultsWhenOverridesAreAbsent)
  {
    const ExecutionLimits Limits;
    EXPECT_EQ(Limits.MaxSteps, 100000U);
    EXPECT_EQ(Limits.MaxObjects, 16384U);
    EXPECT_EQ(Limits.MaxCallDepth, 256U);
    EXPECT_EQ(Limits.MaxEvaluationDepth, 64U);
  }

  // Every newly created limits object observes current environment values without changing an existing object's snapshot.
  TEST_F(ExecutionConfigTest, RefreshesEachLimitFromTheEnvironment)
  {
    ASSERT_TRUE(Steps.set("13"));
    ASSERT_TRUE(Objects.set("17"));
    ASSERT_TRUE(CallDepth.set("3"));
    ASSERT_TRUE(EvaluationDepth.set("5"));
    const ExecutionLimits First;
    EXPECT_EQ(First.MaxSteps, 13U);
    EXPECT_EQ(First.MaxObjects, 17U);
    EXPECT_EQ(First.MaxCallDepth, 3U);
    EXPECT_EQ(First.MaxEvaluationDepth, 5U);
    ASSERT_TRUE(Steps.set("23"));
    ASSERT_TRUE(Objects.set("29"));
    ASSERT_TRUE(CallDepth.set("7"));
    ASSERT_TRUE(EvaluationDepth.set("11"));
    const ExecutionLimits Second;
    EXPECT_EQ(Second.MaxSteps, 23U);
    EXPECT_EQ(Second.MaxObjects, 29U);
    EXPECT_EQ(Second.MaxCallDepth, 7U);
    EXPECT_EQ(Second.MaxEvaluationDepth, 11U);
    EXPECT_EQ(First.MaxSteps, 13U);
    EXPECT_EQ(First.MaxObjects, 17U);
    EXPECT_EQ(First.MaxCallDepth, 3U);
    EXPECT_EQ(First.MaxEvaluationDepth, 5U);
  }

  // Malformed, signed, whitespace-padded and overflowing overrides use each entry's registered default.
  TEST_F(ExecutionConfigTest, InvalidOverridesFallBackToRegisteredDefaults)
  {
    const char *InvalidValues[] = {
        "invalid",
        "-1",
        "+2",
        " 7",
        "184467440737095516160",
    };
    for (const char *Value : InvalidValues)
    {
      SCOPED_TRACE(Value);
      ASSERT_TRUE(Steps.set(Value));
      ASSERT_TRUE(Objects.set(Value));
      ASSERT_TRUE(CallDepth.set(Value));
      ASSERT_TRUE(EvaluationDepth.set(Value));
      const ExecutionLimits Limits;
      EXPECT_EQ(Limits.MaxSteps, 100000U);
      EXPECT_EQ(Limits.MaxObjects, 16384U);
      EXPECT_EQ(Limits.MaxCallDepth, 256U);
      EXPECT_EQ(Limits.MaxEvaluationDepth, 64U);
    }
  }

  // Zero is an intentional execution limit and prevents work rather than being replaced with a default.
  TEST_F(ExecutionConfigTest, PreservesZeroLimitsAndStopsExecution)
  {
    ASSERT_TRUE(Steps.set("0"));
    ASSERT_TRUE(Objects.set("0"));
    ASSERT_TRUE(CallDepth.set("0"));
    ASSERT_TRUE(EvaluationDepth.set("0"));
    const ExecutionLimits Limits;
    EXPECT_EQ(Limits.MaxSteps, 0U);
    EXPECT_EQ(Limits.MaxObjects, 0U);
    EXPECT_EQ(Limits.MaxCallDepth, 0U);
    EXPECT_EQ(Limits.MaxEvaluationDepth, 0U);
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionEngine Engine(Context);
    EXPECT_EQ(Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Engine.createFrame(ExecutionFrameKind::Module), nullptr);
  }

  // An engine captures defaults when it is constructed, while explicit limits continue to override those defaults.
  TEST_F(ExecutionConfigTest, CapturesEngineDefaultsAndAcceptsExplicitOverrides)
  {
    ASSERT_TRUE(Steps.set("2"));
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionEngine DefaultEngine(Context);
    ASSERT_TRUE(Steps.set("9"));
    EXPECT_EQ(DefaultEngine.consumeStep(), ExecutionStatus::Success);
    EXPECT_EQ(DefaultEngine.consumeStep(), ExecutionStatus::Success);
    EXPECT_EQ(DefaultEngine.consumeStep(), ExecutionStatus::BudgetExceeded);
    ExecutionLimits Explicit;
    Explicit.MaxSteps = 1;
    ExecutionEngine CustomEngine(Context, Explicit);
    EXPECT_EQ(CustomEngine.consumeStep(), ExecutionStatus::Success);
    EXPECT_EQ(CustomEngine.consumeStep(), ExecutionStatus::BudgetExceeded);
    ExecutionEngine RefreshedEngine(Context);
    for (unsigned Index = 0; Index < 9; ++Index)
    {
      EXPECT_EQ(RefreshedEngine.consumeStep(), ExecutionStatus::Success);
    }
    EXPECT_EQ(RefreshedEngine.consumeStep(), ExecutionStatus::BudgetExceeded);
  }
} // namespace ink::execution::test
