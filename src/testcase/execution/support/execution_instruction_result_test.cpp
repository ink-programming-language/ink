#include "ink/execution/support/execution_instruction_result.h"

#include "ink/execution/memory/execution_heap.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>
#include <utility>

namespace ink::execution::test
{
  static_assert(std::is_nothrow_default_constructible_v<ExecutionInstructionResult>);
  static_assert(std::is_nothrow_copy_constructible_v<ExecutionInstructionResult>);
  static_assert(std::is_nothrow_copy_assignable_v<ExecutionInstructionResult>);
  static_assert(std::is_nothrow_move_constructible_v<ExecutionInstructionResult>);
  static_assert(std::is_nothrow_move_assignable_v<ExecutionInstructionResult>);

  namespace
  {
    struct InstructionResultTestContext
    {
        core::CompilationContext Compilation;
        ir::IRContext Context{Compilation};
        ir::IRBuilder Builder{Context};
        ExecutionHeap Heap{Context};
        const ir::Type &Bool = Context.typePool().getType<ir::TypeKind::Bool>();
        const ir::Type &Void = Context.typePool().getType<ir::TypeKind::Void>();
    };

    void expectFailure(const ExecutionInstructionResult &Result, ExecutionStatus Status = ExecutionStatus::InvalidArguments)
    {
      EXPECT_EQ(Result.action(), ExecutionInstructionAction::Failure);
      EXPECT_EQ(Result.status(), Status);
      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(static_cast<bool>(Result));
      EXPECT_FALSE(Result.value());
      EXPECT_EQ(Result.target(), nullptr);
    }
  } // namespace

  // Default construction, empty value actions, and a failure marked Success all become consistent InvalidArguments failures.
  TEST(ExecutionInstructionResultTest, RejectsMissingValuesAndSuccessFailures)
  {
    expectFailure(ExecutionInstructionResult{});
    expectFailure(ExecutionInstructionResult::continueWith(ExecutionValueRef{}));
    expectFailure(ExecutionInstructionResult::returnValue(ExecutionValueRef{}));
    expectFailure(ExecutionInstructionResult::continueWith(ExecutionValueResult{}));
    expectFailure(ExecutionInstructionResult::returnValue(ExecutionValueResult{}));
    expectFailure(ExecutionInstructionResult::failure(ExecutionStatus::Success));
    ExecutionValueResult Missing;
    Missing.Status = ExecutionStatus::Success;
    expectFailure(ExecutionInstructionResult::continueWith(Missing));
    expectFailure(ExecutionInstructionResult::returnValue(Missing));
  }

  // Continue and Return retain the same immutable scalar or void payload while reporting distinct actions and no jump target.
  TEST(ExecutionInstructionResultTest, DistinguishesContinueAndReturnForScalarAndVoidValues)
  {
    InstructionResultTestContext Test;
    const ExecutionValueRef Values[] = {Test.Heap.boolean(Test.Bool, true), Test.Heap.voidValue(Test.Void)};
    for (const auto &Value : Values)
    {
      ASSERT_TRUE(Value);
      auto Continue = ExecutionInstructionResult::continueWith(Value);
      auto Return = ExecutionInstructionResult::returnValue(Value);
      EXPECT_EQ(Continue.action(), ExecutionInstructionAction::Continue);
      EXPECT_EQ(Return.action(), ExecutionInstructionAction::Return);
      for (const auto *Result : {&Continue, &Return})
      {
        EXPECT_TRUE(Result->succeeded());
        EXPECT_TRUE(static_cast<bool>(*Result));
        EXPECT_EQ(Result->status(), ExecutionStatus::Success);
        EXPECT_EQ(Result->value().get(), Value.get());
        EXPECT_EQ(Result->target(), nullptr);
      }
      const ExecutionValueResult Evaluated{ExecutionStatus::Success, Value};
      auto EvaluatedContinue = ExecutionInstructionResult::continueWith(Evaluated);
      auto EvaluatedReturn = ExecutionInstructionResult::returnValue(Evaluated);
      EXPECT_EQ(EvaluatedContinue.action(), ExecutionInstructionAction::Continue);
      EXPECT_EQ(EvaluatedReturn.action(), ExecutionInstructionAction::Return);
      EXPECT_EQ(EvaluatedContinue.value().get(), Value.get());
      EXPECT_EQ(EvaluatedReturn.value().get(), Value.get());
    }
  }

