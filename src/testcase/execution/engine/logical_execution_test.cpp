#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <iterator>
#include <span>

namespace ink::execution::test
{
  namespace
  {
    struct LogicalExecutionContext
    {
        explicit LogicalExecutionContext(ExecutionLimits Limits = {})
            : Engine(Context, Limits)
        {
        }

        std::unique_ptr<ir::Function> function(std::span<const ir::Type *const> Parameters = {})
        {
          return Builder.createFunction(Context.namePool().intern("Logical"), *Context.typePool().getType<ir::TypeKind::Function>(Bool, Parameters));
        }

        bool begin(ir::Function &Function)
        {
          auto *Block = Builder.createFunctionBody(Function);
          return Block && Builder.setInsertPoint(*Block);
        }

        core::CompilationContext Compilation;
        ir::IRContext Context{Compilation};
        ir::IRBuilder Builder{Context};
        ExecutionEngine Engine;
        const ir::Type &Bool = Context.typePool().getType<ir::TypeKind::Bool>();
    };

    void expectBoolean(const ExecutionValueResult &Result, bool Expected)
    {
      ASSERT_TRUE(Result);
      ASSERT_EQ(Result.Value.kind(), RuntimeKind::Boolean);
      EXPECT_EQ(Result.Value.boolean(), Expected);
    }
  } // namespace

