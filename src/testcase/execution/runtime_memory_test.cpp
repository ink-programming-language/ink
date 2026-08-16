#include "ink/execution/runtime_value.h"
#include "ink/ir/context.h"
#include "ink/ir/type_layout.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

namespace ink::execution
{
  namespace
  {
    struct RuntimeMemoryTestContext
    {
      core::CompilationContext Compilation;
      ir::IRContext IR{Compilation};
    };

    RuntimeValueRef pointerAtByteOffset(RuntimeValueArena &Values, const ir::Type &PointerType, const RuntimeValue &Slice, const ir::Type &ByteType, std::uint64_t ByteOffset, RuntimeMemoryStatus &Status)
    {
      RuntimeValueRef BasePointer = Values.pointerFromByteSlice(PointerType, Slice);
      if (BasePointer == nullptr)
      {
        Status = RuntimeMemoryStatus::InvalidValue;
        return nullptr;
      }
      return Values.getElementPointer(PointerType, *BasePointer, ByteType, ByteOffset, Status);
    }

    // Verifies that borrowed mutable and const byte slices can be loaded across arenas while only mutable slices permit stores into their shared backing memory.
    TEST(RuntimeMemoryTest, LoadsAndStoresBorrowedSlicesWithConstProtection)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ConstBytePointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &ConstByteSliceType = Context.IR.getType(ir::TypeKind::ConstByteSlice);
      std::array<std::uint8_t, 3> MutableBytes = {4, 5, 6};
      const std::array<std::uint8_t, 3> ConstBytes = {7, 8, 9};
      RuntimeValueArena SourceValues;
      RuntimeValueArena AccessValues;
      const RuntimeValueRef MutableSlice = SourceValues.mutableByteSliceValue(ByteSliceType, MutableBytes.data(), MutableBytes.size());
      const RuntimeValueRef ConstSlice = SourceValues.byteSliceValue(ConstByteSliceType, ConstBytes.data(), ConstBytes.size());
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      ASSERT_NE(MutableSlice, nullptr);
      ASSERT_NE(ConstSlice, nullptr);
      const RuntimeValueRef MutablePointer = pointerAtByteOffset(AccessValues, BytePointerType, *MutableSlice, ByteType, 1, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(MutablePointer, nullptr);
      const RuntimeValueRef ConstPointer = pointerAtByteOffset(AccessValues, ConstBytePointerType, *ConstSlice, ByteType, 2, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(ConstPointer, nullptr);
      RuntimeValueRef LoadedByte = nullptr;
      EXPECT_EQ(AccessValues.loadValue(*MutablePointer, ByteType, LoadedByte), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedByte, nullptr);
      EXPECT_EQ(LoadedByte->integer(), 5u);
      const RuntimeValueRef MutableStoredByte = AccessValues.integerValue(ByteType, 42);
      ASSERT_NE(MutableStoredByte, nullptr);
      EXPECT_EQ(AccessValues.storeValue(*MutablePointer, *MutableStoredByte), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(MutableBytes[1], 42);
      EXPECT_EQ(AccessValues.loadValue(*ConstPointer, ByteType, LoadedByte), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedByte, nullptr);
      EXPECT_EQ(LoadedByte->integer(), 9u);
      const RuntimeValueRef ConstBasePointer = AccessValues.pointerFromByteSlice(ConstBytePointerType, *ConstSlice);
      const RuntimeValueRef ConstStoredByte = AccessValues.integerValue(ByteType, 10);
      ASSERT_NE(ConstBasePointer, nullptr);
      ASSERT_NE(ConstStoredByte, nullptr);
      EXPECT_EQ(AccessValues.storeValue(*ConstBasePointer, *ConstStoredByte), RuntimeMemoryStatus::InvalidValue);
      EXPECT_EQ(ConstBytes[0], 7);
    }

    // Verifies that borrowed slice factories accept an empty null-backed slice but reject null non-empty storage and mismatched slice mutability types.
    TEST(RuntimeMemoryTest, ValidatesBorrowedSliceFactoryInputs)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ConstBytePointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &ConstByteSliceType = Context.IR.getType(ir::TypeKind::ConstByteSlice);
      std::uint8_t Byte = 1;
      RuntimeValueArena Values;
      const RuntimeValueRef EmptyMutableSlice = Values.mutableByteSliceValue(ByteSliceType, nullptr, 0);
      const RuntimeValueRef EmptyConstSlice = Values.byteSliceValue(ConstByteSliceType, nullptr, 0);
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      ASSERT_NE(EmptyMutableSlice, nullptr);
      ASSERT_NE(EmptyConstSlice, nullptr);
      EXPECT_EQ(EmptyMutableSlice->byteLength(), 0u);
      EXPECT_EQ(EmptyConstSlice->byteLength(), 0u);
      const RuntimeValueRef EmptyMutablePointer = pointerAtByteOffset(Values, BytePointerType, *EmptyMutableSlice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(EmptyMutablePointer, nullptr);
      const RuntimeValueRef EmptyConstPointer = pointerAtByteOffset(Values, ConstBytePointerType, *EmptyConstSlice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(EmptyConstPointer, nullptr);
      RuntimeValueRef LoadedByte = nullptr;
      EXPECT_EQ(Values.loadValue(*EmptyMutablePointer, ByteType, LoadedByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedByte, nullptr);
      EXPECT_EQ(Values.loadValue(*EmptyConstPointer, ByteType, LoadedByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedByte, nullptr);
      EXPECT_EQ(Values.mutableByteSliceValue(ByteSliceType, nullptr, 1), nullptr);
      EXPECT_EQ(Values.byteSliceValue(ConstByteSliceType, nullptr, 1), nullptr);
      EXPECT_EQ(Values.mutableByteSliceValue(ConstByteSliceType, &Byte, 1), nullptr);
      EXPECT_EQ(Values.byteSliceValue(ByteSliceType, &Byte, 1), nullptr);
    }

    // Verifies that managed byte slices are zero-initialized and that stores become visible to subsequent checked loads.
    TEST(RuntimeMemoryTest, ZeroInitializesAndMutatesManagedSlices)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = Values.allocateByteSlice(ByteSliceType, 4, 17, Status);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      EXPECT_EQ(Slice->kind(), RuntimeValueKind::ByteSlice);
      EXPECT_EQ(&Slice->type(), &ByteSliceType);
      EXPECT_EQ(Slice->byteLength(), 4u);
      EXPECT_TRUE(Slice->memoryAlive());
      EXPECT_NE(Slice->pointer(), nullptr);
      EXPECT_NE(Slice->mutablePointer(), nullptr);
      for (std::size_t Index = 0; Index < 4; ++Index)
      {
        const RuntimeValueRef Pointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, Index, Status);
        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
        ASSERT_NE(Pointer, nullptr);
        RuntimeValueRef LoadedByte = nullptr;
        EXPECT_EQ(Values.loadValue(*Pointer, ByteType, LoadedByte), RuntimeMemoryStatus::Ok);
        ASSERT_NE(LoadedByte, nullptr);
        EXPECT_EQ(LoadedByte->integer(), 0u) << "index " << Index;
      }

      const RuntimeValueRef LastPointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, 3, Status);
      const RuntimeValueRef StoredByte = Values.integerValue(ByteType, 0xA5);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(LastPointer, nullptr);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_EQ(Values.storeValue(*LastPointer, *StoredByte), RuntimeMemoryStatus::Ok);
      RuntimeValueRef LoadedByte = nullptr;
      EXPECT_EQ(Values.loadValue(*LastPointer, ByteType, LoadedByte), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedByte, nullptr);
      EXPECT_EQ(LoadedByte->integer(), 0xA5u);
    }

    // Verifies that each supported integer memory type selects its target width and round-trips through arbitrary byte-derived pointers without alignment assumptions.
    TEST(RuntimeMemoryTest, LoadsAndStoresTypedScalarsAtByteOffsets)
    {
      RuntimeMemoryTestContext Context;
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);
      const ir::Type &BoolType = Context.IR.getType(ir::TypeKind::Bool);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      std::array<std::uint8_t, 14> Bytes = {};
      RuntimeValueArena Values(Target);
      const RuntimeValueRef Slice = Values.mutableByteSliceValue(ByteSliceType, Bytes.data(), Bytes.size());
      const std::uint64_t NegativeI32 = static_cast<std::uint64_t>(static_cast<std::int64_t>(-1985229329));
      const std::uint64_t PointerSizeValue = 0x0102030405060708ULL;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      ASSERT_NE(Slice, nullptr);
      const RuntimeValueRef BoolPointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef I32Pointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, 1, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef PointerSizePointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, 5, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef BytePointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, Bytes.size() - 1, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef StoredBool = Values.integerValue(BoolType, 1);
      const RuntimeValueRef StoredI32 = Values.integerValue(I32Type, NegativeI32);
      const RuntimeValueRef StoredPointerSize = Values.integerValue(PointerSizeType, PointerSizeValue);
      const RuntimeValueRef StoredByte = Values.integerValue(ByteType, 0xA5);
      ASSERT_NE(BoolPointer, nullptr);
      ASSERT_NE(I32Pointer, nullptr);
      ASSERT_NE(PointerSizePointer, nullptr);
      ASSERT_NE(BytePointer, nullptr);
      ASSERT_NE(StoredBool, nullptr);
      ASSERT_NE(StoredI32, nullptr);
      ASSERT_NE(StoredPointerSize, nullptr);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_EQ(Values.storeValue(*BoolPointer, *StoredBool), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(Values.storeValue(*I32Pointer, *StoredI32), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(Values.storeValue(*PointerSizePointer, *StoredPointerSize), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(Values.storeValue(*BytePointer, *StoredByte), RuntimeMemoryStatus::Ok);
      RuntimeValueRef LoadedValue = nullptr;
      EXPECT_EQ(Values.loadValue(*BoolPointer, BoolType, LoadedValue), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(LoadedValue->integer(), 1u);
      EXPECT_EQ(Values.loadValue(*I32Pointer, I32Type, LoadedValue), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(LoadedValue->integer(), NegativeI32);
      EXPECT_EQ(Values.loadValue(*PointerSizePointer, PointerSizeType, LoadedValue), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(LoadedValue->integer(), PointerSizeValue);
      EXPECT_EQ(Values.loadValue(*BytePointer, ByteType, LoadedValue), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(LoadedValue->integer(), 0xA5u);
    }

    // Verifies that i32 and 64-bit ptrsize use the fixed byte sequence selected by the target byte order for both stores and loads.
    TEST(RuntimeMemoryTest, EncodesAndDecodesTargetByteOrder)
    {
      struct Case
      {
        core::ByteOrder Order;
        std::array<std::uint8_t, 12> ExpectedBytes;
      };
      const Case Cases[] = {
          {core::ByteOrder::LittleEndian, {0x78, 0x56, 0x34, 0x12, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}},
          {core::ByteOrder::BigEndian, {0x12, 0x34, 0x56, 0x78, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}},
      };
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ConstBytePointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &ConstByteSliceType = Context.IR.getType(ir::TypeKind::ConstByteSlice);

      for (const Case &CaseValue : Cases)
      {
        const core::TargetContext Target(core::PointerWidth::Bits64, CaseValue.Order);
        RuntimeValueArena Values(Target);
        std::array<std::uint8_t, 12> StoredBytes = {};
        const std::array<std::uint8_t, 12> FixedBytes = CaseValue.ExpectedBytes;
        const RuntimeValueRef StoredSlice = Values.mutableByteSliceValue(ByteSliceType, StoredBytes.data(), StoredBytes.size());
        const RuntimeValueRef FixedSlice = Values.byteSliceValue(ConstByteSliceType, FixedBytes.data(), FixedBytes.size());
        RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

        ASSERT_NE(StoredSlice, nullptr);
        ASSERT_NE(FixedSlice, nullptr);
        const RuntimeValueRef StoredI32Pointer = pointerAtByteOffset(Values, BytePointerType, *StoredSlice, ByteType, 0, Status);
        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
        const RuntimeValueRef StoredPointerSizePointer = pointerAtByteOffset(Values, BytePointerType, *StoredSlice, ByteType, 4, Status);
        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
        const RuntimeValueRef FixedI32Pointer = pointerAtByteOffset(Values, ConstBytePointerType, *FixedSlice, ByteType, 0, Status);
        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
        const RuntimeValueRef FixedPointerSizePointer = pointerAtByteOffset(Values, ConstBytePointerType, *FixedSlice, ByteType, 4, Status);
        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
        const RuntimeValueRef StoredI32 = Values.integerValue(I32Type, 0x12345678);
        const RuntimeValueRef StoredPointerSize = Values.integerValue(PointerSizeType, 0x0102030405060708ULL);
        ASSERT_NE(StoredI32Pointer, nullptr);
        ASSERT_NE(StoredPointerSizePointer, nullptr);
        ASSERT_NE(FixedI32Pointer, nullptr);
        ASSERT_NE(FixedPointerSizePointer, nullptr);
        ASSERT_NE(StoredI32, nullptr);
        ASSERT_NE(StoredPointerSize, nullptr);
        EXPECT_EQ(Values.storeValue(*StoredI32Pointer, *StoredI32), RuntimeMemoryStatus::Ok);
        EXPECT_EQ(Values.storeValue(*StoredPointerSizePointer, *StoredPointerSize), RuntimeMemoryStatus::Ok);
        EXPECT_EQ(StoredBytes, CaseValue.ExpectedBytes);
        RuntimeValueRef LoadedValue = nullptr;
        EXPECT_EQ(Values.loadValue(*FixedI32Pointer, I32Type, LoadedValue), RuntimeMemoryStatus::Ok);
        ASSERT_NE(LoadedValue, nullptr);
        EXPECT_EQ(LoadedValue->integer(), 0x12345678u);
        EXPECT_EQ(Values.loadValue(*FixedPointerSizePointer, PointerSizeType, LoadedValue), RuntimeMemoryStatus::Ok);
        ASSERT_NE(LoadedValue, nullptr);
        EXPECT_EQ(LoadedValue->integer(), 0x0102030405060708ULL);
      }
    }

    // Verifies that 32-bit and 64-bit targets independently constrain ptrsize values, access widths, and borrowed slice lengths.
    TEST(RuntimeMemoryTest, EnforcesTargetPointerSizeSemantics)
    {
      RuntimeMemoryTestContext Context;
      const core::TargetContext Target32(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian);
      const core::TargetContext BigEndianTarget32(core::PointerWidth::Bits32, core::ByteOrder::BigEndian);
      const core::TargetContext Target64(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ConstBytePointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &ConstByteSliceType = Context.IR.getType(ir::TypeKind::ConstByteSlice);
      RuntimeValueArena Values32(Target32);
      RuntimeValueArena BigEndianValues32(BigEndianTarget32);
      RuntimeValueArena Values64(Target64);
      std::array<std::uint8_t, 4> Bytes = {};
      std::array<std::uint8_t, 4> BigEndianStoredBytes = {};
      const std::array<std::uint8_t, 4> BigEndianFixedBytes = {0x12, 0x34, 0x56, 0x78};
      const RuntimeValueRef Slice = Values32.mutableByteSliceValue(ByteSliceType, Bytes.data(), Bytes.size());
      const RuntimeValueRef BigEndianStoredSlice = BigEndianValues32.mutableByteSliceValue(ByteSliceType, BigEndianStoredBytes.data(), BigEndianStoredBytes.size());
      const RuntimeValueRef BigEndianFixedSlice = BigEndianValues32.byteSliceValue(ConstByteSliceType, BigEndianFixedBytes.data(), BigEndianFixedBytes.size());
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      const std::optional<ir::TypeLayout> PointerSizeLayout32 = ir::computeTypeLayout(PointerSizeType, Target32);
      const std::optional<ir::TypeLayout> PointerSizeLayout64 = ir::computeTypeLayout(PointerSizeType, Target64);
      ASSERT_TRUE(PointerSizeLayout32.has_value());
      ASSERT_TRUE(PointerSizeLayout64.has_value());
      EXPECT_EQ(PointerSizeLayout32->Size, 4u);
      EXPECT_EQ(PointerSizeLayout64->Size, 8u);
      EXPECT_NE(Values32.integerValue(PointerSizeType, std::numeric_limits<std::uint32_t>::max()), nullptr);
      EXPECT_EQ(Values32.integerValue(PointerSizeType, static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) + 1), nullptr);
      EXPECT_NE(Values64.integerValue(PointerSizeType, static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) + 1), nullptr);
      ASSERT_NE(Slice, nullptr);
      ASSERT_NE(BigEndianStoredSlice, nullptr);
      ASSERT_NE(BigEndianFixedSlice, nullptr);
      const RuntimeValueRef Pointer32 = pointerAtByteOffset(Values32, BytePointerType, *Slice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef Pointer64 = pointerAtByteOffset(Values64, BytePointerType, *Slice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef BigEndianStoredPointer = pointerAtByteOffset(BigEndianValues32, BytePointerType, *BigEndianStoredSlice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef BigEndianFixedPointer = pointerAtByteOffset(BigEndianValues32, ConstBytePointerType, *BigEndianFixedSlice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef StoredPointerSize32 = Values32.integerValue(PointerSizeType, 0x12345678);
      ASSERT_NE(Pointer32, nullptr);
      ASSERT_NE(Pointer64, nullptr);
      ASSERT_NE(BigEndianStoredPointer, nullptr);
      ASSERT_NE(BigEndianFixedPointer, nullptr);
      ASSERT_NE(StoredPointerSize32, nullptr);
      EXPECT_EQ(Values32.storeValue(*Pointer32, *StoredPointerSize32), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(Bytes, (std::array<std::uint8_t, 4>{0x78, 0x56, 0x34, 0x12}));
      const std::array<std::uint8_t, 4> StoredBytes = Bytes;
      const RuntimeValueRef Sentinel = Values64.integerValue(ByteType, 99);
      RuntimeValueRef LoadedValue = Sentinel;
      ASSERT_NE(Sentinel, nullptr);
      EXPECT_EQ(Values64.loadValue(*Pointer64, PointerSizeType, LoadedValue), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedValue, Sentinel);
      const RuntimeValueRef StoredPointerSize64 = Values64.integerValue(PointerSizeType, 0x0102030405060708ULL);
      ASSERT_NE(StoredPointerSize64, nullptr);
      EXPECT_EQ(Values64.storeValue(*Pointer64, *StoredPointerSize64), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Bytes, StoredBytes);
      const RuntimeValueRef BigEndianStoredPointerSize = BigEndianValues32.integerValue(PointerSizeType, 0x12345678);
      ASSERT_NE(BigEndianStoredPointerSize, nullptr);
      EXPECT_EQ(BigEndianValues32.storeValue(*BigEndianStoredPointer, *BigEndianStoredPointerSize), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(BigEndianStoredBytes, BigEndianFixedBytes);
      LoadedValue = Sentinel;
      EXPECT_EQ(BigEndianValues32.loadValue(*BigEndianFixedPointer, PointerSizeType, LoadedValue), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(LoadedValue->integer(), 0x12345678u);
      const RuntimeValueRef LargePointer = Values64.getElementPointer(BytePointerType, *Pointer64, ByteType, 0x100000000ULL, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(LargePointer, nullptr);
      LoadedValue = Sentinel;
      EXPECT_EQ(Values64.loadValue(*LargePointer, ByteType, LoadedValue), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedValue, Sentinel);
      const RuntimeValueRef StoredByte = Values64.integerValue(ByteType, 1);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_EQ(Values64.storeValue(*LargePointer, *StoredByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Bytes, StoredBytes);

      if constexpr (sizeof(std::size_t) > sizeof(std::uint32_t))
      {
        std::uint8_t Byte = 0;
        const std::size_t UnrepresentableLength = static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
        EXPECT_EQ(Values32.mutableByteSliceValue(ByteSliceType, &Byte, UnrepresentableLength), nullptr);
        EXPECT_EQ(Values32.byteSliceValue(ConstByteSliceType, &Byte, UnrepresentableLength), nullptr);
        Status = RuntimeMemoryStatus::Ok;
        EXPECT_EQ(Values32.allocateByteSlice(ByteSliceType, UnrepresentableLength, 1, Status), nullptr);
        EXPECT_EQ(Status, RuntimeMemoryStatus::InvalidValue);
      }
    }

    // Verifies that the runtime GEP API composes outer stride and nested field offsets while defensively rejecting non-struct and out-of-range paths.
    TEST(RuntimeMemoryTest, ComputesNestedStructFieldPointerPaths)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &F64Type = Context.IR.getType(ir::TypeKind::F64);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::StructType &InnerType = Context.IR.createStructType("Inner", {&ByteType, &I32Type});
      const ir::StructType &OuterType = Context.IR.createStructType("Outer", {&ByteType, &InnerType, &F64Type});
      std::array<std::uint8_t, 36> Bytes{};
      RuntimeValueArena Values;
      const RuntimeValueRef Slice = Values.mutableByteSliceValue(ByteSliceType, Bytes.data(), Bytes.size());
      ASSERT_NE(Slice, nullptr);
      const RuntimeValueRef BasePointer = Values.pointerFromByteSlice(BytePointerType, *Slice);
      ASSERT_NE(BasePointer, nullptr);
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      const RuntimeValueRef FieldPointer = Values.getElementPointer(BytePointerType, *BasePointer, OuterType, 1, {1, 1}, Status);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(FieldPointer, nullptr);
      EXPECT_EQ(runtimePointerByteOffset(*FieldPointer), 32U);
      EXPECT_EQ(Values.getElementPointer(BytePointerType, *BasePointer, ByteType, 0, {0}, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::InvalidValue);
      EXPECT_EQ(Values.getElementPointer(BytePointerType, *BasePointer, OuterType, 0, {3}, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::InvalidValue);
    }

    // Verifies that synthetic targets reject standalone host pointers while allowing bounded logical pointers derived from byte slices.
    TEST(RuntimeMemoryTest, CreatesLogicalSlicePointersForSyntheticTargets)
    {
      RuntimeMemoryTestContext Context;
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ConstBytePointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      std::uint8_t Byte = 0;
      RuntimeValueArena Values(Target);
      const RuntimeValueRef Slice = Values.mutableByteSliceValue(ByteSliceType, &Byte, 1);

      ASSERT_NE(Slice, nullptr);
      EXPECT_EQ(Values.mutablePointerValue(BytePointerType, &Byte), nullptr);
      EXPECT_EQ(Values.pointerValue(ConstBytePointerType, &Byte), nullptr);
      EXPECT_NE(Values.mutablePointerValue(BytePointerType, nullptr), nullptr);
      EXPECT_NE(Values.pointerValue(ConstBytePointerType, nullptr), nullptr);
      const RuntimeValueRef MutablePointer = Values.pointerFromByteSlice(BytePointerType, *Slice);
      const RuntimeValueRef ConstPointer = Values.pointerFromByteSlice(ConstBytePointerType, *Slice);
      ASSERT_NE(MutablePointer, nullptr);
      ASSERT_NE(ConstPointer, nullptr);
      EXPECT_TRUE(MutablePointer->memoryAlive());
      EXPECT_TRUE(ConstPointer->memoryAlive());
      EXPECT_EQ(MutablePointer->byteLength(), 1U);
      EXPECT_EQ(ConstPointer->byteLength(), 1U);
    }

    // Verifies that typed memory access rejects untracked native pointers instead of dereferencing arbitrary host memory.
    TEST(RuntimeMemoryTest, RejectsTypedAccessThroughUntrackedNativePointer)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      std::uint8_t Byte = 19;
      RuntimeValueArena Values;
      const RuntimeValueRef Pointer = Values.mutablePointerValue(BytePointerType, &Byte);
      const RuntimeValueRef StoredValue = Values.integerValue(ByteType, 42);
      RuntimeValueRef LoadedValue = Values.integerValue(ByteType, 99);

      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(StoredValue, nullptr);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(Values.loadValue(*Pointer, ByteType, LoadedValue), RuntimeMemoryStatus::UntrackedPointer);
      ASSERT_NE(LoadedValue, nullptr);
      EXPECT_EQ(LoadedValue->integer(), 99u);
      EXPECT_EQ(Values.storeValue(*Pointer, *StoredValue), RuntimeMemoryStatus::UntrackedPointer);
      EXPECT_EQ(Byte, 19);
    }

    // Verifies that multi-byte accesses reject truncated or overflowing ranges without changing the output value or partially modifying storage.
    TEST(RuntimeMemoryTest, RejectsInvalidTypedScalarAccessesAtomically)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &BoolType = Context.IR.getType(ir::TypeKind::Bool);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      std::array<std::uint8_t, sizeof(std::uint32_t)> Bytes = {2, 3, 4, 5};
      const std::array<std::uint8_t, sizeof(std::uint32_t)> OriginalBytes = Bytes;
      RuntimeValueArena Values;
      const RuntimeValueRef Slice = Values.mutableByteSliceValue(ByteSliceType, Bytes.data(), Bytes.size());
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      ASSERT_NE(Slice, nullptr);
      const RuntimeValueRef BasePointer = pointerAtByteOffset(Values, PointerType, *Slice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef TruncatedPointer = pointerAtByteOffset(Values, PointerType, *Slice, ByteType, 1, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef MaximumPointer = pointerAtByteOffset(Values, PointerType, *Slice, ByteType, std::numeric_limits<std::size_t>::max(), Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(BasePointer, nullptr);
      ASSERT_NE(TruncatedPointer, nullptr);
      ASSERT_NE(MaximumPointer, nullptr);
      const RuntimeValueRef Sentinel = Values.integerValue(ByteType, 99);
      RuntimeValueRef LoadedValue = Sentinel;
      ASSERT_NE(Sentinel, nullptr);
      EXPECT_EQ(Values.loadValue(*BasePointer, BoolType, LoadedValue), RuntimeMemoryStatus::InvalidRepresentation);
      EXPECT_EQ(LoadedValue, Sentinel);
      EXPECT_EQ(Values.integerValue(BoolType, 2), nullptr);
      EXPECT_EQ(Values.loadValue(*TruncatedPointer, I32Type, LoadedValue), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedValue, Sentinel);
      const RuntimeValueRef StoredI32 = Values.integerValue(I32Type, 37);
      ASSERT_NE(StoredI32, nullptr);
      EXPECT_EQ(Values.storeValue(*TruncatedPointer, *StoredI32), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Values.loadValue(*MaximumPointer, I32Type, LoadedValue), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedValue, Sentinel);
      EXPECT_EQ(Values.storeValue(*MaximumPointer, *StoredI32), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Values.loadValue(*BasePointer, PointerType, LoadedValue), RuntimeMemoryStatus::InvalidValue);
      EXPECT_EQ(LoadedValue, Sentinel);
      EXPECT_EQ(Values.storeValue(*BasePointer, *BasePointer), RuntimeMemoryStatus::InvalidValue);
      EXPECT_EQ(Bytes, OriginalBytes);
    }

    // Verifies that a zero-sized managed slice has a live lifetime and rejects index zero as the first out-of-bounds access.
    TEST(RuntimeMemoryTest, SupportsEmptyManagedSlice)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = Values.allocateByteSlice(ByteSliceType, 0, 18, Status);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      EXPECT_EQ(Slice->byteLength(), 0u);
      EXPECT_TRUE(Slice->memoryAlive());
      const RuntimeValueRef Pointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, 0, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Pointer, nullptr);
      const RuntimeValueRef Sentinel = Values.integerValue(ByteType, 7);
      RuntimeValueRef LoadedByte = Sentinel;
      ASSERT_NE(Sentinel, nullptr);
      EXPECT_EQ(Values.loadValue(*Pointer, ByteType, LoadedByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedByte, Sentinel);
      const RuntimeValueRef StoredByte = Values.integerValue(ByteType, 1);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_EQ(Values.storeValue(*Pointer, *StoredByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Values.endByteSliceLifetime(*Slice, 18), RuntimeMemoryStatus::Ok);
      EXPECT_FALSE(Slice->memoryAlive());
    }

    // Verifies that checked load and store accept a live managed slice from another arena while observing its source-owned lifetime state.
    TEST(RuntimeMemoryTest, AccessesForeignManagedSliceWithoutClaimingOwnership)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena SourceValues;
      RuntimeValueArena AccessValues;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = SourceValues.allocateByteSlice(ByteSliceType, 2, 19, Status);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      const RuntimeValueRef Pointer = pointerAtByteOffset(AccessValues, BytePointerType, *Slice, ByteType, 1, Status);
      const RuntimeValueRef StoredByte = AccessValues.integerValue(ByteType, 42);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_EQ(AccessValues.storeValue(*Pointer, *StoredByte), RuntimeMemoryStatus::Ok);
      RuntimeValueRef LoadedByte = nullptr;
      EXPECT_EQ(AccessValues.loadValue(*Pointer, ByteType, LoadedByte), RuntimeMemoryStatus::Ok);
      ASSERT_NE(LoadedByte, nullptr);
      EXPECT_EQ(LoadedByte->integer(), 42u);
      EXPECT_EQ(AccessValues.endByteSliceLifetime(*Slice, 19), RuntimeMemoryStatus::NotOwned);
      EXPECT_EQ(SourceValues.endByteSliceLifetime(*Slice, 19), RuntimeMemoryStatus::Ok);
      LoadedByte = nullptr;
      EXPECT_EQ(AccessValues.loadValue(*Pointer, ByteType, LoadedByte), RuntimeMemoryStatus::LifetimeEnded);
      EXPECT_EQ(LoadedByte, nullptr);
    }

    // Verifies that a foreign arena can safely retain allocation provenance after the source arena is destroyed and observes the ended lifetime without a dangling sidecar.
    TEST(RuntimeMemoryTest, RetainsForeignPointerProvenanceAfterSourceArenaDestruction)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena AccessValues;
      RuntimeValueRef Pointer = nullptr;

      {
        RuntimeValueArena SourceValues;
        RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
        const RuntimeValueRef Slice = SourceValues.allocateByteSlice(ByteSliceType, 1, 20, Status);

        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
        ASSERT_NE(Slice, nullptr);
        Pointer = AccessValues.pointerFromByteSlice(BytePointerType, *Slice);
        ASSERT_NE(Pointer, nullptr);
        EXPECT_TRUE(Pointer->memoryAlive());
      }

      ASSERT_NE(Pointer, nullptr);
      EXPECT_FALSE(Pointer->memoryAlive());
      RuntimeValueRef LoadedValue = nullptr;
      EXPECT_EQ(AccessValues.loadValue(*Pointer, ByteType, LoadedValue), RuntimeMemoryStatus::LifetimeEnded);
      EXPECT_EQ(LoadedValue, nullptr);
    }

    // Verifies that byte loads and stores reject the first index outside a slice as well as larger indexes without changing valid elements.
    TEST(RuntimeMemoryTest, RejectsOutOfBoundsLoadsAndStores)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      std::array<std::uint8_t, 2> Bytes = {11, 12};
      RuntimeValueArena Values;
      const RuntimeValueRef Slice = Values.mutableByteSliceValue(ByteSliceType, Bytes.data(), Bytes.size());
      const RuntimeValueRef Integer = Values.integerValue(ByteType, 0);
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      ASSERT_NE(Slice, nullptr);
      ASSERT_NE(Integer, nullptr);
      const RuntimeValueRef OnePastPointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, Bytes.size(), Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef FarPointer = pointerAtByteOffset(Values, BytePointerType, *Slice, ByteType, Bytes.size() + 100, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(OnePastPointer, nullptr);
      ASSERT_NE(FarPointer, nullptr);
      const RuntimeValueRef Sentinel = Values.integerValue(ByteType, 99);
      RuntimeValueRef LoadedByte = Sentinel;
      ASSERT_NE(Sentinel, nullptr);
      EXPECT_EQ(Values.loadValue(*OnePastPointer, ByteType, LoadedByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedByte, Sentinel);
      EXPECT_EQ(Values.loadValue(*FarPointer, ByteType, LoadedByte), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(LoadedByte, Sentinel);
      const RuntimeValueRef StoredByte21 = Values.integerValue(ByteType, 21);
      const RuntimeValueRef StoredByte22 = Values.integerValue(ByteType, 22);
      ASSERT_NE(StoredByte21, nullptr);
      ASSERT_NE(StoredByte22, nullptr);
      EXPECT_EQ(Values.storeValue(*OnePastPointer, *StoredByte21), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Values.storeValue(*FarPointer, *StoredByte22), RuntimeMemoryStatus::OutOfBounds);
      EXPECT_EQ(Bytes[0], 11);
      EXPECT_EQ(Bytes[1], 12);
      EXPECT_EQ(Values.loadValue(*Integer, ByteType, LoadedByte), RuntimeMemoryStatus::InvalidValue);
      EXPECT_EQ(LoadedByte, Sentinel);
      EXPECT_EQ(Values.storeValue(*Integer, *StoredByte21), RuntimeMemoryStatus::InvalidValue);
    }

    // Verifies that lifetime.end requires the allocating arena and owner frame, preserves liveness after rejected attempts, and reports repeated termination.
    TEST(RuntimeMemoryTest, EnforcesManagedSliceOwnershipAndExplicitLifetimeEnd)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      std::uint8_t BorrowedByte = 1;
      RuntimeValueArena Values;
      RuntimeValueArena ForeignValues;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = Values.allocateByteSlice(ByteSliceType, 1, 31, Status);
      const RuntimeValueRef BorrowedSlice = Values.mutableByteSliceValue(ByteSliceType, &BorrowedByte, 1);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      ASSERT_NE(BorrowedSlice, nullptr);
      const RuntimeValueRef Pointer = Values.pointerFromByteSlice(BytePointerType, *Slice);
      const RuntimeValueRef StoredByte = Values.integerValue(ByteType, 2);
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_EQ(Values.endByteSliceLifetime(*BorrowedSlice, 31), RuntimeMemoryStatus::NotOwned);
      EXPECT_EQ(ForeignValues.endByteSliceLifetime(*Slice, 31), RuntimeMemoryStatus::NotOwned);
      EXPECT_EQ(Values.endByteSliceLifetime(*Slice, 32), RuntimeMemoryStatus::NotOwned);
      EXPECT_TRUE(Slice->memoryAlive());
      EXPECT_EQ(Values.endByteSliceLifetime(*Slice, 31), RuntimeMemoryStatus::Ok);
      EXPECT_FALSE(Slice->memoryAlive());
      EXPECT_EQ(Slice->pointer(), nullptr);
      EXPECT_EQ(Slice->mutablePointer(), nullptr);
      EXPECT_EQ(Values.endByteSliceLifetime(*Slice, 31), RuntimeMemoryStatus::LifetimeEnded);
      RuntimeValueRef LoadedByte = nullptr;
      EXPECT_EQ(Values.loadValue(*Pointer, ByteType, LoadedByte), RuntimeMemoryStatus::LifetimeEnded);
      EXPECT_EQ(LoadedByte, nullptr);
      EXPECT_EQ(Values.storeValue(*Pointer, *StoredByte), RuntimeMemoryStatus::LifetimeEnded);
    }

    // Verifies that frame cleanup ends every allocation owned by the selected frame while leaving other frames alive and remaining idempotent.
    TEST(RuntimeMemoryTest, EndsOnlySelectedFrameLifetimes)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef First = Values.allocateByteSlice(ByteSliceType, 1, 41, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef Second = Values.allocateByteSlice(ByteSliceType, 2, 41, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef OtherFrame = Values.allocateByteSlice(ByteSliceType, 1, 42, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);

      Values.endFrameLifetimes(41);

      ASSERT_NE(First, nullptr);
      ASSERT_NE(Second, nullptr);
      ASSERT_NE(OtherFrame, nullptr);
      const RuntimeValueRef OtherFramePointer = Values.pointerFromByteSlice(BytePointerType, *OtherFrame);
      const RuntimeValueRef StoredByte = Values.integerValue(ByteType, 37);
      ASSERT_NE(OtherFramePointer, nullptr);
      ASSERT_NE(StoredByte, nullptr);
      EXPECT_FALSE(First->memoryAlive());
      EXPECT_FALSE(Second->memoryAlive());
      EXPECT_TRUE(OtherFrame->memoryAlive());
      EXPECT_EQ(Values.endByteSliceLifetime(*First, 41), RuntimeMemoryStatus::LifetimeEnded);
      EXPECT_EQ(Values.storeValue(*OtherFramePointer, *StoredByte), RuntimeMemoryStatus::Ok);
      Values.endFrameLifetimes(41);
      EXPECT_TRUE(OtherFrame->memoryAlive());
      Values.endFrameLifetimes(42);
      EXPECT_FALSE(OtherFrame->memoryAlive());
    }

    // Verifies that cloning borrowed and managed slices preserves shared backing storage, constness, lifetime state, and managed ownership.
    TEST(RuntimeMemoryTest, ClonesSlicesWithSharedBackingAndConstProtection)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ConstBytePointerType = Context.IR.getType(ir::TypeKind::ConstBytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &ConstByteSliceType = Context.IR.getType(ir::TypeKind::ConstByteSlice);
      std::array<std::uint8_t, 2> MutableBytes = {1, 2};
      const std::array<std::uint8_t, 2> ConstBytes = {3, 4};
      RuntimeValueArena SourceValues;
      RuntimeValueArena ClonedValues;
      const RuntimeValueRef MutableSource = SourceValues.mutableByteSliceValue(ByteSliceType, MutableBytes.data(), MutableBytes.size());
      const RuntimeValueRef ConstSource = SourceValues.byteSliceValue(ConstByteSliceType, ConstBytes.data(), ConstBytes.size());
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef ManagedSource = SourceValues.allocateByteSlice(ByteSliceType, 1, 51, Status);

      ASSERT_NE(MutableSource, nullptr);
      ASSERT_NE(ConstSource, nullptr);
      ASSERT_NE(ManagedSource, nullptr);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      const RuntimeValueRef MutableClone = ClonedValues.clone(*MutableSource);
      const RuntimeValueRef ConstClone = ClonedValues.clone(*ConstSource);
      const RuntimeValueRef ManagedClone = ClonedValues.clone(*ManagedSource);
      const RuntimeValueRef ManagedAlias = SourceValues.clone(*ManagedSource);
      ASSERT_NE(MutableClone, nullptr);
      ASSERT_NE(ConstClone, nullptr);
      ASSERT_NE(ManagedClone, nullptr);
      ASSERT_NE(ManagedAlias, nullptr);
      EXPECT_TRUE(ClonedValues.owns(MutableClone));
      EXPECT_TRUE(ClonedValues.owns(ConstClone));
      EXPECT_NE(MutableClone, MutableSource);
      EXPECT_NE(ConstClone, ConstSource);
      EXPECT_EQ(MutableClone->pointer(), MutableSource->pointer());
      EXPECT_EQ(ConstClone->pointer(), ConstSource->pointer());
      EXPECT_EQ(MutableClone->byteLength(), MutableSource->byteLength());
      EXPECT_EQ(ConstClone->byteLength(), ConstSource->byteLength());
      const RuntimeValueRef MutablePointer = ClonedValues.pointerFromByteSlice(BytePointerType, *MutableClone);
      const RuntimeValueRef ConstPointer = ClonedValues.pointerFromByteSlice(ConstBytePointerType, *ConstClone);
      const RuntimeValueRef StoredByte19 = ClonedValues.integerValue(ByteType, 19);
      const RuntimeValueRef StoredByte20 = ClonedValues.integerValue(ByteType, 20);
      ASSERT_NE(MutablePointer, nullptr);
      ASSERT_NE(ConstPointer, nullptr);
      ASSERT_NE(StoredByte19, nullptr);
      ASSERT_NE(StoredByte20, nullptr);
      EXPECT_EQ(ClonedValues.storeValue(*MutablePointer, *StoredByte19), RuntimeMemoryStatus::Ok);
      EXPECT_EQ(MutableBytes[0], 19);
      EXPECT_EQ(ClonedValues.storeValue(*ConstPointer, *StoredByte20), RuntimeMemoryStatus::InvalidValue);
      EXPECT_EQ(ConstBytes[0], 3);
      EXPECT_TRUE(ManagedClone->memoryAlive());
      EXPECT_TRUE(ManagedAlias->memoryAlive());
      EXPECT_EQ(ManagedClone->pointer(), ManagedSource->pointer());
      EXPECT_EQ(ManagedAlias->pointer(), ManagedSource->pointer());
      EXPECT_EQ(ClonedValues.endByteSliceLifetime(*ManagedClone, 51), RuntimeMemoryStatus::NotOwned);
      EXPECT_EQ(SourceValues.endByteSliceLifetime(*ManagedSource, 51), RuntimeMemoryStatus::Ok);
      EXPECT_FALSE(ManagedClone->memoryAlive());
      EXPECT_FALSE(ManagedAlias->memoryAlive());
    }

    // Verifies that cloning an aggregate of borrowed logical pointers retains shared provenance, byte offsets, and repeated pointer aliases across arenas.
    TEST(RuntimeMemoryTest, PreservesBorrowedPointerProvenanceAndAliasesThroughAggregateClone)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::StructType &PointerBundleType = Context.IR.createStructType("PointerBundle", {&BytePointerType, &BytePointerType, &BytePointerType});
      std::array<std::uint8_t, 3> Bytes = {1, 2, 3};
      RuntimeValueArena SourceValues;
      RuntimeValueArena ClonedValues;
      const RuntimeValueRef Slice = SourceValues.mutableByteSliceValue(ByteSliceType, Bytes.data(), Bytes.size());
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;

      ASSERT_NE(Slice, nullptr);
      const RuntimeValueRef BasePointer = SourceValues.pointerFromByteSlice(BytePointerType, *Slice);
      ASSERT_NE(BasePointer, nullptr);
      const RuntimeValueRef OffsetPointer = SourceValues.getElementPointer(BytePointerType, *BasePointer, ByteType, 1, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(OffsetPointer, nullptr);
      const RuntimeValueRef Bundle = SourceValues.aggregateValue(PointerBundleType, {BasePointer, OffsetPointer, OffsetPointer});
      ASSERT_NE(Bundle, nullptr);
      const RuntimeValueRef ClonedBundle = ClonedValues.clone(*Bundle);
      ASSERT_NE(ClonedBundle, nullptr);
      const RuntimeValue *ClonedBasePointer = ClonedBundle->field(0);
      const RuntimeValue *ClonedOffsetPointer = ClonedBundle->field(1);
      const RuntimeValue *ClonedOffsetAlias = ClonedBundle->field(2);
      ASSERT_NE(ClonedBasePointer, nullptr);
      ASSERT_NE(ClonedOffsetPointer, nullptr);
      ASSERT_NE(ClonedOffsetAlias, nullptr);
      EXPECT_EQ(ClonedOffsetPointer, ClonedOffsetAlias);
      const std::optional<bool> BaseEquivalent = runtimePointersEqual(*BasePointer, *ClonedBasePointer);
      const std::optional<bool> OffsetEquivalent = runtimePointersEqual(*OffsetPointer, *ClonedOffsetPointer);
      ASSERT_TRUE(BaseEquivalent.has_value());
      ASSERT_TRUE(OffsetEquivalent.has_value());
      EXPECT_TRUE(*BaseEquivalent);
      EXPECT_TRUE(*OffsetEquivalent);
      const RuntimeValueRef RecomputedOffset = ClonedValues.getElementPointer(BytePointerType, *ClonedBasePointer, ByteType, 1, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(RecomputedOffset, nullptr);
      const std::optional<bool> RecomputedEquivalent = runtimePointersEqual(*RecomputedOffset, *ClonedOffsetPointer);
      ASSERT_TRUE(RecomputedEquivalent.has_value());
      EXPECT_TRUE(*RecomputedEquivalent);
    }

    // Verifies that a pointer derived from managed storage retains its backing lifetime and rejects access after explicit lifetime end.
    TEST(RuntimeMemoryTest, TracksManagedLifetimeThroughDerivedPointer)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = Values.allocateByteSlice(ByteSliceType, 1, 55, Status);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      const void *Address = Slice->pointer();
      const RuntimeValueRef Pointer = Values.pointerFromByteSlice(BytePointerType, *Slice);
      ASSERT_NE(Pointer, nullptr);
      EXPECT_TRUE(Pointer->memoryAlive());
      EXPECT_EQ(Pointer->pointer(), Address);
      EXPECT_EQ(Values.endByteSliceLifetime(*Slice, 55), RuntimeMemoryStatus::Ok);
      EXPECT_FALSE(Pointer->memoryAlive());
      EXPECT_EQ(Pointer->pointer(), nullptr);
      EXPECT_EQ(Pointer->mutablePointer(), nullptr);
      RuntimeValueRef Loaded = nullptr;
      EXPECT_EQ(Values.loadValue(*Pointer, Context.IR.getType(ir::TypeKind::Byte), Loaded), RuntimeMemoryStatus::LifetimeEnded);
      EXPECT_EQ(Loaded, nullptr);
    }

    // Verifies that a logical pointer derived in another arena continues to observe the source allocation's lifetime sidecar.
    TEST(RuntimeMemoryTest, TracksForeignManagedLifetimeThroughDerivedPointer)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      RuntimeValueArena SourceValues;
      RuntimeValueArena DestinationValues;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = SourceValues.allocateByteSlice(ByteSliceType, 1, 56, Status);

      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      const void *Address = Slice->pointer();
      const RuntimeValueRef Pointer = DestinationValues.pointerFromByteSlice(BytePointerType, *Slice);
      ASSERT_NE(Pointer, nullptr);
      EXPECT_EQ(Pointer->pointer(), Address);
      EXPECT_EQ(SourceValues.endByteSliceLifetime(*Slice, 56), RuntimeMemoryStatus::Ok);
      EXPECT_FALSE(Pointer->memoryAlive());
      EXPECT_EQ(Pointer->pointer(), nullptr);
      RuntimeValueRef Loaded = nullptr;
      EXPECT_EQ(DestinationValues.loadValue(*Pointer, Context.IR.getType(ir::TypeKind::Byte), Loaded), RuntimeMemoryStatus::LifetimeEnded);
      EXPECT_EQ(Loaded, nullptr);
    }

    // Verifies that managed allocation accepts the exact per-allocation byte limit and reports the dedicated status immediately above it.
    TEST(RuntimeMemoryTest, EnforcesPerAllocationSizeLimit)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::Type &ConstByteSliceType = Context.IR.getType(ir::TypeKind::ConstByteSlice);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;

      EXPECT_EQ(Values.allocateByteSlice(ConstByteSliceType, 1, 61, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::InvalidValue);
      const RuntimeValueRef MaximumSlice = Values.allocateByteSlice(ByteSliceType, MaximumRuntimeByteAllocationSize, 61, Status);
      ASSERT_NE(MaximumSlice, nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::Ok);
      EXPECT_EQ(MaximumSlice->byteLength(), MaximumRuntimeByteAllocationSize);
      EXPECT_EQ(Values.allocateByteSlice(ByteSliceType, MaximumRuntimeByteAllocationSize + 1, 61, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::AllocationSizeLimitExceeded);
    }

    // Verifies that managed allocations may fill the exact cumulative storage budget and report storage exhaustion for the next non-empty allocation.
    TEST(RuntimeMemoryTest, EnforcesCumulativeStorageLimit)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      constexpr std::size_t AllocationCount = MaximumRuntimeByteStorage / MaximumRuntimeByteAllocationSize;
      static_assert(MaximumRuntimeByteStorage % MaximumRuntimeByteAllocationSize == 0);

      for (std::size_t AllocationIndex = 0; AllocationIndex < AllocationCount; ++AllocationIndex)
      {
        const RuntimeValueRef Slice = Values.allocateByteSlice(ByteSliceType, MaximumRuntimeByteAllocationSize, 71, Status);
        ASSERT_NE(Slice, nullptr) << "allocation " << AllocationIndex;
        ASSERT_EQ(Status, RuntimeMemoryStatus::Ok) << "allocation " << AllocationIndex;
      }
      EXPECT_NE(Values.allocateByteSlice(ByteSliceType, 0, 71, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::Ok);
      EXPECT_EQ(Values.allocateByteSlice(ByteSliceType, 1, 71, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::StorageLimitExceeded);
    }

    // Verifies that zero-sized managed allocations consume allocation slots and that the first allocation beyond the exact count limit receives its dedicated status.
    TEST(RuntimeMemoryTest, EnforcesAllocationCountLimit)
    {
      RuntimeMemoryTestContext Context;
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      RuntimeValueArena Values;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      std::size_t FirstFailedAllocation = MaximumRuntimeByteAllocationCount;

      for (std::size_t AllocationIndex = 0; AllocationIndex < MaximumRuntimeByteAllocationCount; ++AllocationIndex)
      {
        if (Values.allocateByteSlice(ByteSliceType, 0, 81, Status) == nullptr || Status != RuntimeMemoryStatus::Ok)
        {
          FirstFailedAllocation = AllocationIndex;
          break;
        }
      }

      EXPECT_EQ(FirstFailedAllocation, MaximumRuntimeByteAllocationCount);
      EXPECT_EQ(Values.allocateByteSlice(ByteSliceType, 0, 81, Status), nullptr);
      EXPECT_EQ(Status, RuntimeMemoryStatus::AllocationCountLimitExceeded);
    }
  } // namespace
} // namespace ink::execution
