#include "ink/execution/memory/execution_heap.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <limits>

namespace ink::execution::test
{
  // Nested native arrays retain independent value snapshots while element addresses share contiguous storage.
  TEST(ExecutionArrayStorageTest, NativeNestedElementsPreserveSnapshotsAndManagedLifetimes)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    const auto &Integer = *Context.typePool().getType<ir::TypeKind::Integer>(32, true);
    const auto &Row = *Context.typePool().getType<ir::TypeKind::Array>(Integer, 2);
    const auto &Matrix = *Context.typePool().getType<ir::TypeKind::Array>(Row, 2);
    const auto One = Heap.integer(Integer, ExecutionInteger(32, 1));
    const auto Two = Heap.integer(Integer, ExecutionInteger(32, 2));
    const auto RowValue = Heap.array(Row, {One, Two});
    const auto Initial = Heap.array(Matrix, {RowValue, RowValue});
    const auto Allocation = Heap.allocateCell(Matrix, true, Initial);
    ASSERT_TRUE(Allocation);
    ExecutionCell &Cell = *Allocation.Place.storage().cell();
    ASSERT_NE(Cell.data(), nullptr);
    EXPECT_EQ(Cell.size(), 4 * sizeof(std::int32_t));
    const auto Before = Heap.load(Allocation.Place);
    ASSERT_TRUE(Before);
    const RuntimeTypeId IntegerId = Heap.bridge().lowerType(Integer);
    const auto Pointer = ExecutionPointer::fromPlace(Allocation.Place, 3 * sizeof(std::int32_t));
    ASSERT_TRUE(Pointer.valid());
    *static_cast<std::int32_t *>(Pointer.address()) = 42;
    const auto Element = Cell.loadElement(3 * sizeof(std::int32_t), IntegerId);
    ASSERT_TRUE(Element);
    EXPECT_EQ(Element.Value.Bits, 42U);
    ASSERT_EQ(Cell.storeElement(sizeof(std::int32_t), RuntimeValue::fromBits(7, IntegerId)), ExecutionStatus::Success);
    const auto After = Heap.load(Allocation.Place);
    ASSERT_TRUE(After);
    EXPECT_EQ(After.Value.array()[0].array()[1].integer().lowWord(), 7U);
    EXPECT_EQ(After.Value.array()[1].array()[1].integer().lowWord(), 42U);
    EXPECT_EQ(Before.Value.array()[1].array()[1].integer().lowWord(), 2U);
    EXPECT_EQ(Initial.array()[0].array()[1].integer().lowWord(), 2U);
    const auto Recovered = Heap.pointerFromAddress(Pointer.address());
    EXPECT_EQ(Recovered.address(), Pointer.address());
    EXPECT_EQ(Cell.loadElement(Cell.size(), IntegerId).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Cell.elementLayout(Cell.size(), IntegerId, true)->Type, IntegerId);
    EXPECT_EQ(Cell.elementLayout(1, IntegerId), nullptr);
    ASSERT_EQ(Heap.release(Allocation.Place), ExecutionStatus::Success);
    EXPECT_EQ(Pointer.status(), ExecutionStatus::Success);
    EXPECT_EQ(After.Value.array()[1].array()[1].integer().lowWord(), 42U);
  }

  // Wide-integer arrays support initialized element reads before the final element initializes the complete array.
  TEST(ExecutionArrayStorageTest, PartiallyInitializesNonnativeArrayElements)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    const auto &Integer = *Context.typePool().getType<ir::TypeKind::Integer>(128, false);
    const auto &Array = *Context.typePool().getType<ir::TypeKind::Array>(Integer, 2);
    const RuntimeTypeId IntegerId = Heap.bridge().lowerType(Integer);
    const auto Allocation = Heap.allocateCell(Array);
    ASSERT_TRUE(Allocation);
    ExecutionCell &Cell = *Allocation.Place.storage().cell();
    EXPECT_NE(Cell.data(), nullptr);
    EXPECT_EQ(Cell.loadElement(0, IntegerId).Status, ExecutionStatus::Uninitialized);
    ASSERT_EQ(Cell.storeElement(0, RuntimeValue::fromInteger(ExecutionInteger(128, 11), IntegerId)), ExecutionStatus::Success);
    EXPECT_EQ(Cell.loadRuntime().Status, ExecutionStatus::Uninitialized);
    ASSERT_TRUE(Cell.loadElement(0, IntegerId));
    EXPECT_EQ(Cell.loadElement(0, IntegerId).Value.integer().lowWord(), 11U);
    EXPECT_EQ(Cell.loadElement(16, IntegerId).Status, ExecutionStatus::Uninitialized);
    ASSERT_EQ(Cell.storeElement(16, RuntimeValue::fromInteger(ExecutionInteger(128, 22), IntegerId)), ExecutionStatus::Success);
    const auto Complete = Heap.load(Allocation.Place);
    ASSERT_TRUE(Complete);
    EXPECT_EQ(Complete.Value.array()[1].integer().lowWord(), 22U);
    ASSERT_EQ(Cell.storeElement(0, RuntimeValue::fromInteger(ExecutionInteger(128, 99), IntegerId)), ExecutionStatus::Success);
    EXPECT_EQ(Complete.Value.array()[0].integer().lowWord(), 11U);
    const auto WrongType = RuntimeValue::fromBits(0, Heap.bridge().lowerType(Context.typePool().getType<ir::TypeKind::Bool>()));
    EXPECT_EQ(Cell.storeElement(0, WrongType), ExecutionStatus::TypeMismatch);
  }

  // Constant freezing and semantic/runtime bridging preserve nested array types and pooled element identities.
  TEST(ExecutionArrayStorageTest, RoundTripsNestedConstantsAndRejectsMalformedValues)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    const auto &Integer = *Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto &Row = *Context.typePool().getType<ir::TypeKind::Array>(Integer, 2);
    const auto &Matrix = *Context.typePool().getType<ir::TypeKind::Array>(Row, 2);
    const auto Byte = Heap.integer(Integer, ExecutionInteger(8, 65));
    const auto RowValue = Heap.array(Row, {Byte, Byte});
    const auto Initial = Heap.array(Matrix, {RowValue, RowValue});
    ASSERT_TRUE(Initial);
    const ir::Constant *Frozen = Initial.toConstant(Context);
    ASSERT_NE(Frozen, nullptr);
    ASSERT_TRUE(ir::ArrayConstant::classof(Frozen));
    const auto Restored = Heap.fromConstant(*Frozen);
    ASSERT_TRUE(Restored);
    EXPECT_EQ(Restored.toConstant(Context), Frozen);
    const auto Lowered = Heap.bridge().lowerConstant(*Frozen);
    ASSERT_TRUE(Lowered);
    const auto Raised = Heap.bridge().raiseValue(Heap, Lowered.Value, Lowered.Value.Type);
    ASSERT_TRUE(Raised);
    EXPECT_EQ(Raised.Value.toConstant(Context), Frozen);
    EXPECT_FALSE(Heap.array(Row, {Byte}));
    EXPECT_FALSE(Heap.array(Row, {RowValue, RowValue}));
    const auto ReadOnly = Heap.allocateCell(Matrix, false, Initial);
    ASSERT_TRUE(ReadOnly);
    EXPECT_EQ(ReadOnly.Place.storage().cell()->storeElement(0, RuntimeValue::fromBits(66, Heap.bridge().lowerType(Integer))), ExecutionStatus::ReadOnly);
  }

  // Empty arrays remain valid values and dynamic array storage is charged against the existing allocation budget.
  TEST(ExecutionArrayStorageTest, HandlesEmptyArraysAndChargesStorageBudget)
  {
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionHeap Heap(Context);
    const auto &Integer = *Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto &Empty = *Context.typePool().getType<ir::TypeKind::Array>(Integer, 0);
    const auto EmptyValue = Heap.array(Empty, {});
    ASSERT_TRUE(EmptyValue);
    const auto EmptyCell = Heap.allocateCell(Empty, true, EmptyValue);
    ASSERT_TRUE(EmptyCell);
    const auto Loaded = Heap.load(EmptyCell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_TRUE(Loaded.Value.array().empty());
    const auto &EmptyRows = *Context.typePool().getType<ir::TypeKind::Array>(Empty, 2);
    const auto NestedEmpty = Heap.array(EmptyRows, {EmptyValue, EmptyValue});
    const auto EmptyRowsCell = Heap.allocateCell(EmptyRows, true, NestedEmpty);
    ASSERT_TRUE(EmptyRowsCell);
    const RuntimeTypeId EmptyId = Heap.bridge().lowerType(Empty);
    const auto EmptyRow = EmptyRowsCell.Place.storage().cell()->loadElement(0, EmptyId);
    ASSERT_TRUE(EmptyRow);
    EXPECT_TRUE(EmptyRow.Value.array().empty());
    EXPECT_EQ(EmptyRowsCell.Place.storage().cell()->storeElement(0, RuntimeValue::fromArray({}, EmptyId)), ExecutionStatus::Success);
    const auto &Array = *Context.typePool().getType<ir::TypeKind::Array>(Integer, 100);
    const RuntimeTypeId Type = Heap.bridge().lowerType(Array);
    ExecutionMemoryManager Tiny(10, sizeof(ExecutionCell) + 10);
    EXPECT_EQ(Tiny.allocateCell(*Heap.bridge().types()->get(Type)).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Tiny.allocatedStorageCount(), 0U);
    const auto &Wide = *Context.typePool().getType<ir::TypeKind::Integer>(64, false);
    const auto &Overflow = *Context.typePool().getType<ir::TypeKind::Array>(Wide, std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Heap.bridge().lowerType(Overflow), InvalidRuntimeType);
  }
} // namespace ink::execution::test
