#include "../artifact/artifact_test_support.h"
#include "ink/ir/analysis/type_layout.h"

#include <gtest/gtest.h>

#include <cstring>

namespace ink::execution::test
{
  // Native-backed heterogeneous fields use the shared target layout, and stored snapshots do not alias later writes.
  TEST(ClassStorageTest, UsesTargetOffsetsAndIndependentSnapshots)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Record"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Flag"), &Test.Bool}, {Test.Context.namePool().intern("Number"), &Test.Int32}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::Record"));
    const auto Value = Heap.classValue(*Class, {Heap.boolean(Test.Bool, true), Heap.integer(Test.Int32, ExecutionInteger(32, 19))});
    ASSERT_TRUE(Value);
    const auto Storage = Heap.allocateCell(*Class, true, Value);
    ASSERT_TRUE(Storage);
    auto &Cell = *Storage.Place.storage().cell();
    const auto Layout = ir::computeTypeLayout(*Class, Test.Compilation.targetContext());
    ASSERT_TRUE(Layout);
    EXPECT_EQ(Cell.layout().Size, Layout->Size);
    EXPECT_EQ(Cell.layout().Alignment, Layout->Alignment);
    EXPECT_EQ(Cell.layout().classDesc().Fields[1].Offset, Layout->FieldOffsets[1]);
    ASSERT_NE(Cell.data(), nullptr);
    const auto Snapshot = Cell.loadRuntime();
    ASSERT_TRUE(Snapshot);
    const auto IntegerType = Heap.bridge().lowerType(Test.Int32);
    EXPECT_EQ(Cell.storeElement(Cell.layout().classDesc().Fields[1].Offset, RuntimeValue::fromBits(23, IntegerType)), ExecutionStatus::Success);
    EXPECT_EQ(Cell.loadElement(Cell.layout().classDesc().Fields[1].Offset, IntegerType).Value.Bits, 23U);
    EXPECT_EQ(Snapshot.Value.fields()[1].Bits, 19U);
    EXPECT_EQ(Value.fields()[1].integer().lowWord(), 19U);
    EXPECT_EQ(Cell.loadElement(1, IntegerType).Status, ExecutionStatus::TypeMismatch);
    ExecutionMemoryManager Tiny(1, sizeof(ExecutionCell));
    EXPECT_EQ(Tiny.allocateCell(Cell.layout()).Status, ExecutionStatus::BudgetExceeded);
  }

  // Recursive pointer types can be lowered beginning with the pointer itself without duplicating the recursive identity.
  TEST(ClassStorageTest, RegistersRecursivePointersBeforeFollowingFields)
  {
    ArtifactContext Test;
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Node"));
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Class, ir::AccessKind::ReadWrite);
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Next"), Pointer}, {Test.Context.namePool().intern("Value"), &Test.Int32}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::Node"));
    const RuntimeTypeId PointerId = Test.Bridge.lowerType(*Pointer);
    ASSERT_NE(PointerId, InvalidRuntimeType);
    const RuntimeTypeId ClassId = Test.Bridge.lowerType(*Class);
    ASSERT_NE(ClassId, InvalidRuntimeType);
    const auto *Layout = Test.Bridge.types()->get(ClassId);
    ASSERT_NE(Layout, nullptr);
    EXPECT_EQ(Layout->classDesc().Fields[0].Type, PointerId);
    EXPECT_EQ(Test.Bridge.types()->get(PointerId)->pointerDesc().Pointee, ClassId);
    EXPECT_FALSE(Layout->Native);
    ExecutionMemoryManager Memory;
    const auto Value = RuntimeValue::fromClass({RuntimeValue::fromPointer({}, PointerId), RuntimeValue::fromBits(42, Layout->classDesc().Fields[1].Type)}, ClassId);
    const auto Cell = Memory.allocateCell(*Layout, true, Value);
    ASSERT_TRUE(Cell);
    EXPECT_EQ(Cell.Place.storage().cell()->loadRuntime().Value.fields()[1].Bits, 42U);
  }

  // A function signature can reserve both a recursive class result and a pointer parameter before either is completed.
  TEST(ClassStorageTest, CompletesReservedSignatureTypesOnDemand)
  {
    ArtifactContext Test;
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("CallableNode"));
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Class, ir::AccessKind::ReadWrite);
    const ir::Type *Parameters[] = {Pointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(*Class, Parameters);
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Next"), Pointer}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::CallableNode"));
    const RuntimeTypeId SignatureId = Test.Bridge.lowerType(*Signature);
    ASSERT_NE(SignatureId, InvalidRuntimeType);
    const RuntimeTypeId ClassId = Test.Bridge.lowerType(*Class);
    const RuntimeTypeId PointerId = Test.Bridge.lowerType(*Pointer);
    const auto *ClassLayout = Test.Bridge.types()->get(ClassId);
    ASSERT_NE(ClassLayout, nullptr);
    EXPECT_EQ(ClassLayout->classDesc().Fields[0].Type, PointerId);
    EXPECT_EQ(Test.Bridge.types()->get(SignatureId)->functionDesc().ReturnType, ClassId);
    EXPECT_EQ(Test.Bridge.types()->get(SignatureId)->functionDesc().Parameters[0], PointerId);
    EXPECT_EQ(Test.Bridge.types()->get(PointerId)->pointerDesc().Pointee, ClassId);
  }

  // Copying a pointer field preserves its target while freezing the containing object rejects the live address.
  TEST(ClassStorageTest, PointerFieldsAliasExplicitlyAndCannotFreeze)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Borrow"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Address"), Pointer}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::Borrow"));
    const auto Integer = Heap.integer(Test.Int32, ExecutionInteger(32, 42));
    const auto Target = Heap.allocateCell(Test.Int32, true, Integer);
    ASSERT_TRUE(Target);
    const auto Value = Heap.classValue(*Class, {Heap.pointer(*Pointer, ExecutionPointer::fromPlace(Target.Place))});
    ASSERT_TRUE(Value);
    const auto Storage = Heap.allocateCell(*Class, true, Value);
    ASSERT_TRUE(Storage);
    const auto Copy = Heap.load(Storage.Place);
    ASSERT_TRUE(Copy);
    EXPECT_EQ(Copy.Value.fields()[0].pointer().address(), Target.Place.storage().cell()->data());
    EXPECT_EQ(Copy.Value.toConstant(Test.Context), nullptr);
    EXPECT_EQ(Heap.release(Target.Place), ExecutionStatus::Success);
    EXPECT_EQ(Copy.Value.fields()[0].pointer().status(), ExecutionStatus::Success);
  }

  // Empty classes occupy a distinct one-byte object extent, including when nested in arrays.
  TEST(ClassStorageTest, EmptyClassesHaveAddressableExtent)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Empty"));
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, {}, "tests::Empty"));
    const auto Value = Heap.classValue(*Class, {});
    ASSERT_TRUE(Value);
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(*Class, 2);
    const auto ArrayValue = Heap.array(*Array, {Value, Value});
    const auto Storage = Heap.allocateCell(*Array, true, ArrayValue);
    ASSERT_TRUE(Storage);
    const auto ClassId = Heap.bridge().lowerType(*Class);
    EXPECT_EQ(Heap.bridge().types()->get(ClassId)->Size, 1U);
    EXPECT_EQ(Storage.Place.storage().cell()->size(), 2U);
    ASSERT_TRUE(Storage.Place.storage().cell()->loadElement(1, ClassId));
    EXPECT_EQ(Storage.Place.storage().cell()->loadElement(2, ClassId).Status, ExecutionStatus::TypeMismatch);
  }

  // Padded non-native integer fields retain their exact bit width while array strides and class offsets use the shared object ABI.
  TEST(ClassStorageTest, PaddedIntegersUseTargetStrideWithoutChangingTheirValue)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Integer = Test.Context.typePool().getType<ir::TypeKind::Integer>(24, false);
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(*Integer, 2);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("PackedValues"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Flag"), &Test.Bool}, {Test.Context.namePool().intern("Values"), Array}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::PackedValues"));
    const auto First = Heap.integer(*Integer, ExecutionInteger(24, 0x123456));
    const auto Second = Heap.integer(*Integer, ExecutionInteger(24, 0xabcdef));
    const auto Value = Heap.classValue(*Class, {Heap.boolean(Test.Bool, true), Heap.array(*Array, {First, Second})});
    const auto Storage = Heap.allocateCell(*Class, true, Value);
    ASSERT_TRUE(Storage);
    auto &Cell = *Storage.Place.storage().cell();
    const auto IntegerId = Heap.bridge().lowerType(*Integer);
    EXPECT_EQ(Heap.bridge().types()->get(IntegerId)->Size, 4U);
    EXPECT_EQ(Cell.layout().classDesc().Fields[1].Offset, 4U);
    EXPECT_EQ(Cell.layout().Size, 12U);
    EXPECT_NE(Cell.data(), nullptr);
    const auto Loaded = Cell.loadElement(8, IntegerId);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.Bits, 0xabcdefU);
    EXPECT_EQ(Cell.storeElement(8, RuntimeValue::fromBits(0x654321, IntegerId)), ExecutionStatus::Success);
    EXPECT_EQ(Cell.loadElement(8, IntegerId).Value.Bits, 0x654321U);
    EXPECT_EQ(Value.fields()[1].array()[1].integer().lowWord(), 0xabcdefU);
  }

  // Wide fields and pointer fields occupy aligned native bytes, and a native pointer-slot write is visible on the next load.
  TEST(ClassStorageTest, StoresWideAndPointerFieldsInOneAlignedObject)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("RawLayout"));
    const ir::ClassField Fields[] = {
        {Test.Context.namePool().intern("Tag"), &Test.Bool},
        {Test.Context.namePool().intern("Bits"), Wide},
        {Test.Context.namePool().intern("Address"), Pointer},
    };
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::RawLayout"));
    const auto First = Heap.allocateCell(Test.Int32, true, Heap.integer(Test.Int32, ExecutionInteger(32, 40)));
    const auto Second = Heap.allocateCell(Test.Int32, true, Heap.integer(Test.Int32, ExecutionInteger(32, 42)));
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    const auto Initial = Heap.classValue(*Class, {Heap.boolean(Test.Bool, true), Heap.integer(*Wide, ExecutionInteger(128, 7)), Heap.pointer(*Pointer, ExecutionPointer::fromPlace(First.Place))});
    const auto Object = Heap.allocateCell(*Class, true, Initial);
    ASSERT_TRUE(Object);
    auto &Cell = *Object.Place.storage().cell();
    const auto &Layout = Cell.layout();
    auto *Bytes = static_cast<std::byte *>(Cell.data());
    ASSERT_NE(Bytes, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(Bytes) % Layout.Alignment, 0U);
    EXPECT_EQ(Layout.classDesc().Fields[1].Offset, Test.Compilation.targetContext().integerAlignment(128));
    void *Stored = nullptr;
    std::memcpy(&Stored, Bytes + Layout.classDesc().Fields[2].Offset, sizeof(Stored));
    EXPECT_EQ(Stored, First.Place.storage().cell()->data());
    const auto Snapshot = Heap.load(Object.Place);
    ASSERT_TRUE(Snapshot);
    void *Replacement = Second.Place.storage().cell()->data();
    std::memcpy(Bytes + Layout.classDesc().Fields[2].Offset, &Replacement, sizeof(Replacement));
    const auto Loaded = Heap.load(Object.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.fields()[2].pointer().address(), Replacement);
    EXPECT_EQ(Snapshot.Value.fields()[2].pointer().address(), Stored);
    EXPECT_EQ(Loaded.Value.fields()[1].integer().lowWord(), 7U);
  }

  // Updating another field cannot release string bytes referenced by an object's native slice or change previous value snapshots.
  TEST(ClassStorageTest, RetainsStringBytesAcrossFieldStores)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("TextBox"));
    const ir::ClassField Fields[] = {
        {Test.Context.namePool().intern("Text"), String},
        {Test.Context.namePool().intern("Count"), &Test.Int32},
    };
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::TextBox"));
    const auto Storage = Heap.allocateCell(*Class);
    ASSERT_TRUE(Storage);
    auto &Cell = *Storage.Place.storage().cell();
    ASSERT_EQ(Heap.store(Storage.Place, Heap.classValue(*Class, {Heap.string(*String, "first"), Heap.integer(Test.Int32, ExecutionInteger(32, 1))})), ExecutionStatus::Success);
    const auto Snapshot = Heap.load(Storage.Place);
    ASSERT_TRUE(Snapshot);
    const RuntimeTypeId StringId = Heap.bridge().lowerType(*String);
    const RuntimeTypeId IntegerId = Heap.bridge().lowerType(Test.Int32);
    ASSERT_EQ(Cell.storeElement(Cell.layout().classDesc().Fields[1].Offset, RuntimeValue::fromBits(42, IntegerId)), ExecutionStatus::Success);
    ASSERT_EQ(Cell.storeElement(0, RuntimeValue::fromString("second", StringId)), ExecutionStatus::Success);
    ASSERT_EQ(Cell.storeElement(Cell.layout().classDesc().Fields[1].Offset, RuntimeValue::fromBits(43, IntegerId)), ExecutionStatus::Success);
    const auto Loaded = Heap.load(Storage.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.fields()[0].string(), "second");
    EXPECT_EQ(Loaded.Value.fields()[1].integer().lowWord(), 43U);
    EXPECT_EQ(Snapshot.Value.fields()[0].string(), "first");
    const char *Bytes = nullptr;
    std::memcpy(&Bytes, Cell.data(), sizeof(Bytes));
    EXPECT_EQ(std::string_view(Bytes, 6), "second");
  }
  // Every allocation and array element layout shares the type table's immutable class description.
  TEST(ClassStorageTest, SharesClassDescriptionAcrossInstancesAndArrayLayouts)
  {
    ArtifactContext Test;
    ExecutionHeap Heap(Test.Context);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("SharedRecord"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Number"), &Test.Int32}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::SharedRecord"));
    const RuntimeTypeId Type = Heap.bridge().lowerType(*Class);
    const TypeDesc &Layout = *Heap.bridge().types()->get(Type);
    const auto First = Heap.memoryManager().allocateCell(Layout);
    const auto Second = Heap.memoryManager().allocateCell(Layout);
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(*Class, 3);
    const RuntimeTypeId ArrayType = Heap.bridge().lowerType(*Array);
    EXPECT_EQ(&First.Place.storage().cell()->layout().classDesc(), &Layout.classDesc());
    EXPECT_EQ(&Second.Place.storage().cell()->layout().classDesc(), &Layout.classDesc());
    EXPECT_EQ(&Heap.bridge().types()->get(ArrayType)->arrayDesc().ElementLayout->classDesc(), &Layout.classDesc());
    ASSERT_EQ(Layout.classDesc().Fields.size(), 1U);
    EXPECT_EQ(Layout.classDesc().Fields[0].Name, "Number");
    EXPECT_EQ(Layout.classDesc().Fields[0].Type, Heap.bridge().lowerType(Test.Int32));
  }
  // Exhausting the owned-string budget cannot publish earlier fields of a failed aggregate assignment.
  TEST(ClassStorageTest, RejectsStringBudgetWithoutPartialAggregateStore)
  {
    ArtifactContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("BudgetRecord"));
    const ir::ClassField Fields[] = {
        {Test.Context.namePool().intern("Count"), &Test.Int32},
        {Test.Context.namePool().intern("Text"), String},
    };
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::BudgetRecord"));
    const RuntimeTypeId Type = Test.Bridge.lowerType(*Class);
    const RuntimeTypeId IntegerId = Test.Bridge.lowerType(Test.Int32);
    const RuntimeTypeId StringId = Test.Bridge.lowerType(*String);
    ExecutionMemoryManager Memory(8, 4096);
    const auto Initial = RuntimeValue::fromClass({RuntimeValue::fromBits(19, IntegerId), RuntimeValue::fromString("original", StringId)}, Type);
    const auto Storage = Memory.allocateCell(*Test.Bridge.types()->get(Type), true, Initial);
    ASSERT_TRUE(Storage);
    auto &Cell = *Storage.Place.storage().cell();
    const auto Replacement = RuntimeValue::fromClass({RuntimeValue::fromBits(99, IntegerId), RuntimeValue::fromString(std::string(4096, 'x'), StringId)}, Type);
    EXPECT_EQ(Cell.storeRuntime(Replacement), ExecutionStatus::BudgetExceeded);
    const auto Loaded = Cell.loadRuntime();
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.fields()[0].Bits, 19U);
    EXPECT_EQ(Loaded.Value.fields()[1].string(), "original");
  }
} // namespace ink::execution::test
