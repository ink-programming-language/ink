#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#ifdef _WIN32
#define INK_TEST_EXPORT extern "C" __declspec(dllexport)
#else
#define INK_TEST_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace ink::execution::test
{
  namespace
  {
    ExecutionEngine *ObservedEngine = nullptr;
    unsigned ClearCallbackCount = 0;

    class BytecodeStorageContext final
    {
      public:
        BytecodeStorageContext()
            : Context(Compilation),
              Builder(Context),
              Engine(Context),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        std::unique_ptr<ir::Function> function(std::string_view Name, const ir::Type &ReturnType, std::span<const ir::Type *const> Parameters = {}, bool External = false)
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Name), *Signature, {}, {}, ir::CallingConvention::C, External ? ir::LanguageLinkage::C : ir::LanguageLinkage::Ink, External ? ir::FunctionBinding::Import : ir::FunctionBinding::Local) : nullptr;
        }

        bool begin(ir::Function &Function)
        {
          auto *Entry = Builder.createFunctionBody(Function);
          return Entry && Builder.setInsertPoint(*Entry);
        }

        const ir::IntegerConstant &integer(std::uint64_t Bits)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Bits));
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
    };
  } // namespace

  INK_TEST_EXPORT std::int32_t inkTestBytecodeLiveScalarValues() noexcept
  {
    return ObservedEngine ? static_cast<std::int32_t>(ObservedEngine->heap().liveValueCount()) : -1;
  }

  INK_TEST_EXPORT std::int32_t inkTestBytecodeClearNativeCache() noexcept
  {
    if (!ObservedEngine)
    {
      return -1;
    }
    ++ClearCallbackCount;
    ObservedEngine->clearNativeSymbolCache();
    return 37;
  }

  INK_TEST_EXPORT std::int32_t inkTestBytecodeLiveStorage() noexcept
  {
    return ObservedEngine ? static_cast<std::int32_t>(ObservedEngine->heap().liveStorageCount()) : -1;
  }

  // Reentering a local allocation in a loop resets its initialization even though its frame address is reused.
  TEST(BytecodeStorageTest, LoopAllocationResetsLocalInitialization)
  {
    BytecodeStorageContext Test;
    const auto &Bool = Test.Context.typePool().getType<ir::TypeKind::Bool>();
    const auto &False = Test.Context.constantPool().getBoolConstant(false);
    const auto &True = Test.Context.constantPool().getBoolConstant(true);
    auto Function = Test.function("ResetLocal", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Visited = Test.Builder.createAllocaInstruction(Bool);
    ASSERT_NE(Visited, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Visited, False), nullptr);
    auto *Loop = Test.Builder.createBasicBlock(*Function);
    auto *Initialize = Test.Builder.createBasicBlock(*Function);
    auto *Read = Test.Builder.createBasicBlock(*Function);
    ASSERT_NE(Test.Builder.createBranchInstruction(*Loop), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Loop));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    auto *Condition = Test.Builder.createLoadInstruction(*Visited);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Condition, nullptr);
    ASSERT_NE(Test.Builder.createConditionalBranchInstruction(*Condition, *Read, *Initialize), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Initialize));
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(42)), nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Visited, True), nullptr);
    ASSERT_NE(Test.Builder.createBranchInstruction(*Loop), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Read));
    auto *Value = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(Value, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
    EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::Uninitialized);
    EXPECT_EQ(Test.Engine.heap().allocatedStorageCount(), 3U);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
  }

  // Floating parameters and local load snapshots preserve signed zero and NaN payload bits at every supported width.
  TEST(BytecodeStorageTest, LocalFloatSnapshotsPreserveExactBits)
  {
    struct FloatCase
    {
        std::uint32_t Width;
        std::array<std::uint64_t, 4> Payloads;
    };
    constexpr FloatCase Cases[] = {
        {16, {0, 0x8000, 0x7e55, 0xfe37}},
        {32, {0, 0x80000000, 0x7fc12345, 0xffc54321}},
        {64, {0, 0x8000000000000000ULL, 0x7ff8123456789abcULL, 0xfff8abc987654321ULL}},
    };
    BytecodeStorageContext Test;
    for (const FloatCase &Case : Cases)
    {
      SCOPED_TRACE(Case.Width);
      const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Float>(Case.Width);
      ASSERT_NE(Type, nullptr);
      const ir::Type *ParameterTypes[] = {Type};
      auto Function = Test.function("FloatSnapshot", *Type, ParameterTypes);
      ASSERT_NE(Function, nullptr);
      ASSERT_TRUE(Test.begin(*Function));
      auto *Local = Test.Builder.createAllocaInstruction(*Type);
      ASSERT_NE(Local, nullptr);
      ASSERT_NE(Test.Builder.createStoreInstruction(*Local, *Function->parameters()[0]), nullptr);
      auto *Snapshot = Test.Builder.createLoadInstruction(*Local);
      ASSERT_NE(Snapshot, nullptr);
      const auto *Zero = Test.Context.constantPool().getFloatConstant(*Type, ir::FloatBits(Case.Width, 0));
      ASSERT_NE(Zero, nullptr);
      ASSERT_NE(Test.Builder.createStoreInstruction(*Local, *Zero), nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Snapshot), nullptr);
      for (std::uint64_t Payload : Case.Payloads)
      {
        SCOPED_TRACE(Payload);
        const ExecutionValueRef Arguments[] = {Test.Engine.heap().floating(*Type, ir::FloatBits(Case.Width, Payload))};
        ASSERT_TRUE(Arguments[0]);
        const auto Result = Test.Engine.execute(*Function, Arguments);
        ASSERT_TRUE(Result);
        ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Float);
        EXPECT_EQ(Result.Value.floating(), ir::FloatBits(Case.Width, Payload));
        EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
      }
      Test.Builder.clearInsertPoint();
    }
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

