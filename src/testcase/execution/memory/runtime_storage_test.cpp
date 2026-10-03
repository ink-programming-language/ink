#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/memory/execution_memory_manager.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string_view>

namespace ink::execution::test
{
  // Lowered layouts, strings and wide integers remain usable after every semantic owner is destroyed.
  TEST(RuntimeStorageTest, RetainsOwnedValuesAndLayoutsAfterSemanticContextDestruction)
  {
    ExecutionMemoryManager Memory;
    std::shared_ptr<RuntimeTypeTable> Types;
    ExecutionPlace IntegerPlace;
    ExecutionPlace WidePlace;
    ExecutionPlace StringPlace;
    RuntimeTypeId IntegerType = InvalidRuntimeType;
    RuntimeTypeId WideType = InvalidRuntimeType;
    RuntimeValue WideSnapshot;
    {
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      SemanticValueBridge Bridge(Context);
      Types = Bridge.types();
      const auto *Integer = Context.typePool().getType<ir::TypeKind::Integer>(32, true);
      const auto *Wide = Context.typePool().getType<ir::TypeKind::Integer>(128, false);
      const auto *Byte = Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto *String = Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
      const std::uint64_t Words[] = {0xfedcba9876543210ULL, 0x8123456789abcdefULL};
      const auto IntegerValue = Bridge.lowerConstant(*Context.constantPool().getIntegerConstant(*Integer, ir::IntegerBits(32, 17)));
      const auto WideValue = Bridge.lowerConstant(*Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, Words)));
      const auto StringValue = Bridge.lowerConstant(*Context.constantPool().getStringConstant(*String, std::string_view("A\0B", 3)));
      ASSERT_TRUE(IntegerValue);
      ASSERT_TRUE(WideValue);
      ASSERT_TRUE(StringValue);
      IntegerType = IntegerValue.Value.Type;
      WideType = WideValue.Value.Type;
      WideSnapshot = WideValue.Value;
      const auto First = Memory.allocateCell(*Types->get(IntegerType), true, IntegerValue.Value);
      const auto Second = Memory.allocateCell(*Types->get(WideType), true, WideValue.Value);
      const auto Third = Memory.allocateCell(*Types->get(StringValue.Value.Type), true, StringValue.Value);
      ASSERT_TRUE(First);
      ASSERT_TRUE(Second);
      ASSERT_TRUE(Third);
      IntegerPlace = First.Place;
      WidePlace = Second.Place;
      StringPlace = Third.Place;
    }
    EXPECT_EQ(Types->get(IntegerType)->BitWidth, 32U);
    EXPECT_EQ(IntegerPlace.storage().cell()->loadRuntime().Value.Bits, 17U);
    EXPECT_EQ(WidePlace.storage().cell()->loadRuntime().Value.integer().bitWidth(), 128U);
    EXPECT_EQ(WidePlace.storage().cell()->loadRuntime().Value.integer().lowWord(), 0xfedcba9876543210ULL);
    EXPECT_EQ(StringPlace.storage().cell()->loadRuntime().Value.string(), std::string_view("A\0B", 3));
    const RuntimeValue Replacement = RuntimeValue::fromInteger(ExecutionInteger(128, 9), WideType);
    ASSERT_EQ(WidePlace.storage().cell()->storeRuntime(Replacement), ExecutionStatus::Success);
    EXPECT_EQ(WideSnapshot.integer().lowWord(), 0xfedcba9876543210ULL);
    EXPECT_EQ(WidePlace.storage().cell()->loadRuntime().Value.integer().lowWord(), 9U);
  }

  // Type identity survives layout lowering: equally sized bool and u8 storage cannot be interchanged.
  TEST(RuntimeStorageTest, RejectsEqualSizedValuesWithDifferentTypeIdentities)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    SemanticValueBridge Bridge(Context);
    ExecutionMemoryManager Memory;
    const auto Bool = Bridge.lowerType(Context.typePool().getType<ir::TypeKind::Bool>());
    const auto Byte = Bridge.lowerType(*Context.typePool().getType<ir::TypeKind::Integer>(8, false));
    ASSERT_EQ(Bridge.types()->get(Bool)->Size, Bridge.types()->get(Byte)->Size);
    const auto Cell = Memory.allocateCell(*Bridge.types()->get(Bool), true, RuntimeValue::fromBits(1, Bool));
    ASSERT_TRUE(Cell);
    EXPECT_EQ(Cell.Place.storage().cell()->storeRuntime(RuntimeValue::fromBits(0, Byte)), ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Cell.Place.storage().cell()->loadRuntime().Value.Bits, 1U);
    const auto Rejected = Memory.allocateCell(*Bridge.types()->get(Bool), true, RuntimeValue::fromBits(2, Bool));
    EXPECT_EQ(Rejected.Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Memory.allocatedStorageCount(), 1U);
  }

  // Eliminated local allocations consume cumulative budgets without creating live managed objects.
  TEST(RuntimeStorageTest, ChargesFrameLocalsWithoutManagedAllocation)
  {
    ExecutionMemoryManager Memory(2, sizeof(ExecutionCell) * 2);
    ASSERT_EQ(Memory.chargeLocalAllocation(), ExecutionStatus::Success);
    EXPECT_EQ(Memory.liveStorageCount(), 0U);
    EXPECT_EQ(Memory.liveStorageBytes(), 0U);
    EXPECT_EQ(Memory.allocatedStorageCount(), 1U);
    EXPECT_EQ(Memory.allocatedStorageBytes(), sizeof(ExecutionCell));
    ASSERT_EQ(Memory.chargeLocalAllocation(), ExecutionStatus::Success);
    EXPECT_EQ(Memory.chargeLocalAllocation(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Memory.allocatedStorageCount(), 2U);
  }

  // Runtime pointer validation distinguishes a foreign allocation from a released local generation.
  TEST(RuntimeStorageTest, ValidatesPointerOwnershipAndGenerationWithoutIR)
  {
    ExecutionMemoryManager Memory;
    ExecutionMemoryManager Foreign;
    const auto Storage = Memory.allocateBuffer("abc");
    const auto Other = Foreign.allocateBuffer("xyz");
    ASSERT_TRUE(Storage.valid());
    ASSERT_TRUE(Other.valid());
    const auto Pointer = ExecutionPointer::fromBuffer(Storage, 1);
    EXPECT_EQ(Memory.validatePointer(Pointer), ExecutionStatus::Success);
    EXPECT_EQ(Memory.validatePointer(ExecutionPointer::fromBuffer(Other)), ExecutionStatus::InvalidPlace);
    ASSERT_EQ(Memory.release(Storage), ExecutionStatus::Success);
    const auto Replacement = Memory.allocateBuffer("replacement");
    ASSERT_TRUE(Replacement.valid());
    EXPECT_EQ(Memory.validatePointer(Pointer), ExecutionStatus::ExpiredPlace);
    EXPECT_NE(Replacement, Storage);
  }

  // Only semantic result transport may retain a local expired pointer; factories, stores and foreign snapshots still fail.
  TEST(RuntimeStorageTest, TransportsExpiredLocalSnapshotsWithoutMakingThemUsable)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    ExecutionHeap Foreign(Context);
    const auto *Byte = Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *PointerType = Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    const RuntimeTypeId Type = Heap.bridge().lowerType(*PointerType);
    const auto Storage = Heap.allocateBuffer("local");
    const auto Pointer = ExecutionPointer::fromBuffer(Storage);
    ASSERT_EQ(Heap.release(Storage), ExecutionStatus::Success);
    EXPECT_FALSE(Heap.pointer(*PointerType, Pointer).valid());
    EXPECT_EQ(Heap.lastStatus(), ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Heap.liveValueCount(), 0U);
    const auto Snapshot = Heap.bridge().raiseValue(Heap, RuntimeValue::fromPointer(Pointer, Type), Type);
    ASSERT_TRUE(Snapshot);
    EXPECT_FALSE(Snapshot.Value.valid());
    EXPECT_EQ(Snapshot.Value.pointer().status(), ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Heap.bridge().lowerValue(Snapshot.Value).Status, ExecutionStatus::ExpiredPlace);
    const auto Destination = Heap.allocateCell(*PointerType);
    ASSERT_TRUE(Destination);
    EXPECT_EQ(Heap.store(Destination.Place, Snapshot.Value), ExecutionStatus::ExpiredPlace);
    const auto Other = Foreign.allocateBuffer("foreign");
    const auto ForeignPointer = ExecutionPointer::fromBuffer(Other);
    ASSERT_EQ(Foreign.release(Other), ExecutionStatus::Success);
    EXPECT_EQ(Heap.bridge().raiseValue(Heap, RuntimeValue::fromPointer(ForeignPointer, Type), Type).Status, ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Heap.liveValueCount(), 1U);
  }

  // Independent layout tables may reuse numeric IDs without allowing a heap adapter to reinterpret their cells.
  TEST(RuntimeStorageTest, RejectsForeignLayoutDomainsAtSemanticBoundary)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    SemanticValueBridge Other(Context);
    const auto &Bool = Context.typePool().getType<ir::TypeKind::Bool>();
    const auto *Byte = Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const RuntimeTypeId LocalType = Heap.bridge().lowerType(Bool);
    const RuntimeTypeId ForeignType = Other.lowerType(*Byte);
    ASSERT_EQ(LocalType, ForeignType);
    ASSERT_NE(Heap.bridge().types()->domain(), Other.types()->domain());
    const auto Cell = Heap.memoryManager().allocateCell(*Other.types()->get(ForeignType), true, RuntimeValue::fromBits(1, ForeignType));
    ASSERT_TRUE(Cell);
    EXPECT_EQ(Heap.load(Cell.Place).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Heap.store(Cell.Place, Heap.boolean(Bool, false)), ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Cell.Place.storage().cell()->loadRuntime().Value.Bits, 1U);
    const auto *Pointer = Context.typePool().getType<ir::TypeKind::Pointer>(Bool, ir::AccessKind::ReadWrite);
    const RuntimeTypeId PointerType = Heap.bridge().lowerType(*Pointer);
    const auto RuntimePointer = RuntimeValue::fromPointer(ExecutionPointer::fromPlace(Cell.Place), PointerType);
    EXPECT_EQ(Heap.bridge().raiseValue(Heap, RuntimePointer, PointerType).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Heap.liveValueCount(), 0U);
  }
} // namespace ink::execution::test
