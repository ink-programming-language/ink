#include "ink/execution/ffi/external_function.h"
#include "ink/execution/ffi/ffi_argument.h"
#include "ink/execution/ffi/native_symbol_cache.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#if defined(_WIN32) || defined(__linux__)
namespace ink::execution::test
{
  namespace
  {
    extern "C" std::int32_t inkTestNativeStorageWrite(std::int32_t *Value, std::int32_t Replacement) noexcept
    {
      const std::int32_t Previous = *Value;
      *Value = Replacement;
      return Previous;
    }

    extern "C" std::int32_t inkTestNativeStorageRead(const std::int32_t *Value) noexcept
    {
      return *Value;
    }

    extern "C" bool inkTestNativeStorageAlias(std::int32_t *First, std::int32_t *Second) noexcept
    {
      *First = 41;
      *Second += 1;
      return First == Second && *First == 42;
    }

    extern "C" std::int32_t *inkTestNativeStorageIdentity(std::int32_t *Value) noexcept
    {
      return Value;
    }

    extern "C" bool inkTestNativeStorageIntegers(std::int8_t *I8, std::uint8_t *U8, std::int16_t *I16, std::uint16_t *U16, std::int32_t *I32, std::uint32_t *U32, std::int64_t *I64, std::uint64_t *U64) noexcept
    {
      const bool InitialValuesMatch = *I8 == -7 && *U8 == 5 && *I16 == -7 && *U16 == 5 && *I32 == -7 && *U32 == 5 && *I64 == -7 && *U64 == 5;
      *I8 = -101;
      *U8 = 231;
      *I16 = -30123;
      *U16 = 60123;
      *I32 = -2000000011;
      *U32 = 4000000011U;
      *I64 = -9000000000000000007LL;
      *U64 = 18000000000000000007ULL;
      return InitialValuesMatch;
    }

    extern "C" bool inkTestNativeStorageFloatingAndBool(float *F32, double *F64, bool *Flag) noexcept
    {
      const bool InitialValuesMatch = *F32 == 1.5F && *F64 == -2.25 && !*Flag;
      *F32 = -12.5F;
      *F64 = 123.75;
      *Flag = true;
      return InitialValuesMatch;
    }

    class NativeStorageContext final
    {
      public:
        explicit NativeStorageContext(core::TargetContext Target = core::TargetContext::native())
            : Compilation(Target),
              Context(Compilation),
              Builder(Context),
              Heap(Context),
              Int32(integerType(32, true)),
              Int32Pointer(pointerType(Int32))
        {
        }

        const ir::IntegerType &integerType(std::uint32_t Width, bool Signed)
        {
          return *Context.typePool().getType<ir::TypeKind::Integer>(Width, Signed);
        }

        const ir::PointerType &pointerType(const ir::Type &Pointee, ir::AccessKind Access = ir::AccessKind::ReadWrite)
        {
          return *Context.typePool().getType<ir::TypeKind::Pointer>(Pointee, Access);
        }

        ExecutionValueRef integer(const ir::IntegerType &Type, std::uint64_t Bits)
        {
          if (Type.bitWidth() < 64)
          {
            Bits &= (std::uint64_t{1} << Type.bitWidth()) - 1;
          }
          return Heap.integer(Type, ExecutionInteger(Type.bitWidth(), Bits));
        }

        ExecutionValueRef pointer(const ir::PointerType &Type, ExecutionPlace Place)
        {
          return Heap.pointer(Type, ExecutionPointer::fromPlace(Place));
        }

        std::unique_ptr<ir::Function> function(const ir::Type &Result, std::span<const ir::Type *const> Parameters)
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(Result, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern("inkTestNativeStorage"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, ir::FunctionBinding::Import) : nullptr;
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionHeap Heap;
        const ir::IntegerType &Int32;
        const ir::PointerType &Int32Pointer;
    };
  } // namespace

