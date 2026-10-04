#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/memory/execution_memory_manager.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>

namespace ink::execution::test
{
  namespace
  {
    class MemoryContext final
    {
      public:
        MemoryContext()
            : Context(Compilation),
              Builder(Context),
              Engine(Context),
              Heap(Engine.heap()),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true)),
              UInt8(*Context.typePool().getType<ir::TypeKind::Integer>(8, false)),
              BytePointer(*Context.typePool().getType<ir::TypeKind::Pointer>(UInt8, ir::AccessKind::ReadWrite))
        {
        }

        ExecutionValueRef integer(std::uint64_t Bits)
        {
          return Heap.integer(Int32, ExecutionInteger(32, Bits));
        }

        std::unique_ptr<ir::Function> byteReader()
        {
          const ir::Type *Parameters[] = {&BytePointer};
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(UInt8, Parameters);
          auto Function = Builder.createFunction(Context.namePool().intern("ReadNativeByte"), *Signature);
          auto *Body = Builder.createFunctionBody(*Function);
          Builder.setInsertPoint(*Body);
          auto *Loaded = Builder.createLoadInstruction(*Function->parameters().front());
          Builder.createReturnInstruction(Loaded);
          return Function;
        }

        std::unique_ptr<ir::Function> byteWriter()
        {
          const auto &Void = Context.typePool().getType<ir::TypeKind::Void>();
          const ir::Type *Parameters[] = {&BytePointer, &UInt8};
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(Void, Parameters);
          auto Function = Builder.createFunction(Context.namePool().intern("WriteNativeByte"), *Signature);
          auto *Body = Builder.createFunctionBody(*Function);
          Builder.setInsertPoint(*Body);
          Builder.createStoreInstruction(*Function->parameters()[0], *Function->parameters()[1]);
          Builder.createReturnInstruction();
          return Function;
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        ExecutionHeap &Heap;
        const ir::IntegerType &Int32;
        const ir::IntegerType &UInt8;
        const ir::PointerType &BytePointer;
    };