  // Failed evaluation results preserve their failure status and discard any inconsistent payload supplied by the caller.
  TEST(ExecutionInstructionResultTest, PropagatesFailuresWithoutValuesOrTargets)
  {
    InstructionResultTestContext Test;
    const ExecutionStatus Statuses[] = {ExecutionStatus::InvalidArguments, ExecutionStatus::TypeMismatch, ExecutionStatus::Uninitialized, ExecutionStatus::BudgetExceeded, ExecutionStatus::Cancelled};
    for (auto Status : Statuses)
    {
      expectFailure(ExecutionInstructionResult::failure(Status), Status);
      ExecutionValueResult Failed;
      Failed.Status = Status;
      Failed.Value = Test.Heap.boolean(Test.Bool, true);
      ASSERT_TRUE(Failed.Value);
      expectFailure(ExecutionInstructionResult::continueWith(Failed), Status);
      expectFailure(ExecutionInstructionResult::returnValue(std::move(Failed)), Status);
    }
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
  }

  // Jump results borrow the exact block identity and create no execution-value payload, including when copied.
  TEST(ExecutionInstructionResultTest, BorrowsJumpTargetsWithoutAllocatingValues)
  {
    InstructionResultTestContext Test;
    auto Target = Test.Builder.createBasicBlock();
    ASSERT_NE(Target, nullptr);
    const auto ValueCount = Test.Heap.liveValueCount();
    auto Jump = ExecutionInstructionResult::jumpTo(*Target);
    auto Copy = Jump;
    for (const auto *Result : {&Jump, &Copy})
    {
      EXPECT_EQ(Result->action(), ExecutionInstructionAction::Jump);
      EXPECT_EQ(Result->status(), ExecutionStatus::Success);
      EXPECT_TRUE(Result->succeeded());
      EXPECT_TRUE(static_cast<bool>(*Result));
      EXPECT_FALSE(Result->value());
      EXPECT_EQ(Result->target(), Target.get());
    }
    EXPECT_EQ(Test.Heap.liveValueCount(), ValueCount);
    Jump = ExecutionInstructionResult::failure(ExecutionStatus::Cancelled);
    expectFailure(Jump, ExecutionStatus::Cancelled);
    EXPECT_EQ(Copy.target(), Target.get());
  }

  // Moving any action transfers its complete state and resets the source to a payload-free InvalidArguments failure.
  TEST(ExecutionInstructionResultTest, MovesPreserveActionsAndResetSources)
  {
    InstructionResultTestContext Test;
    auto Target = Test.Builder.createBasicBlock();
    ASSERT_NE(Target, nullptr);
    std::array<ExecutionInstructionResult, 4> Results = {
        ExecutionInstructionResult::continueWith(Test.Heap.boolean(Test.Bool, true)),
        ExecutionInstructionResult::jumpTo(*Target),
        ExecutionInstructionResult::returnValue(Test.Heap.voidValue(Test.Void)),
        ExecutionInstructionResult::failure(ExecutionStatus::Cancelled),
    };
    for (auto &Result : Results)
    {
      const auto Action = Result.action();
      const auto Status = Result.status();
      const auto *Value = Result.value().get();
      const auto *Block = Result.target();
      auto Moved = std::move(Result);
      expectFailure(Result);
      EXPECT_EQ(Moved.action(), Action);
      EXPECT_EQ(Moved.status(), Status);
      EXPECT_EQ(Moved.value().get(), Value);
      EXPECT_EQ(Moved.target(), Block);
      auto Assigned = ExecutionInstructionResult::continueWith(Test.Heap.boolean(Test.Bool, false));
      Assigned = std::move(Moved);
      expectFailure(Moved);
      EXPECT_EQ(Assigned.action(), Action);
      EXPECT_EQ(Assigned.status(), Status);
      EXPECT_EQ(Assigned.value().get(), Value);
      EXPECT_EQ(Assigned.target(), Block);
    }
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
  }

  // Copied results share one payload, and replacing the last value action releases it without affecting a borrowed jump target.
  TEST(ExecutionInstructionResultTest, CopiesSharePayloadsAndAssignmentReleasesOldValues)
  {
    InstructionResultTestContext Test;
    auto Target = Test.Builder.createBasicBlock();
    ASSERT_NE(Target, nullptr);
    auto Result = ExecutionInstructionResult::continueWith(Test.Heap.boolean(Test.Bool, true));
    ASSERT_TRUE(Result);
    EXPECT_EQ(Test.Heap.liveValueCount(), 1U);
    {
      auto Copy = Result;
      ExecutionInstructionResult Assigned;
      Assigned = Copy;
      EXPECT_EQ(Copy.value().get(), Result.value().get());
      EXPECT_EQ(Assigned.value().get(), Result.value().get());
      Result = ExecutionInstructionResult::jumpTo(*Target);
      EXPECT_EQ(Test.Heap.liveValueCount(), 1U);
      EXPECT_TRUE(Copy.value().boolean());
      EXPECT_TRUE(Assigned.value().boolean());
    }
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
    EXPECT_EQ(Result.target(), Target.get());
    auto *Same = &Result;
    Result = std::move(*Same);
    EXPECT_EQ(Result.action(), ExecutionInstructionAction::Jump);
    EXPECT_EQ(Result.target(), Target.get());
  }
} // namespace ink::execution::test