  // A native write changes the next Ink load while earlier immutable load results retain their original values.
  TEST(ExecutionNativeStorageTest, NativeWritesUpdateCellWithoutChangingEarlierSnapshots)
  {
    NativeStorageContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(Test.Int32, 7));
    ASSERT_TRUE(Cell);
    const auto Before = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Before);
    const ir::Type *Parameters[] = {&Test.Int32Pointer, &Test.Int32};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageWrite);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(Test.Int32Pointer, Cell.Place), Test.integer(Test.Int32, 42)};
    ASSERT_TRUE(Arguments[0]);
    ASSERT_NE(Arguments[0].pointer().address(), nullptr);
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 7));
    const auto After = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(After);
    EXPECT_EQ(After.Value.integer().bits(), ir::IntegerBits(32, 42));
    EXPECT_EQ(Before.Value.integer().bits(), ir::IntegerBits(32, 7));
    EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
    EXPECT_EQ(Test.Heap.allocatedStorageCount(), 1U);
  }

  // A C call writes through an interior typed array pointer without changing siblings or earlier snapshots.
  TEST(ExecutionNativeStorageTest, NativeWriteToInteriorArrayElementPreservesOtherElements)
  {
    NativeStorageContext Test;
    const auto &Array = *Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 3);
    const auto Initial = Test.Heap.array(Array, {Test.integer(Test.Int32, 11), Test.integer(Test.Int32, 22), Test.integer(Test.Int32, 33)});
    const auto Cell = Test.Heap.allocateCell(Array, true, Initial);
    ASSERT_TRUE(Cell);
    const auto Before = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Before);
    const ir::Type *Parameters[] = {&Test.Int32Pointer, &Test.Int32};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_NE(Function, nullptr);
    auto Resolve = [](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageWrite);
    };
    NativeSymbolCache Cache(Resolve);
    const auto Pointer = Test.Heap.pointer(Test.Int32Pointer, ExecutionPointer::fromPlace(Cell.Place, sizeof(std::int32_t)));
    const ExecutionValueRef Arguments[] = {Pointer, Test.integer(Test.Int32, 77)};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().lowWord(), 22U);
    const auto After = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(After);
    EXPECT_EQ(After.Value.array()[0].integer().lowWord(), 11U);
    EXPECT_EQ(After.Value.array()[1].integer().lowWord(), 77U);
    EXPECT_EQ(After.Value.array()[2].integer().lowWord(), 33U);
    EXPECT_EQ(Before.Value.array()[1].integer().lowWord(), 22U);
  }

  // Ink stores update the same native address, including signed values observed through a const C pointer.
  TEST(ExecutionNativeStorageTest, InkStoresAreVisibleToNativeReadsAtStableAddress)
  {
    NativeStorageContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(Test.Int32, 11));
    ASSERT_TRUE(Cell);
    const auto &ReadPointer = Test.pointerType(Test.Int32, ir::AccessKind::ReadOnly);
    const ir::Type *Parameters[] = {&ReadPointer};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageRead);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(ReadPointer, Cell.Place)};
    ASSERT_TRUE(Arguments[0]);
    void *const Address = Arguments[0].pointer().address();
    ASSERT_NE(Address, nullptr);
    const auto First = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(First);
    EXPECT_EQ(First.Value.integer().bits(), ir::IntegerBits(32, 11));
    ASSERT_EQ(Test.Heap.store(Cell.Place, Test.integer(Test.Int32, static_cast<std::uint64_t>(-123))), ExecutionStatus::Success);
    EXPECT_EQ(Arguments[0].pointer().address(), Address);
    const auto Second = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Second);
    EXPECT_EQ(Second.Value.integer().bits(), ir::IntegerBits(32, static_cast<std::uint32_t>(-123)));
  }

  // Repeated addresses of one cell preserve C pointer equality and expose each alias's writes immediately.
  TEST(ExecutionNativeStorageTest, RepeatedAddressArgumentsShareNativeIdentityAndWrites)
  {
    NativeStorageContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(Test.Int32, 0));
    ASSERT_TRUE(Cell);
    const auto &Bool = Test.Context.typePool().getType<ir::TypeKind::Bool>();
    const ir::Type *Parameters[] = {&Test.Int32Pointer, &Test.Int32Pointer};
    auto Function = Test.function(Bool, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageAlias);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(Test.Int32Pointer, Cell.Place), Test.pointer(Test.Int32Pointer, Cell.Place)};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_TRUE(Result.Value.boolean());
    const auto Loaded = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 42));
    EXPECT_EQ(Test.Heap.allocatedStorageCount(), 1U);
  }

  // A returned cell address stays managed and expires at the original release boundary even after its slot is reused.
  TEST(ExecutionNativeStorageTest, NativeReturnedCellAddressRetainsManagedLifetime)
  {
    NativeStorageContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(Test.Int32, 9));
    ASSERT_TRUE(Cell);
    const ir::Type *Parameters[] = {&Test.Int32Pointer};
    auto Function = Test.function(Test.Int32Pointer, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageIdentity);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(Test.Int32Pointer, Cell.Place)};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Pointer);
    const auto &Returned = Result.Value.pointer();
    EXPECT_EQ(Returned.kind(), ExecutionPointer::Kind::Place);
    EXPECT_EQ(Returned.place(), Cell.Place);
    EXPECT_EQ(Returned.address(), Arguments[0].pointer().address());
    EXPECT_EQ(Test.Heap.allocatedStorageCount(), 1U);
    ASSERT_EQ(Test.Heap.store(Returned.place(), Test.integer(Test.Int32, 81)), ExecutionStatus::Success);
    const auto Loaded = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 81));
    ASSERT_EQ(Test.Heap.release(Cell.Place), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Returned.status(), ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Returned.address(), nullptr);
    EXPECT_EQ(callExternalFunction(Test.Heap, Cache, *Function, Arguments).Status, ExecutionStatus::ExpiredPlace);
    ASSERT_TRUE(Test.Heap.allocateCell(Test.Int32, true, Test.integer(Test.Int32, 100)));
    EXPECT_EQ(Returned.status(), ExecutionStatus::ExpiredPlace);
  }

  // Every native signed and unsigned integer width shares its exact host representation with C pointer parameters.
  TEST(ExecutionNativeStorageTest, NativeIntegerWidthsRoundTripThroughSharedCells)
  {
    NativeStorageContext Test;
    struct IntegerCase
    {
        std::uint32_t Width;
        bool Signed;
        std::uint64_t Expected;
    };
    constexpr IntegerCase Cases[] = {
        {8, true, static_cast<std::uint8_t>(-101)},
        {8, false, 231},
        {16, true, static_cast<std::uint16_t>(-30123)},
        {16, false, 60123},
        {32, true, static_cast<std::uint32_t>(-2000000011)},
        {32, false, 4000000011ULL},
        {64, true, static_cast<std::uint64_t>(-9000000000000000007LL)},
        {64, false, 18000000000000000007ULL},
    };
    std::vector<const ir::Type *> Parameters;
    std::vector<ExecutionValueRef> Arguments;
    std::vector<ExecutionPlace> Places;
    for (const auto &Case : Cases)
    {
      const auto &Type = Test.integerType(Case.Width, Case.Signed);
      const auto &Pointer = Test.pointerType(Type);
      const auto Initial = Test.integer(Type, Case.Signed ? static_cast<std::uint64_t>(-7) : 5);
      const auto Cell = Test.Heap.allocateCell(Type, true, Initial);
      ASSERT_TRUE(Cell);
      Parameters.push_back(&Pointer);
      Arguments.push_back(Test.pointer(Pointer, Cell.Place));
      Places.push_back(Cell.Place);
    }
    auto Function = Test.function(Test.Context.typePool().getType<ir::TypeKind::Bool>(), Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageIntegers);
                            });
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_TRUE(Result.Value.boolean());
    for (std::size_t Index = 0; Index < Places.size(); ++Index)
    {
      const auto Loaded = Test.Heap.load(Places[Index]);
      ASSERT_TRUE(Loaded);
      EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(Cases[Index].Width, Cases[Index].Expected));
    }
  }

  // Boolean and both native floating widths preserve values when C reads and overwrites their shared cell memory.
  TEST(ExecutionNativeStorageTest, NativeFloatingAndBooleanPointersUpdateCells)
  {
    NativeStorageContext Test;
    const auto &Bool = Test.Context.typePool().getType<ir::TypeKind::Bool>();
    const auto *F32 = Test.Context.typePool().getType<ir::TypeKind::Float>(32);
    const auto *F64 = Test.Context.typePool().getType<ir::TypeKind::Float>(64);
    ASSERT_NE(F32, nullptr);
    ASSERT_NE(F64, nullptr);
    const auto FloatCell = Test.Heap.allocateCell(*F32, true, Test.Heap.floating(*F32, ir::FloatBits(32, std::bit_cast<std::uint32_t>(1.5F))));
    const auto DoubleCell = Test.Heap.allocateCell(*F64, true, Test.Heap.floating(*F64, ir::FloatBits(64, std::bit_cast<std::uint64_t>(-2.25))));
    const auto BoolCell = Test.Heap.allocateCell(Bool, true, Test.Heap.boolean(Bool, false));
    ASSERT_TRUE(FloatCell);
    ASSERT_TRUE(DoubleCell);
    ASSERT_TRUE(BoolCell);
    const auto &FloatPointer = Test.pointerType(*F32);
    const auto &DoublePointer = Test.pointerType(*F64);
    const auto &BoolPointer = Test.pointerType(Bool);
    const ir::Type *Parameters[] = {&FloatPointer, &DoublePointer, &BoolPointer};
    auto Function = Test.function(Bool, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageFloatingAndBool);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(FloatPointer, FloatCell.Place), Test.pointer(DoublePointer, DoubleCell.Place), Test.pointer(BoolPointer, BoolCell.Place)};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_TRUE(Result.Value.boolean());
    const auto LoadedFloat = Test.Heap.load(FloatCell.Place);
    const auto LoadedDouble = Test.Heap.load(DoubleCell.Place);
    const auto LoadedBool = Test.Heap.load(BoolCell.Place);
    ASSERT_TRUE(LoadedFloat);
    ASSERT_TRUE(LoadedDouble);
    ASSERT_TRUE(LoadedBool);
    EXPECT_EQ(LoadedFloat.Value.floating().bits(), std::bit_cast<std::uint32_t>(-12.5F));
    EXPECT_EQ(LoadedDouble.Value.floating().bits(), std::bit_cast<std::uint64_t>(123.75));
    EXPECT_TRUE(LoadedBool.Value.boolean());
  }

  // Readonly cells can be read through const C pointers but cannot be exported as mutable C pointer arguments.
  TEST(ExecutionNativeStorageTest, ReadOnlyCellAllowsNativeReadsAndRejectsWritableArgument)
  {
    NativeStorageContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, false, Test.integer(Test.Int32, 71));
    ASSERT_TRUE(Cell);
    const auto &ReadPointer = Test.pointerType(Test.Int32, ir::AccessKind::ReadOnly);
    const ir::Type *Parameters[] = {&ReadPointer};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageRead);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(ReadPointer, Cell.Place)};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 71));
    FfiArgument Argument;
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32Pointer, Test.pointer(Test.Int32Pointer, Cell.Place)), ExecutionStatus::ReadOnly);
    EXPECT_EQ(Argument.address(), nullptr);
    const auto Loaded = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 71));
  }

  // Uninitialized cells cannot escape to C without an output-parameter contract, regardless of pointer access mode.
  TEST(ExecutionNativeStorageTest, RejectsUninitializedCellArguments)
  {
    NativeStorageContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32);
    ASSERT_TRUE(Cell);
    const auto &ReadPointer = Test.pointerType(Test.Int32, ir::AccessKind::ReadOnly);
    FfiArgument Argument;
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32Pointer, Test.pointer(Test.Int32Pointer, Cell.Place)), ExecutionStatus::Uninitialized);
    EXPECT_EQ(Argument.address(), nullptr);
    EXPECT_EQ(Argument.prepare(Test.Heap, ReadPointer, Test.pointer(ReadPointer, Cell.Place)), ExecutionStatus::Uninitialized);
    EXPECT_EQ(Argument.address(), nullptr);
    EXPECT_EQ(Test.Heap.load(Cell.Place).Status, ExecutionStatus::Uninitialized);
  }

  // A pointer borrowed from another heap is rejected even when its IR type belongs to the caller's context.
  TEST(ExecutionNativeStorageTest, RejectsCellArgumentsOwnedByAnotherHeap)
  {
    NativeStorageContext Test;
    ExecutionHeap Other(Test.Context);
    const auto Cell = Other.allocateCell(Test.Int32, true, Other.integer(Test.Int32, ExecutionInteger(32, 19)));
    ASSERT_TRUE(Cell);
    const auto Pointer = Test.pointer(Test.Int32Pointer, Cell.Place);
    ASSERT_TRUE(Pointer);
    FfiArgument Argument;
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32Pointer, Pointer), ExecutionStatus::ForeignContext);
    EXPECT_EQ(Argument.address(), nullptr);
    const auto Loaded = Other.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 19));
  }

  // Wide interpreted integers retain normal Ink values but cannot expose an unsupported native storage layout to C.
  TEST(ExecutionNativeStorageTest, RejectsCellsWithoutSupportedNativeLayout)
  {
    NativeStorageContext Test;
    const auto &Wide = Test.integerType(128, true);
    const auto &PointerType = Test.pointerType(Wide);
    const auto Cell = Test.Heap.allocateCell(Wide, true, Test.integer(Wide, 23));
    ASSERT_TRUE(Cell);
    const auto Pointer = Test.pointer(PointerType, Cell.Place);
    ASSERT_TRUE(Pointer);
    EXPECT_EQ(Pointer.pointer().address(), nullptr);
    FfiArgument Argument;
    EXPECT_EQ(Argument.prepare(Test.Heap, PointerType, Pointer), ExecutionStatus::UnsupportedExternalSignature);
    EXPECT_EQ(Argument.address(), nullptr);
    const auto Loaded = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(128, 23));
  }

  // Described target layouts cannot invoke host code merely because a cell's pointer width matches the host.
  TEST(ExecutionNativeStorageTest, NonNativeTargetCannotPassCellAddressToHostFunction)
  {
    const auto Native = core::TargetContext::native();
    NativeStorageContext Test(core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(Test.Int32, 7));
    ASSERT_TRUE(Cell);
    const ir::Type *Parameters[] = {&Test.Int32Pointer};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageRead);
                            });
    const ExecutionValueRef Arguments[] = {Test.pointer(Test.Int32Pointer, Cell.Place)};
    EXPECT_EQ(callExternalFunction(Test.Heap, Cache, *Function, Arguments).Status, ExecutionStatus::HostAbiMismatch);
  }
} // namespace ink::execution::test
#endif