    template <typename T>
    void checkNativeInteger(MemoryContext &Test, T Initial, T Changed)
    {
      constexpr std::uint32_t Width = sizeof(T) * 8;
      using Unsigned = std::make_unsigned_t<T>;
      const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Integer>(Width, std::is_signed_v<T>);
      ASSERT_NE(Type, nullptr);
      const auto Cell = Test.Heap.allocateCell(*Type, true, Test.Heap.integer(*Type, ExecutionInteger(Width, std::bit_cast<Unsigned>(Initial))));
      ASSERT_TRUE(Cell);
      ASSERT_NE(Cell.Place.storage().cell()->data(), nullptr);
      auto *Address = static_cast<T *>(Cell.Place.storage().cell()->data());
      EXPECT_EQ(reinterpret_cast<std::uintptr_t>(Address) % alignof(T), 0U);
      EXPECT_EQ(Cell.Place.storage().cell()->size(), sizeof(T));
      EXPECT_EQ(*Address, Initial);
      const auto Snapshot = Test.Heap.load(Cell.Place);
      ASSERT_TRUE(Snapshot);
      *Address = Changed;
      const auto Current = Test.Heap.load(Cell.Place);
      ASSERT_TRUE(Current);
      EXPECT_EQ(Current.Value.integer().bits(), ir::IntegerBits(Width, std::bit_cast<Unsigned>(Changed)));
      EXPECT_EQ(Snapshot.Value.integer().bits(), ir::IntegerBits(Width, std::bit_cast<Unsigned>(Initial)));
      ASSERT_EQ(Test.Heap.store(Cell.Place, Snapshot.Value), ExecutionStatus::Success);
      EXPECT_EQ(*Address, Initial);
      EXPECT_EQ(Test.Heap.pointerFromAddress(Address).address(), Address);
    }
  } // namespace

  // Every supported integer width shares correctly aligned native storage and preserves earlier load snapshots.
  TEST(ExecutionMemoryManagerTest, SharesNativeIntegerStorageWithoutChangingSnapshots)
  {
    MemoryContext Test;
    checkNativeInteger<std::int8_t>(Test, -9, std::numeric_limits<std::int8_t>::min());
    checkNativeInteger<std::uint8_t>(Test, 9, std::numeric_limits<std::uint8_t>::max());
    checkNativeInteger<std::int16_t>(Test, -123, std::numeric_limits<std::int16_t>::min());
    checkNativeInteger<std::uint16_t>(Test, 123, std::numeric_limits<std::uint16_t>::max());
    checkNativeInteger<std::int32_t>(Test, -12345, std::numeric_limits<std::int32_t>::min());
    checkNativeInteger<std::uint32_t>(Test, 12345, std::numeric_limits<std::uint32_t>::max());
    checkNativeInteger<std::int64_t>(Test, -1234567, std::numeric_limits<std::int64_t>::min());
    checkNativeInteger<std::uint64_t>(Test, 1234567, std::numeric_limits<std::uint64_t>::max());
  }

  // Float storage keeps negative zero and NaN payload bits across interpreter stores and native writes.
  TEST(ExecutionMemoryManagerTest, PreservesNativeFloatingPointObjectBits)
  {
    MemoryContext Test;
    const auto *Float32 = Test.Context.typePool().getType<ir::TypeKind::Float>(32);
    const auto *Float64 = Test.Context.typePool().getType<ir::TypeKind::Float>(64);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(Float64, nullptr);
    const auto First = Test.Heap.allocateCell(*Float32, true, Test.Heap.floating(*Float32, ir::FloatBits(32, 0x80000000U)));
    const auto Second = Test.Heap.allocateCell(*Float64, true, Test.Heap.floating(*Float64, ir::FloatBits(64, 0x8000000000000000ULL)));
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    auto *Single = static_cast<float *>(First.Place.storage().cell()->data());
    auto *Double = static_cast<double *>(Second.Place.storage().cell()->data());
    ASSERT_NE(Single, nullptr);
    ASSERT_NE(Double, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(Single) % alignof(float), 0U);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(Double) % alignof(double), 0U);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(*Single), 0x80000000U);
    EXPECT_EQ(std::bit_cast<std::uint64_t>(*Double), 0x8000000000000000ULL);
    const std::uint32_t SingleNan = 0x7fc12345U;
    const std::uint64_t DoubleNan = 0x7ff8123456789abcULL;
    std::memcpy(Single, &SingleNan, sizeof(SingleNan));
    std::memcpy(Double, &DoubleNan, sizeof(DoubleNan));
    const auto FirstResult = Test.Heap.load(First.Place);
    const auto SecondResult = Test.Heap.load(Second.Place);
    ASSERT_TRUE(FirstResult);
    ASSERT_TRUE(SecondResult);
    EXPECT_EQ(FirstResult.Value.floating().bits(), SingleNan);
    EXPECT_EQ(SecondResult.Value.floating().bits(), DoubleNan);
  }

  // Byte writes cannot make the interpreter evaluate an invalid native bool representation.
  TEST(ExecutionMemoryManagerTest, RejectsInvalidBooleanBytesAndAcceptsNativeBooleanWrites)
  {
    MemoryContext Test;
    const auto &Bool = Test.Context.typePool().getType<ir::TypeKind::Bool>();
    const auto Cell = Test.Heap.allocateCell(Bool, true, Test.Heap.boolean(Bool, false));
    ASSERT_TRUE(Cell);
    auto *Address = static_cast<bool *>(Cell.Place.storage().cell()->data());
    ASSERT_NE(Address, nullptr);
    *Address = true;
    const auto Loaded = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_TRUE(Loaded.Value.boolean());
    *reinterpret_cast<unsigned char *>(Address) = 2;
    EXPECT_EQ(Test.Heap.load(Cell.Place).Status, ExecutionStatus::TypeMismatch);
    ASSERT_EQ(Test.Heap.store(Cell.Place, Test.Heap.boolean(Bool, false)), ExecutionStatus::Success);
    EXPECT_FALSE(*Address);
  }

  // Native backing never replaces initialization, readonly, runtime or exact-type checks.
  TEST(ExecutionMemoryManagerTest, PreservesCellValidationWithNativeStorage)
  {
    MemoryContext Test;
    const auto Empty = Test.Heap.allocateCell(Test.Int32);
    ASSERT_TRUE(Empty);
    EXPECT_FALSE(Empty.Place.storage().cell()->initialized());
    *static_cast<std::int32_t *>(Empty.Place.storage().cell()->data()) = 55;
    EXPECT_EQ(Test.Heap.load(Empty.Place).Status, ExecutionStatus::Uninitialized);
    const auto Constant = Test.Heap.allocateCell(Test.Int32, false);
    ASSERT_TRUE(Constant);
    ASSERT_EQ(Test.Heap.store(Constant.Place, Test.integer(11)), ExecutionStatus::Success);
    EXPECT_TRUE(Constant.Place.storage().cell()->initialized());
    EXPECT_EQ(Test.Heap.store(Constant.Place, Test.integer(12)), ExecutionStatus::ReadOnly);
    EXPECT_EQ(Test.Heap.store(Empty.Place, Test.Heap.integer(Test.UInt8, ExecutionInteger(8, 1))), ExecutionStatus::TypeMismatch);
    const auto Runtime = Test.Heap.allocateCell(Test.Int32, true, {}, true);
    ASSERT_TRUE(Runtime);
    EXPECT_NE(Runtime.Place.storage().cell()->data(), nullptr);
    EXPECT_EQ(Test.Heap.load(Runtime.Place).Status, ExecutionStatus::RuntimeValue);
    EXPECT_EQ(Test.Heap.store(Runtime.Place, Test.integer(12)), ExecutionStatus::RuntimeValue);
  }

  // Wide integers and strings expose stable byte storage while loaded values remain independent snapshots.
  TEST(ExecutionMemoryManagerTest, StoresWideIntegersAndStringsInContiguousMemory)
  {
    MemoryContext Test;
    const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, true);
    const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(Test.UInt8, ir::AccessKind::ReadOnly);
    ASSERT_NE(Wide, nullptr);
    ASSERT_NE(Slice, nullptr);
    const std::uint64_t Words[] = {0xfedcba9876543210ULL, 0x8123456789abcdefULL};
    const auto WideValue = Test.Heap.integer(*Wide, ExecutionInteger(ir::IntegerBits(128, Words)));
    const auto StringValue = Test.Heap.string(*Slice, std::string_view("A\0B", 3));
    const auto WideCell = Test.Heap.allocateCell(*Wide, true, WideValue);
    const auto StringCell = Test.Heap.allocateCell(*Slice, true, StringValue);
    ASSERT_TRUE(WideCell);
    ASSERT_TRUE(StringCell);
    EXPECT_NE(WideCell.Place.storage().cell()->data(), nullptr);
    EXPECT_NE(StringCell.Place.storage().cell()->data(), nullptr);
    EXPECT_EQ(Test.Heap.load(WideCell.Place).Value.integer().bits(), ir::IntegerBits(128, Words));
    EXPECT_EQ(Test.Heap.load(StringCell.Place).Value.string(), std::string_view("A\0B", 3));
  }

  // Explicit host-shaped targets retain their scalar bits in addressable storage.
  TEST(ExecutionMemoryManagerTest, ExposesScalarStorageForHostShapedTargets)
  {
    const auto Native = core::TargetContext::native();
    core::CompilationContext Compilation(core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    const auto *Int32 = Context.typePool().getType<ir::TypeKind::Integer>(32, true);
    ASSERT_NE(Int32, nullptr);
    const auto Value = Heap.integer(*Int32, ExecutionInteger(32, 17));
    const auto Cell = Heap.allocateCell(*Int32, true, Value);
    ASSERT_TRUE(Cell);
    EXPECT_NE(Cell.Place.storage().cell()->data(), nullptr);
    EXPECT_EQ(ExecutionPointer::fromPlace(Cell.Place).address(), Cell.Place.storage().cell()->data());
    EXPECT_EQ(Heap.load(Cell.Place).Value.integer().bits(), Value.integer().bits());
  }

  // Interior and one-past pointers preserve raw addresses independently of allocation bookkeeping.
  TEST(ExecutionMemoryManagerTest, PreservesRawAddressesWithoutOwningTheirStorage)
  {
    MemoryContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(42));
    ASSERT_TRUE(Cell);
    auto *Start = static_cast<unsigned char *>(Cell.Place.storage().cell()->data());
    const auto Interior = Test.Heap.pointerFromAddress(Start + 1);
    const auto End = Test.Heap.pointerFromAddress(Start + sizeof(std::int32_t));
    EXPECT_EQ(Interior.address(), Start + 1);
    EXPECT_EQ(End.address(), Start + sizeof(std::int32_t));
    ASSERT_EQ(Test.Heap.release(Cell.Place), ExecutionStatus::Success);
    EXPECT_EQ(Interior.address(), Start + 1);
    EXPECT_EQ(End.address(), Start + sizeof(std::int32_t));
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
  }

  // Fixed buffers allocate their requested zero-filled extent only after object and byte budgets succeed.
  TEST(ExecutionMemoryManagerTest, AllocatesZeroFilledBuffersWithinCumulativeBudgets)
  {
    MemoryContext Test;
    const std::size_t Budget = sizeof(ExecutionCell) + sizeof(std::int32_t) + sizeof(ExecutionBuffer) + 5;
    ExecutionHeap Heap(Test.Context, 3, Budget);
    const auto Cell = Heap.allocateCell(Test.Int32);
    const auto Buffer = Heap.allocateBuffer(std::size_t{5});
    ASSERT_TRUE(Cell);
    ASSERT_TRUE(Buffer.valid());
    EXPECT_EQ(Buffer.buffer()->size(), 5U);
    EXPECT_EQ(std::string_view(Buffer.buffer()->data(), 5), std::string_view("\0\0\0\0\0", 5));
    EXPECT_EQ(Heap.liveStorageCount(), 2U);
    EXPECT_EQ(Heap.allocatedStorageBytes(), Budget);
    EXPECT_EQ(Heap.liveStorageBytes(), Budget);
    EXPECT_FALSE(Heap.allocateBuffer(std::size_t{0}).valid());
    EXPECT_EQ(Heap.lastStatus(), ExecutionStatus::BudgetExceeded);
    ASSERT_EQ(Heap.release(Cell.Place), ExecutionStatus::Success);
    ASSERT_EQ(Heap.release(Buffer), ExecutionStatus::Success);
    EXPECT_EQ(Heap.liveStorageBytes(), 0U);
    EXPECT_EQ(Heap.allocatedStorageBytes(), Budget);
    EXPECT_EQ(Heap.allocatedStorageCount(), 2U);
    EXPECT_FALSE(Heap.allocateBuffer(std::size_t{1}).valid());
    EXPECT_EQ(Heap.lastStatus(), ExecutionStatus::BudgetExceeded);
  }

  // Addressable wide and pointer cells charge their aligned byte backing before allocation, including cumulative limits after release.
  TEST(ExecutionMemoryManagerTest, ChargesWideAndPointerBackingStorage)
  {
    MemoryContext Test;
    const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, true);
    ASSERT_NE(Wide, nullptr);
    const std::size_t Budget = sizeof(ExecutionCell) * 2 + 16 + sizeof(void *);
    ExecutionHeap Heap(Test.Context, 3, Budget);
    const auto WideCell = Heap.allocateCell(*Wide);
    const auto PointerCell = Heap.allocateCell(Test.BytePointer);
    ASSERT_TRUE(WideCell);
    ASSERT_TRUE(PointerCell);
    EXPECT_EQ(Heap.allocatedStorageBytes(), Budget);
    EXPECT_EQ(Heap.liveStorageBytes(), Budget);
    ASSERT_EQ(Heap.release(WideCell.Place), ExecutionStatus::Success);
    ASSERT_EQ(Heap.release(PointerCell.Place), ExecutionStatus::Success);
    EXPECT_EQ(Heap.liveStorageBytes(), 0U);
    EXPECT_EQ(Heap.allocatedStorageBytes(), Budget);
    EXPECT_EQ(Heap.allocateCell(*Wide).Status, ExecutionStatus::BudgetExceeded);
    ExecutionHeap TooSmall(Test.Context, 1, sizeof(ExecutionCell) + 15);
    EXPECT_EQ(TooSmall.allocateCell(*Wide).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(TooSmall.allocatedStorageCount(), 0U);
  }

  // Overflow and failed allocation requests do not consume either counter or construct an oversized temporary buffer.
  TEST(ExecutionMemoryManagerTest, RejectsBufferSizeOverflowBeforeAllocation)
  {
    MemoryContext Test;
    ExecutionMemoryManager Memory(1);
    EXPECT_FALSE(Memory.allocateBuffer(std::numeric_limits<std::size_t>::max()).valid());
    EXPECT_EQ(Memory.lastStatus(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Memory.allocatedStorageCount(), 0U);
    EXPECT_EQ(Memory.allocatedStorageBytes(), 0U);
    const auto Empty = Memory.allocateBuffer(std::size_t{0});
    ASSERT_TRUE(Empty.valid());
    EXPECT_EQ(Empty.buffer()->size(), 0U);
    EXPECT_TRUE(Memory.owns(Empty));
    EXPECT_FALSE(Test.Heap.owns(Empty));
    EXPECT_EQ(Test.Heap.release(Empty), ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Memory.release(Empty), ExecutionStatus::Success);
    EXPECT_FALSE(Memory.allocateBuffer(std::size_t{0}).valid());
    EXPECT_EQ(Memory.lastStatus(), ExecutionStatus::BudgetExceeded);
  }

  // IR byte loads and stores accept valid raw addresses independently of the allocating heap.
  TEST(ExecutionMemoryManagerTest, AcceptsLiveRawBuffersAcrossHeapOwners)
  {
    MemoryContext Test;
    ExecutionHeap Foreign(Test.Context);
    auto Reader = Test.byteReader();
    auto Writer = Test.byteWriter();
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const auto LocalBuffer = Test.Heap.allocateBuffer("L");
    const auto ForeignBuffer = Foreign.allocateBuffer("F");
    ASSERT_TRUE(LocalBuffer.valid());
    ASSERT_TRUE(ForeignBuffer.valid());
    ExecutionValueRef ReadArguments[] = {Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromBuffer(LocalBuffer))};
    const auto ReadLocal = Test.Engine.execute(*Reader, ReadArguments);
    ASSERT_TRUE(ReadLocal);
    EXPECT_EQ(ReadLocal.Value.integer().bits(), ir::IntegerBits(8, 'L'));
    ExecutionValueRef WriteArguments[] = {ReadArguments[0], Test.Heap.integer(Test.UInt8, ExecutionInteger(8, 'W'))};
    ASSERT_TRUE(Test.Engine.execute(*Writer, WriteArguments));
    EXPECT_EQ(LocalBuffer.buffer()->data()[0], 'W');
    ReadArguments[0] = Foreign.pointer(Test.BytePointer, ExecutionPointer::fromBuffer(ForeignBuffer));
    WriteArguments[0] = ReadArguments[0];
    EXPECT_EQ(Test.Engine.execute(*Reader, ReadArguments).Status, ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::Success);
    EXPECT_EQ(ForeignBuffer.buffer()->data()[0], 'W');
  }

  // Byte aliases use the scalar's native object bytes and reject one-past, readonly and uninitialized accesses.
  TEST(ExecutionMemoryManagerTest, ExecutesCheckedByteAliasesIntoScalarStorage)
  {
    MemoryContext Test;
    auto Reader = Test.byteReader();
    auto Writer = Test.byteWriter();
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.integer(0x12345678U));
    ASSERT_TRUE(Cell);
    auto *Bytes = static_cast<unsigned char *>(Cell.Place.storage().cell()->data());
    ExecutionValueRef Arguments[] = {Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromPlace(Cell.Place, 1))};
    const auto Read = Test.Engine.execute(*Reader, Arguments);
    ASSERT_TRUE(Read);
    EXPECT_EQ(Read.Value.integer().bits(), ir::IntegerBits(8, Bytes[1]));
    ExecutionValueRef WriteArguments[] = {Arguments[0], Test.Heap.integer(Test.UInt8, ExecutionInteger(8, 0xa5))};
    ASSERT_TRUE(Test.Engine.execute(*Writer, WriteArguments));
    EXPECT_EQ(Bytes[1], 0xa5);
    std::uint32_t Expected = 0;
    std::memcpy(&Expected, Bytes, sizeof(Expected));
    EXPECT_EQ(Test.Heap.load(Cell.Place).Value.integer().bits(), ir::IntegerBits(32, Expected));
    Arguments[0] = Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromPlace(Cell.Place, sizeof(std::int32_t)));
    EXPECT_EQ(Test.Engine.execute(*Reader, Arguments).Status, ExecutionStatus::InvalidPlace);
    const auto Uninitialized = Test.Heap.allocateCell(Test.Int32);
    ASSERT_TRUE(Uninitialized);
    Arguments[0] = Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromPlace(Uninitialized.Place));
    EXPECT_EQ(Test.Engine.execute(*Reader, Arguments).Status, ExecutionStatus::Uninitialized);
    WriteArguments[0] = Arguments[0];
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::Success);
    const auto Constant = Test.Heap.allocateCell(Test.Int32, false, Test.integer(9));
    ASSERT_TRUE(Constant);
    WriteArguments[0] = Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromPlace(Constant.Place));
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::ReadOnly);
  }
} // namespace ink::execution::test