  // Bool parameters cover the complete truth tables for eager not/and/or and equality without growing the constant pool.
  TEST(LogicalExecutionTest, ExecutesBooleanTruthTables)
  {
    LogicalExecutionContext Test;
    const ir::Type *Parameters[] = {&Test.Bool, &Test.Bool};
    struct Row
    {
        bool Left;
        bool Right;
        std::array<bool, 5> Expected;
    };
    const Row Rows[] = {
        {false, false, {true, false, false, true, false}},
        {false, true, {true, false, true, false, true}},
        {true, false, {false, false, true, false, true}},
        {true, true, {false, true, true, true, false}},
    };
    const auto ConstantCount = Test.Context.constantPool().size();
    for (std::size_t Operation = 0; Operation < 5; ++Operation)
    {
      auto Function = Test.function(Parameters);
      ASSERT_NE(Function, nullptr);
      ASSERT_TRUE(Test.begin(*Function));
      const auto &Left = *Function->parameters()[0];
      const auto &Right = *Function->parameters()[1];
      const ir::Value *Value = nullptr;
      switch (Operation)
      {
      case 0: Value = Test.Builder.createLogicalNotInstruction(Left); break;
      case 1: Value = Test.Builder.createLogicalAndInstruction(Left, Right); break;
      case 2: Value = Test.Builder.createLogicalOrInstruction(Left, Right); break;
      case 3: Value = Test.Builder.createCompareInstruction(core::ComparisonPredicate::Equal, Left, Right); break;
      case 4: Value = Test.Builder.createCompareInstruction(core::ComparisonPredicate::NotEqual, Left, Right); break;
      }
      ASSERT_NE(Value, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
      for (const auto &Row : Rows)
      {
        SCOPED_TRACE(Operation);
        SCOPED_TRACE(Row.Left);
        SCOPED_TRACE(Row.Right);
        const ExecutionValueRef Arguments[] = {Test.Engine.heap().boolean(Test.Bool, Row.Left), Test.Engine.heap().boolean(Test.Bool, Row.Right)};
        expectBoolean(Test.Engine.execute(*Function, Arguments), Row.Expected[Operation]);
        EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
      }
    }
    EXPECT_EQ(Test.Context.constantPool().size(), ConstantCount);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

  // Every integer predicate respects signedness and all 128 bits, including the sign bit and differences in the high word.
  TEST(LogicalExecutionTest, ComparesSignedUnsignedAndWideIntegers)
  {
    LogicalExecutionContext Test;
    struct Row
    {
        std::uint32_t Width;
        bool Signed;
        std::array<std::uint64_t, 2> Left;
        std::array<std::uint64_t, 2> Right;
        std::array<bool, 6> Expected;
    };
    const Row Rows[] = {
        {8, true, {255, 0}, {0, 0}, {false, true, true, true, false, false}},
        {8, false, {255, 0}, {0, 0}, {false, true, false, false, true, true}},
        {32, true, {0x80000000, 0}, {0x7fffffff, 0}, {false, true, true, true, false, false}},
        {32, false, {0x80000000, 0}, {0x7fffffff, 0}, {false, true, false, false, true, true}},
        {128, true, {0, 0x8000000000000000ULL}, {0, 0}, {false, true, true, true, false, false}},
        {128, false, {0, 0x8000000000000000ULL}, {0, 0}, {false, true, false, false, true, true}},
        {128, true, {5, 1}, {5, 2}, {false, true, true, true, false, false}},
        {128, false, {5, 2}, {5, 2}, {true, false, false, true, false, true}},
    };
    const core::ComparisonPredicate Predicates[] = {
        core::ComparisonPredicate::Equal,
        core::ComparisonPredicate::NotEqual,
        core::ComparisonPredicate::Less,
        core::ComparisonPredicate::LessEqual,
        core::ComparisonPredicate::Greater,
        core::ComparisonPredicate::GreaterEqual,
    };
    for (const auto &Row : Rows)
    {
      const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Integer>(Row.Width, Row.Signed);
      ASSERT_NE(Type, nullptr);
      const ir::Type *Parameters[] = {Type, Type};
      const std::size_t WordCount = (Row.Width + 63) / 64;
      const ExecutionValueRef Arguments[] = {Test.Engine.heap().integer(*Type, ExecutionInteger(ir::IntegerBits(Row.Width, std::span(Row.Left).first(WordCount)))), Test.Engine.heap().integer(*Type, ExecutionInteger(ir::IntegerBits(Row.Width, std::span(Row.Right).first(WordCount))))};
      for (std::size_t Index = 0; Index < std::size(Predicates); ++Index)
      {
        SCOPED_TRACE(Row.Width);
        SCOPED_TRACE(Row.Signed);
        SCOPED_TRACE(Index);
        auto Function = Test.function(Parameters);
        ASSERT_NE(Function, nullptr);
        ASSERT_TRUE(Test.begin(*Function));
        auto *Compare = Test.Builder.createCompareInstruction(Predicates[Index], *Function->parameters()[0], *Function->parameters()[1]);
        ASSERT_NE(Compare, nullptr);
        ASSERT_NE(Test.Builder.createReturnInstruction(Compare), nullptr);
        expectBoolean(Test.Engine.execute(*Function, Arguments), Row.Expected[Index]);
      }
    }
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

  // Operand failures propagate, and eager and/or still require the right value even when the left value decides the truth table.
  TEST(LogicalExecutionTest, PropagatesUnavailableOperandsWithoutShortCircuitingIr)
  {
    LogicalExecutionContext Test;
    const auto &True = Test.Context.constantPool().getBoolConstant(true);
    const auto &False = Test.Context.constantPool().getBoolConstant(false);
    auto Address = Test.Builder.createDetachedAllocaInstruction(Test.Bool);
    ASSERT_NE(Address, nullptr);
    auto Missing = Test.Builder.createDetachedLoadInstruction(*Address);
    ASSERT_NE(Missing, nullptr);
    for (std::size_t Operation = 0; Operation < 7; ++Operation)
    {
      auto Function = Test.function();
      ASSERT_NE(Function, nullptr);
      ASSERT_TRUE(Test.begin(*Function));
      const ir::Value *Value = nullptr;
      switch (Operation)
      {
      case 0: Value = Test.Builder.createLogicalNotInstruction(*Missing); break;
      case 1: Value = Test.Builder.createLogicalAndInstruction(False, *Missing); break;
      case 2: Value = Test.Builder.createLogicalOrInstruction(True, *Missing); break;
      case 3: Value = Test.Builder.createCompareInstruction(core::ComparisonPredicate::Equal, True, *Missing); break;
      case 4: Value = Test.Builder.createLogicalAndInstruction(*Missing, False); break;
      case 5: Value = Test.Builder.createLogicalOrInstruction(*Missing, True); break;
      case 6: Value = Test.Builder.createCompareInstruction(core::ComparisonPredicate::NotEqual, *Missing, False); break;
      }
      ASSERT_NE(Value, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
      EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::RuntimeValue);
      EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::RuntimeValue);
      EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
    }
  }

  // Logical and comparison execution remain subject to the shared step limit and release temporary values on failure.
  TEST(LogicalExecutionTest, HonorsStepBudgetsAndReleasesTemporaryValues)
  {
    for (std::size_t Operation = 0; Operation < 4; ++Operation)
    {
      ExecutionLimits Limits;
      Limits.MaxSteps = 4;
      LogicalExecutionContext Test(Limits);
      auto Function = Test.function();
      ASSERT_NE(Function, nullptr);
      ASSERT_TRUE(Test.begin(*Function));
      const auto &True = Test.Context.constantPool().getBoolConstant(true);
      const ir::Value *Value = nullptr;
      switch (Operation)
      {
      case 0: Value = Test.Builder.createLogicalNotInstruction(True); break;
      case 1: Value = Test.Builder.createLogicalAndInstruction(True, True); break;
      case 2: Value = Test.Builder.createLogicalOrInstruction(True, True); break;
      case 3: Value = Test.Builder.createCompareInstruction(core::ComparisonPredicate::Equal, True, True); break;
      }
      ASSERT_NE(Value, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
      EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::BudgetExceeded);
      EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
      EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
      EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::BudgetExceeded);
    }
  }

  // Returned logical results are immutable snapshots and remain valid after their invocation and engine are destroyed.
  TEST(LogicalExecutionTest, ReturnedBooleanOutlivesTheEngine)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ir::IRBuilder Builder(Context);
    const auto &Bool = Context.typePool().getType<ir::TypeKind::Bool>();
    const ir::Type *Parameters[] = {&Bool, &Bool};
    auto Function = Builder.createFunction(Context.namePool().intern("Either"), *Context.typePool().getType<ir::TypeKind::Function>(Bool, Parameters));
    ASSERT_NE(Function, nullptr);
    auto *Body = Builder.createFunctionBody(*Function);
    ASSERT_NE(Body, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Body));
    auto *Or = Builder.createLogicalOrInstruction(*Function->parameters()[0], *Function->parameters()[1]);
    ASSERT_NE(Or, nullptr);
    auto *Not = Builder.createLogicalNotInstruction(*Or);
    ASSERT_NE(Not, nullptr);
    ASSERT_NE(Builder.createReturnInstruction(Not), nullptr);
    ExecutionValueResult Retained;
    {
      ExecutionEngine Engine(Context);
      const ExecutionValueRef False[] = {Engine.heap().boolean(Bool, false), Engine.heap().boolean(Bool, false)};
      const ExecutionValueRef True[] = {Engine.heap().boolean(Bool, true), Engine.heap().boolean(Bool, false)};
      Retained = Engine.execute(*Function, False);
      expectBoolean(Retained, true);
      expectBoolean(Engine.execute(*Function, True), false);
      expectBoolean(Retained, true);
    }
    expectBoolean(Retained, true);
  }
} // namespace ink::execution::test
