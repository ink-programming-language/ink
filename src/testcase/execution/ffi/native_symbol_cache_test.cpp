#include "ink/execution/ffi/native_symbol_cache.h"
#include "ink/execution/memory/execution_heap.h"

#include "ink/execution/ffi/external_function.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#if defined(_WIN32) || defined(__linux__)
extern "C" std::int32_t inkTestExternalZero() noexcept;
#endif

namespace ink::execution::test
{
  namespace
  {
    int firstSymbol() noexcept
    {
      return 1;
    }

    int secondSymbol() noexcept
    {
      return 2;
    }

#if defined(_WIN32) || defined(__linux__)
    std::int32_t NativeInvocations = 0;

    extern "C" std::int32_t inkTestNativeSymbolCacheCounter() noexcept
    {
      return ++NativeInvocations;
    }
#endif
  } // namespace

  // Repeated successful lookups return the original address without calling the resolver again.
  TEST(NativeSymbolCacheTest, ReusesSuccessfulResolution)
  {
    const NativeSymbol Symbol = reinterpret_cast<NativeSymbol>(&firstSymbol);
    unsigned Calls = 0;
    auto Resolve = [&](std::string_view Name) -> NativeSymbol
    {
      ++Calls;
      EXPECT_EQ(Name, "cached");
      return Symbol;
    };
    NativeSymbolCache Cache(Resolve);
    EXPECT_EQ(Cache.find("cached"), Symbol);
    EXPECT_EQ(Cache.find("cached"), Symbol);
    EXPECT_EQ(Cache.find("cached"), Symbol);
    EXPECT_EQ(Calls, 1U);
  }

  // Failed lookups are retried until the symbol becomes available, then the successful result is cached.
  TEST(NativeSymbolCacheTest, RetriesFailuresUntilResolutionSucceeds)
  {
    const NativeSymbol Symbol = reinterpret_cast<NativeSymbol>(&firstSymbol);
    unsigned Calls = 0;
    bool Available = false;
    auto Resolve = [&](std::string_view Name) -> NativeSymbol
    {
      ++Calls;
      EXPECT_EQ(Name, "delayed");
      return Available ? Symbol : nullptr;
    };
    NativeSymbolCache Cache(Resolve);
    EXPECT_EQ(Cache.find("delayed"), nullptr);
    EXPECT_EQ(Cache.find("delayed"), nullptr);
    EXPECT_EQ(Calls, 2U);
    Available = true;
    EXPECT_EQ(Cache.find("delayed"), Symbol);
    EXPECT_EQ(Cache.find("delayed"), Symbol);
    EXPECT_EQ(Calls, 3U);
  }