#if defined(_WIN32) || defined(__linux__)
  // A live native observer sees no managed allocation for frame-only locals, while their allocation budget is still charged.
  TEST(BytecodeStorageTest, FrameLocalsAvoidManagedStorageObjects)
  {
    BytecodeStorageContext Test;
    auto Observer = Test.function("inkTestBytecodeLiveStorage", Test.Int32, {}, true);
    auto Function = Test.function("FrameOnlyStorage", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(41)), nullptr);
    auto *Observed = Test.Builder.createCallInstruction(*Observer);
    auto *Loaded = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(Observed, nullptr);
    ASSERT_NE(Loaded, nullptr);
    auto *Sum = Test.Builder.createAddInstruction(*Observed, *Loaded);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    ObservedEngine = &Test.Engine;
    const auto Result = Test.Engine.execute(*Function);
    ObservedEngine = nullptr;
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().lowWord(), 41U);
    EXPECT_EQ(Test.Engine.heap().allocatedStorageCount(), 1U);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
  }

  // A native observer sees no boxed Add, Compare or local Load results while later instructions still need every snapshot.
  TEST(BytecodeStorageTest, ScalarInstructionResultsRemainUnboxedAcrossNativeCalls)
  {
    BytecodeStorageContext Test;
    auto Observer = Test.function("inkTestBytecodeLiveScalarValues", Test.Int32, {}, true);
    auto Function = Test.function("UnboxedScalarResults", Test.Int32);
    ASSERT_NE(Observer, nullptr);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(1)), nullptr);
    auto *FirstLoad = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(FirstLoad, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(2)), nullptr);
    auto *SecondLoad = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(SecondLoad, nullptr);
    constexpr std::uint64_t Count = 32;
    std::vector<const ir::Value *> AddResults;
    std::vector<const ir::Value *> CompareResults;
    const ir::Value *Previous = FirstLoad;
    for (std::uint64_t Index = 0; Index < Count; ++Index)
    {
      auto *Added = Test.Builder.createAddInstruction(*Previous, Test.integer(1));
      ASSERT_NE(Added, nullptr);
      auto *Compared = Test.Builder.createCompareInstruction(ir::ComparisonPredicate::Equal, *Added, Test.integer(Index + 2));
      ASSERT_NE(Compared, nullptr);
      AddResults.push_back(Added);
      CompareResults.push_back(Compared);
      Previous = Added;
    }
    auto *ObservedCount = Test.Builder.createCallInstruction(*Observer);
    ASSERT_NE(ObservedCount, nullptr);
    const ir::Value *Sum = Test.Builder.createAddInstruction(*FirstLoad, *SecondLoad);
    ASSERT_NE(Sum, nullptr);
    for (const ir::Value *Added : AddResults)
    {
      Sum = Test.Builder.createAddInstruction(*Sum, *Added);
      ASSERT_NE(Sum, nullptr);
    }
    const std::uint64_t ExpectedSum = 3 + Count * (Count + 3) / 2;
    const ir::Value *Correct = Test.Builder.createCompareInstruction(ir::ComparisonPredicate::Equal, *Sum, Test.integer(ExpectedSum));
    ASSERT_NE(Correct, nullptr);
    for (const ir::Value *Compared : CompareResults)
    {
      Correct = Test.Builder.createLogicalAndInstruction(*Correct, *Compared);
      ASSERT_NE(Correct, nullptr);
    }
    auto *Success = Test.Builder.createBasicBlock(*Function);
    auto *Failure = Test.Builder.createBasicBlock(*Function);
    ASSERT_NE(Success, nullptr);
    ASSERT_NE(Failure, nullptr);
    ASSERT_NE(Test.Builder.createConditionalBranchInstruction(*Correct, *Success, *Failure), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Success));
    ASSERT_NE(Test.Builder.createReturnInstruction(ObservedCount), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Failure));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(0xffffffff)), nullptr);
    for (unsigned Iteration = 0; Iteration < 2; ++Iteration)
    {
      EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
      ObservedEngine = &Test.Engine;
      const auto Result = Test.Engine.execute(*Function);
      ObservedEngine = nullptr;
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 0));
      EXPECT_EQ(Test.Engine.heap().liveValueCount(), 1U);
      EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    }
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

  // Clearing native plans inside a C callback preserves the active ABI call and the VM's suspended local snapshots.
  TEST(BytecodeStorageTest, NativeCallbacksCanClearPlansAndResumeTheMachine)
  {
    BytecodeStorageContext Test;
    auto Clear = Test.function("inkTestBytecodeClearNativeCache", Test.Int32, {}, true);
    auto Function = Test.function("ClearAndResume", Test.Int32);
    ASSERT_NE(Clear, nullptr);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(11)), nullptr);
    auto *Before = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(Before, nullptr);
    auto *FirstCall = Test.Builder.createCallInstruction(*Clear);
    ASSERT_NE(FirstCall, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(23)), nullptr);
    auto *SecondCall = Test.Builder.createCallInstruction(*Clear);
    ASSERT_NE(SecondCall, nullptr);
    auto *After = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(After, nullptr);
    auto *Snapshots = Test.Builder.createAddInstruction(*Before, *After);
    ASSERT_NE(Snapshots, nullptr);
    auto *CallResults = Test.Builder.createAddInstruction(*FirstCall, *SecondCall);
    ASSERT_NE(CallResults, nullptr);
    auto *Sum = Test.Builder.createAddInstruction(*Snapshots, *CallResults);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    ClearCallbackCount = 0;
    for (unsigned Iteration = 0; Iteration < 2; ++Iteration)
    {
      ObservedEngine = &Test.Engine;
      const auto Result = Test.Engine.execute(*Function);
      ObservedEngine = nullptr;
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 108));
      EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    }
    EXPECT_EQ(ClearCallbackCount, 4U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }
#endif
} // namespace ink::execution::test

#undef INK_TEST_EXPORT
