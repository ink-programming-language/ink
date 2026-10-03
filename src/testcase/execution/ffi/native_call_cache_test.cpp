#include "ink/execution/ffi/native_call_cache.h"
#include "ink/execution/ffi/native_symbol_cache.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <span>
#include <string_view>

namespace ink::execution::test
{
  namespace
  {
    class NativeCallContext final
    {
      public:
        explicit NativeCallContext(core::TargetContext Target = core::TargetContext::native())
            : Compilation(Target),
              Context(Compilation),
              Builder(Context),
              Heap(Context),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        std::unique_ptr<ir::Function> function(std::string_view Name, const ir::Type &ReturnType, std::span<const ir::Type *const> Parameters = {})
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Name), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, ir::FunctionBinding::Import) : nullptr;
        }

        ExecutionValueRef integer(std::uint64_t Bits)
        {
          return Heap.integer(Int32, ExecutionInteger(32, Bits));
        }

        ExecutionValueRef string(std::string_view Text)
        {
          const auto *Byte = Context.typePool().getType<ir::TypeKind::Integer>(8, false);
          const auto *Slice = Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
          return Heap.string(*Slice, Text);
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionHeap Heap;
        NativeCallCache Calls;
        const ir::IntegerType &Int32;
    };

    std::int32_t InvocationCount = 0;

    extern "C" std::int32_t inkTestPlannedIncrement(std::int32_t Value) noexcept
    {
      ++InvocationCount;
      return Value + 1;
    }

    extern "C" std::int32_t inkTestPlannedSecondIncrement(std::int32_t Value) noexcept
    {
      return Value + 2;
    }

    struct ReentrantCallState
    {
        NativeCallContext &Test;
        NativeSymbolCache &Symbols;
        const ir::Function &Function;
        bool InnerSucceeded = false;
        bool ClearBeforeRecursion = false;
    };

    ReentrantCallState *ActiveReentrantCall = nullptr;

    extern "C" std::int32_t inkTestPlannedRecursiveBuffer(std::uint8_t *Buffer, std::int32_t Depth) noexcept
    {
      ++Buffer[0];
      if (Depth == 0)
      {
        return Buffer[0];
      }
      ReentrantCallState &State = *ActiveReentrantCall;
      if (State.ClearBeforeRecursion)
      {
        State.Test.Calls.clear();
        State.Symbols.clear();
      }
      const ExecutionValueRef Arguments[] = {State.Test.string("B"), State.Test.integer(0)};
      const auto Result = State.Test.Calls.invoke(State.Test.Heap, State.Symbols, State.Function, Arguments);
      State.InnerSucceeded = static_cast<bool>(Result);
      return Result ? static_cast<std::int32_t>(1000 * Buffer[0] + Result.Value.integer().bits().words().front()) : -1;
    }
  } // namespace

  // Cached signatures reuse symbol binding while passing fresh scalar arguments and executing each native call.
  TEST(NativeCallCacheTest, ReusesPlanWithoutCachingValuesOrEffects)
  {
    NativeCallContext Test;
    const ir::Type *ParameterTypes[] = {&Test.Int32};
    auto Function = Test.function("plannedIncrement", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    unsigned ResolutionCount = 0;
    auto Resolve = [&](std::string_view Name) -> NativeSymbol
    {
      EXPECT_EQ(Name, "plannedIncrement");
      ++ResolutionCount;
      return reinterpret_cast<NativeSymbol>(&inkTestPlannedIncrement);
    };
    NativeSymbolCache Symbols(Resolve);
    InvocationCount = 0;
    EXPECT_EQ(Test.Calls.size(), 0U);
    for (std::uint64_t Value : {3, 19, 41})
    {
      const ExecutionValueRef Arguments[] = {Test.integer(Value)};
      const auto Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, Value + 1));
      EXPECT_EQ(Test.Calls.size(), 1U);
    }
    EXPECT_EQ(ResolutionCount, 1U);
    EXPECT_EQ(InvocationCount, 3);
  }

  // Clearing both caches permits rebinding the same declaration to a newly available native implementation.
  TEST(NativeCallCacheTest, ClearDiscardsThePreparedBinding)
  {
    NativeCallContext Test;
    const ir::Type *ParameterTypes[] = {&Test.Int32};
    auto Function = Test.function("reboundIncrement", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    NativeSymbol Address = reinterpret_cast<NativeSymbol>(&inkTestPlannedIncrement);
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      return Address;
    };
    NativeSymbolCache Symbols(Resolve);
    const ExecutionValueRef Arguments[] = {Test.integer(10)};
    auto Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 11));
    Address = reinterpret_cast<NativeSymbol>(&inkTestPlannedSecondIncrement);
    Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 11));
    Test.Calls.clear();
    Symbols.clear();
    EXPECT_EQ(Test.Calls.size(), 0U);
    Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 12));
  }

  // Unresolved symbols remain retryable and never leave a prepared call entry behind.
  TEST(NativeCallCacheTest, RetriesMissingSymbolsAtTheNextInvocation)
  {
    NativeCallContext Test;
    const ir::Type *ParameterTypes[] = {&Test.Int32};
    auto Function = Test.function("lateIncrement", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    bool Available = false;
    unsigned ResolutionCount = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++ResolutionCount;
      return Available ? reinterpret_cast<NativeSymbol>(&inkTestPlannedIncrement) : nullptr;
    };
    NativeSymbolCache Symbols(Resolve);
    const ExecutionValueRef Arguments[] = {Test.integer(10)};
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments).Status, ExecutionStatus::SymbolNotFound);
    EXPECT_EQ(Test.Calls.size(), 0U);
    Available = true;
    const auto Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 11));
    EXPECT_EQ(ResolutionCount, 2U);
    EXPECT_EQ(Test.Calls.size(), 1U);
  }

  // Cached plans still reject invalid and foreign arguments before invocation or any unexpected native side effect.
  TEST(NativeCallCacheTest, ValidatesArgumentsOnEveryInvocation)
  {
    NativeCallContext Test;
    NativeCallContext Other;
    const ir::Type *ParameterTypes[] = {&Test.Int32};
    auto Function = Test.function("validatedIncrement", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    unsigned ResolutionCount = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++ResolutionCount;
      return reinterpret_cast<NativeSymbol>(&inkTestPlannedIncrement);
    };
    NativeSymbolCache Symbols(Resolve);
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, {}).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(ResolutionCount, 0U);
    InvocationCount = 0;
    ExecutionValueRef Arguments[] = {Test.integer(10)};
    ASSERT_TRUE(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments));
    Arguments[0] = {};
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments).Status, ExecutionStatus::InvalidArguments);
    Arguments[0] = Other.integer(10);
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments).Status, ExecutionStatus::ForeignContext);
    Arguments[0] = Test.Heap.boolean(Test.Context.typePool().getType<ir::TypeKind::Bool>(), true);
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(InvocationCount, 1);
    EXPECT_EQ(ResolutionCount, 1U);
    EXPECT_EQ(Test.Calls.size(), 1U);
  }

  // Unsupported return layouts never create a usable plan or invoke a symbol with a mismatched C signature.
  TEST(NativeCallCacheTest, RejectsUnsupportedSignaturesWithoutCachingPlans)
  {
    NativeCallContext Test;
    const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, true);
    ASSERT_NE(Wide, nullptr);
    const ir::Type *ParameterTypes[] = {&Test.Int32};
    auto Function = Test.function("wideIncrement", *Wide, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    auto Resolve = [](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&inkTestPlannedIncrement);
    };
    NativeSymbolCache Symbols(Resolve);
    const ExecutionValueRef Arguments[] = {Test.integer(10)};
    InvocationCount = 0;
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments).Status, ExecutionStatus::UnsupportedExternalSignature);
    EXPECT_EQ(Test.Calls.size(), 0U);
    EXPECT_EQ(InvocationCount, 0);
  }

  // A non-native target fails before symbol resolution even when its pointer width and byte order match the host.
  TEST(NativeCallCacheTest, RejectsNonNativeTargetsBeforeResolution)
  {
    const auto Native = core::TargetContext::native();
    NativeCallContext Test(core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
    const ir::Type *ParameterTypes[] = {&Test.Int32};
    auto Function = Test.function("foreignIncrement", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    unsigned ResolutionCount = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++ResolutionCount;
      return reinterpret_cast<NativeSymbol>(&inkTestPlannedIncrement);
    };
    NativeSymbolCache Symbols(Resolve);
    const ExecutionValueRef Arguments[] = {Test.integer(10)};
    EXPECT_EQ(Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments).Status, ExecutionStatus::HostAbiMismatch);
    EXPECT_EQ(ResolutionCount, 0U);
    EXPECT_EQ(Test.Calls.size(), 0U);
  }

  // Recursive native callbacks reuse a signature while retaining independent mutable string buffers for both calls.
  TEST(NativeCallCacheTest, ReentrantCallsKeepTheirOwnArgumentStorage)
  {
    NativeCallContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    const ir::Type *ParameterTypes[] = {Pointer, &Test.Int32};
    auto Function = Test.function("recursiveBuffer", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    auto Resolve = [](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&inkTestPlannedRecursiveBuffer);
    };
    NativeSymbolCache Symbols(Resolve);
    ReentrantCallState State{Test, Symbols, *Function};
    ActiveReentrantCall = &State;
    const ExecutionValueRef Arguments[] = {Test.string("A"), Test.integer(1)};
    const auto Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
    ActiveReentrantCall = nullptr;
    ASSERT_TRUE(Result);
    EXPECT_TRUE(State.InnerSucceeded);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 66067));
    EXPECT_EQ(Test.Calls.size(), 1U);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Arguments[0].string(), "A");
  }

  // A callback may clear and reprepare the same signature without freeing the outer ffi_call's live interface.
  TEST(NativeCallCacheTest, ReentrantClearKeepsActiveCallPlansAlive)
  {
    NativeCallContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    const ir::Type *ParameterTypes[] = {Pointer, &Test.Int32};
    auto Function = Test.function("recursiveClear", Test.Int32, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    unsigned ResolutionCount = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++ResolutionCount;
      return reinterpret_cast<NativeSymbol>(&inkTestPlannedRecursiveBuffer);
    };
    NativeSymbolCache Symbols(Resolve);
    ReentrantCallState State{Test, Symbols, *Function, false, true};
    ActiveReentrantCall = &State;
    const ExecutionValueRef Arguments[] = {Test.string("A"), Test.integer(1)};
    const auto Result = Test.Calls.invoke(Test.Heap, Symbols, *Function, Arguments);
    ActiveReentrantCall = nullptr;
    ASSERT_TRUE(Result);
    EXPECT_TRUE(State.InnerSucceeded);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 66067));
    EXPECT_EQ(ResolutionCount, 2U);
    EXPECT_EQ(Test.Calls.size(), 1U);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
  }
} // namespace ink::execution::test