  // A successful binding stays fixed until clearing the cache permits a fresh resolution.
  TEST(NativeSymbolCacheTest, ClearAllowsSuccessfulBindingToChange)
  {
    const NativeSymbol First = reinterpret_cast<NativeSymbol>(&firstSymbol);
    const NativeSymbol Second = reinterpret_cast<NativeSymbol>(&secondSymbol);
    NativeSymbol Current = First;
    unsigned Calls = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++Calls;
      return Current;
    };
    NativeSymbolCache Cache(Resolve);
    Cache.clear();
    EXPECT_EQ(Cache.find("rebound"), First);
    Current = Second;
    EXPECT_EQ(Cache.find("rebound"), First);
    EXPECT_EQ(Calls, 1U);
    Cache.clear();
    EXPECT_EQ(Cache.find("rebound"), Second);
    EXPECT_EQ(Cache.find("rebound"), Second);
    EXPECT_EQ(Calls, 2U);
  }

  // The cache owns the exact string-view bytes even when the original non-terminated buffer changes.
  TEST(NativeSymbolCacheTest, OwnsNamesPassedAsStringViews)
  {
    const NativeSymbol Symbol = reinterpret_cast<NativeSymbol>(&firstSymbol);
    unsigned Calls = 0;
    auto Resolve = [&](std::string_view Name) -> NativeSymbol
    {
      ++Calls;
      EXPECT_EQ(Name, "cached");
      return Symbol;
    };
    NativeSymbolCache Cache(Resolve);
    std::string Buffer = "cached_suffix";
    EXPECT_EQ(Cache.find(std::string_view(Buffer.data(), 6)), Symbol);
    Buffer[0] = 'X';
    EXPECT_EQ(Cache.find("cached"), Symbol);
    EXPECT_EQ(Calls, 1U);
  }

  // Names differing only in letter case retain distinct successful bindings.
  TEST(NativeSymbolCacheTest, KeepsCaseSensitiveNamesSeparate)
  {
    const NativeSymbol First = reinterpret_cast<NativeSymbol>(&firstSymbol);
    const NativeSymbol Second = reinterpret_cast<NativeSymbol>(&secondSymbol);
    unsigned Calls = 0;
    auto Resolve = [&](std::string_view Name) -> NativeSymbol
    {
      ++Calls;
      return Name == "Symbol" ? First : Second;
    };
    NativeSymbolCache Cache(Resolve);
    EXPECT_EQ(Cache.find("Symbol"), First);
    EXPECT_EQ(Cache.find("symbol"), Second);
    EXPECT_EQ(Cache.find("Symbol"), First);
    EXPECT_EQ(Cache.find("symbol"), Second);
    EXPECT_EQ(Calls, 2U);
  }

  // Separate cache instances resolve the same name independently and preserve their own first binding.
  TEST(NativeSymbolCacheTest, KeepsInstancesIndependent)
  {
    const NativeSymbol First = reinterpret_cast<NativeSymbol>(&firstSymbol);
    const NativeSymbol Second = reinterpret_cast<NativeSymbol>(&secondSymbol);
    NativeSymbol Current = First;
    unsigned Calls = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++Calls;
      return Current;
    };
    NativeSymbolCache FirstCache(Resolve);
    NativeSymbolCache SecondCache(Resolve);
    EXPECT_EQ(FirstCache.find("shared"), First);
    Current = Second;
    EXPECT_EQ(SecondCache.find("shared"), Second);
    EXPECT_EQ(FirstCache.find("shared"), First);
    EXPECT_EQ(SecondCache.find("shared"), Second);
    EXPECT_EQ(Calls, 2U);
  }

  // Empty names and embedded NUL bytes are rejected without reaching even a resolver that always succeeds.
  TEST(NativeSymbolCacheTest, RejectsInvalidNamesBeforeResolution)
  {
    unsigned Calls = 0;
    auto Resolve = [&](std::string_view) -> NativeSymbol
    {
      ++Calls;
      return reinterpret_cast<NativeSymbol>(&firstSymbol);
    };
    NativeSymbolCache Cache(Resolve);
    EXPECT_EQ(Cache.find({}), nullptr);
    EXPECT_EQ(Cache.find(""), nullptr);
    EXPECT_EQ(Cache.find(std::string_view("prefix\0suffix", 13)), nullptr);
    EXPECT_EQ(Cache.find(std::string_view("\0", 1)), nullptr);
    EXPECT_EQ(Calls, 0U);
  }

  // An empty injected resolver reports failure explicitly instead of invoking an empty std::function.
  TEST(NativeSymbolCacheTest, HandlesEmptyResolver)
  {
    NativeSymbolCache Cache(NativeSymbolCache::Resolver{});
    EXPECT_EQ(Cache.find("unavailable"), nullptr);
    Cache.clear();
    EXPECT_EQ(Cache.find("unavailable"), nullptr);
  }

#if defined(_WIN32) || defined(__linux__)
  // The default resolver locates the executable's real exported C function and caches its callable address.
  TEST(NativeSymbolCacheTest, ResolvesExportedFunctionByDefault)
  {
    NativeSymbolCache Cache;
    const NativeSymbol Symbol = Cache.find("inkTestExternalZero");
    ASSERT_NE(Symbol, nullptr);
    EXPECT_EQ(Symbol, reinterpret_cast<NativeSymbol>(&inkTestExternalZero));
    EXPECT_EQ(reinterpret_cast<std::int32_t (*)() noexcept>(Symbol)(), 37);
    EXPECT_EQ(Cache.find("inkTestExternalZero"), Symbol);
  }

  // Repeated external calls reuse symbol resolution while still invoking the native function on every call.
  TEST(NativeSymbolCacheTest, ExternalCallsReuseResolutionAndExecuteEveryTime)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    ir::IRBuilder Builder(Context);
    const auto *Int32 = Context.typePool().getType<ir::TypeKind::Integer>(32, true);
    ASSERT_NE(Int32, nullptr);
    const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(*Int32, std::span<const ir::Type *const>{});
    ASSERT_NE(Signature, nullptr);
    auto Function = Builder.createFunction(Context.namePool().intern("inkTestNativeSymbolCacheCounter"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C);
    ASSERT_NE(Function, nullptr);
    unsigned ResolverCalls = 0;
    auto Resolve = [&](std::string_view Name) -> NativeSymbol
    {
      ++ResolverCalls;
      EXPECT_EQ(Name, "inkTestNativeSymbolCacheCounter");
      return reinterpret_cast<NativeSymbol>(&inkTestNativeSymbolCacheCounter);
    };
    NativeSymbolCache Cache(Resolve);
    NativeInvocations = 0;
    const auto First = callExternalFunction(Heap, Cache, *Function, {});
    const auto Second = callExternalFunction(Heap, Cache, *Function, {});
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    EXPECT_EQ(First.Value.integer().bits(), ir::IntegerBits(32, 1));
    EXPECT_EQ(Second.Value.integer().bits(), ir::IntegerBits(32, 2));
    EXPECT_EQ(ResolverCalls, 1U);
    EXPECT_EQ(NativeInvocations, 2);
  }
#endif
} // namespace ink::execution::test
