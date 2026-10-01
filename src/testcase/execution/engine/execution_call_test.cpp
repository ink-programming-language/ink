#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace ink::execution::test
{
  namespace
  {
    class CallContext final
    {
      public:
        explicit CallContext(ExecutionLimits Limits = {})
            : Context(Compilation),
              Builder(Context),
              Engine(Context, Limits),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        const ir::IntegerConstant &integer(std::uint64_t Value)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Value));
        }

        std::unique_ptr<ir::Function> function(bool ReturnsVoid = false)
        {
          const ir::Type *Parameters[] = {&Int32};
          const ir::Type &ReturnType = ReturnsVoid ? Context.typePool().getType<ir::TypeKind::Void>() : static_cast<const ir::Type &>(Int32);
          const ir::FunctionType *Signature = Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern("F"), *Signature) : nullptr;
        }

        bool begin(ir::Function &Function)
        {
          auto *Body = Builder.createFunctionBody(Function);
          return Body && Builder.setInsertPoint(*Body);
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
    };
  } // namespace

  // Nested invocations isolate mutable parameter copies, retain returned snapshots and release their depth slots for later calls.
  TEST(ExecutionCallTest, CreatesFreshLocalStorageAndPreservesReturnedSnapshots)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 2;
    CallContext Test(Limits);
    auto Callee = Test.function();
    auto Caller = Test.function();
    ASSERT_NE(Callee, nullptr);
    ASSERT_NE(Caller, nullptr);
    ASSERT_TRUE(Test.begin(*Callee));
    auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, *Callee->parameters().front()), nullptr);
    auto *Snapshot = Test.Builder.createLoadInstruction(*Slot);
    ASSERT_NE(Snapshot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, Test.integer(99)), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Snapshot), nullptr);
    ASSERT_TRUE(Test.begin(*Caller));
    const ir::Value *CallArguments[] = {Caller->parameters().front().get()};
    auto *Called = Test.Builder.createCallInstruction(*Callee, CallArguments);
    ASSERT_NE(Called, nullptr);
    auto *Sum = Test.Builder.createAddInstruction(*Called, *Caller->parameters().front());
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    ExecutionValueRef Arguments[] = {Test.Engine.heap().fromConstant(Test.integer(11))};
    const auto First = Test.Engine.execute(*Caller, Arguments);
    ASSERT_TRUE(First);
    EXPECT_EQ(First.Value.toConstant(Test.Context), &Test.integer(22));
    EXPECT_EQ(Arguments[0].toConstant(Test.Context), &Test.integer(11));
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    Arguments[0] = Test.Engine.heap().fromConstant(Test.integer(22));
    const auto Second = Test.Engine.execute(*Caller, Arguments);
    ASSERT_TRUE(Second);
    EXPECT_EQ(Second.Value.toConstant(Test.Context), &Test.integer(44));
    EXPECT_EQ(First.Value.toConstant(Test.Context), &Test.integer(22));
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
  }

  // A nested instruction failure propagates unchanged, releases both invocation frames and allows a later valid call.
  TEST(ExecutionCallTest, ClosesNestedCallFramesWithoutOverwritingBodyFailure)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 2;
    CallContext Test(Limits);
    auto Callee = Test.function();
    auto Caller = Test.function();
    auto Identity = Test.function();
    ASSERT_NE(Callee, nullptr);
    ASSERT_NE(Caller, nullptr);
    ASSERT_NE(Identity, nullptr);
    ASSERT_TRUE(Test.begin(*Callee));
    auto *Uninitialized = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Uninitialized, nullptr);
    auto *Loaded = Test.Builder.createLoadInstruction(*Uninitialized);
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    ASSERT_TRUE(Test.begin(*Caller));
    ASSERT_NE(Test.Builder.createAllocaInstruction(Test.Int32), nullptr);
    const ir::Value *CallArguments[] = {Caller->parameters().front().get()};
    auto *Called = Test.Builder.createCallInstruction(*Callee, CallArguments);
    ASSERT_NE(Called, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Called), nullptr);
    ASSERT_TRUE(Test.begin(*Identity));
    ASSERT_NE(Test.Builder.createReturnInstruction(Identity->parameters().front().get()), nullptr);
    const ExecutionValueRef Arguments[] = {Test.Engine.heap().fromConstant(Test.integer(7))};
    const auto Failed = Test.Engine.execute(*Caller, Arguments);
    EXPECT_EQ(Failed.Status, ExecutionStatus::Uninitialized);
    EXPECT_FALSE(Failed.Value.valid());
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::Uninitialized);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    const auto Recovered = Test.Engine.execute(*Identity, Arguments);
    ASSERT_TRUE(Recovered);
    EXPECT_EQ(Recovered.Value.toConstant(Test.Context), &Test.integer(7));
  }

  // A void IR return produces an owned void value, while an unterminated body cannot manufacture a successful result.
  TEST(ExecutionCallTest, ReturnsVoidValuesAndRejectsMissingReturns)
  {
    CallContext Test;
    auto VoidFunction = Test.function(true);
    auto MissingReturn = Test.function();
    ASSERT_NE(VoidFunction, nullptr);
    ASSERT_NE(MissingReturn, nullptr);
    ASSERT_TRUE(Test.begin(*VoidFunction));
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    ASSERT_TRUE(Test.begin(*MissingReturn));
    ASSERT_NE(Test.Builder.createAllocaInstruction(Test.Int32), nullptr);
    const ExecutionValueRef Arguments[] = {Test.Engine.heap().fromConstant(Test.integer(1))};
    const auto Void = Test.Engine.execute(*VoidFunction, Arguments);
    ASSERT_TRUE(Void);
    EXPECT_EQ(Void.Value.kind(), ExecutionValueKind::Void);
    EXPECT_EQ(Void.Value.type(), &Test.Context.typePool().getType<ir::TypeKind::Void>());
    const auto Missing = Test.Engine.execute(*MissingReturn, Arguments);
    EXPECT_EQ(Missing.Status, ExecutionStatus::MissingBody);
    EXPECT_FALSE(Missing.Value.valid());
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
  }

  // Recursive IR calls release every local cell on depth exhaustion and preserve the terminal failure on subsequent entry.
  TEST(ExecutionCallTest, RecursionSharesDepthBudgetAndUnwindsFrames)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 2;
    CallContext Test(Limits);
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, *Function->parameters().front()), nullptr);
    const ir::Value *CallArguments[] = {Function->parameters().front().get()};
    auto *Called = Test.Builder.createCallInstruction(*Function, CallArguments);
    ASSERT_NE(Called, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Called), nullptr);
    const ExecutionValueRef Arguments[] = {Test.Engine.heap().fromConstant(Test.integer(1))};
    const auto Result = Test.Engine.execute(*Function, Arguments);
    EXPECT_EQ(Result.Status, ExecutionStatus::BudgetExceeded);
    EXPECT_FALSE(Result.Value.valid());
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.execute(*Function, Arguments).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
  }
} // namespace ink::execution::test
