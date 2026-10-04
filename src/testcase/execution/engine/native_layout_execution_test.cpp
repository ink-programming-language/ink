#include "../artifact/artifact_test_support.h"
#include "ink/execution/bytecode/execution_compiler.h"

#include <gtest/gtest.h>

namespace ink::execution::test
{
  // Nested arrays/classes containing strings survive byte copies, callee teardown and mutation of the original object.
  TEST(NativeLayoutExecutionTest, PreservesNestedStringsAndSnapshotsAcrossCalls)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *Record = Test.Builder.createClassType(Test.Context.namePool().intern("TextRecord"));
    const ir::ClassField Fields[] = {
        {Test.Context.namePool().intern("Text"), String},
        {Test.Context.namePool().intern("Count"), &Test.Int32},
    };
    ASSERT_TRUE(Test.Builder.defineClassType(*Record, Fields, "tests::TextRecord"));
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(*Record, 2);
    const ir::Type *RecordParameters[] = {Record};
    auto *Relay = Test.function("relay", *Record, RecordParameters);
    ASSERT_TRUE(Test.begin(*Relay));
    const auto *Storage = Test.Builder.createAllocaInstruction(*Record);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Relay->parameters()[0]), nullptr);
    const auto *Snapshot = Test.Builder.createLoadInstruction(*Storage);
    const auto *Count = Test.Builder.createFieldPointerInstruction(*Storage, 1);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Count, Test.integer(99)), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Snapshot), nullptr);
    const ir::Type *ArrayParameters[] = {Array};
    auto *Main = Test.function("main", *Array, ArrayParameters);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *First = Test.Builder.createArrayExtractInstruction(*Main->parameters()[0], Test.integer(0));
    const auto *Second = Test.Builder.createArrayExtractInstruction(*Main->parameters()[0], Test.integer(1));
    const ir::Value *CallArguments[] = {First};
    const auto *Returned = Test.Builder.createCallInstruction(*Relay, CallArguments);
    const ir::Value *Elements[] = {Returned, Second};
    const auto *Result = Test.Builder.createArrayInstruction(*Array, Elements);
    ASSERT_NE(Test.Builder.createReturnInstruction(Result), nullptr);
    auto &Heap = Engine.heap();
    const auto FirstValue = Heap.classValue(*Record, {Heap.string(*String, std::string_view("a\0b", 3)), Heap.integer(Test.Int32, ExecutionInteger(32, 19))});
    const auto SecondValue = Heap.classValue(*Record, {Heap.string(*String, "second"), Heap.integer(Test.Int32, ExecutionInteger(32, 23))});
    const ExecutionValueRef Arguments[] = {Heap.array(*Array, {FirstValue, SecondValue})};
    const auto Executed = Engine.execute(*Main, Arguments);
    ASSERT_TRUE(Executed) << static_cast<unsigned>(Executed.Status);
    ASSERT_EQ(Executed.Value.array().size(), 2U);
    EXPECT_EQ(Executed.Value.array()[0].fields()[0].string(), std::string_view("a\0b", 3));
    EXPECT_EQ(Executed.Value.array()[0].fields()[1].integer().lowWord(), 19U);
    EXPECT_EQ(Executed.Value.array()[1].fields()[0].string(), "second");
    EXPECT_EQ(Executed.Value.array()[1].fields()[1].integer().lowWord(), 23U);
    EXPECT_EQ(Heap.liveStorageCount(), 0U);
  }

  // A VM string store retains its bytes after the argument wrapper and call frame are destroyed.
  TEST(NativeLayoutExecutionTest, RetainsStringStoredThroughAnExternalAddress)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*String, ir::AccessKind::ReadWrite);
    const ir::Type *Parameters[] = {Pointer, String};
    auto *Store = Test.function("store", Test.Context.typePool().getType<ir::TypeKind::Void>(), Parameters);
    ASSERT_TRUE(Test.begin(*Store));
    ASSERT_NE(Test.Builder.createStoreInstruction(*Store->parameters()[0], *Store->parameters()[1]), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = ExecutionCompiler{}.compile(*Store, Test.Bridge);
    ASSERT_TRUE(Compiled);
    ExecutionImage Image;
    Image.Layouts = Test.Bridge.types();
    const auto Id = Compiled.Function->Id;
    Image.Descriptors.emplace(Id, RuntimeFunctionDescriptor{Id, Compiled.Function->Signature});
    Image.Functions.emplace(Id, std::move(Compiled.Function));
    ExecutionLinker Linker(std::move(Image));
    ExecutionMachine Machine(Engine, Linker);
    struct StringBytes
    {
        const char *Data = nullptr;
        std::size_t Length = 0;
    } Stored;
    {
      const RuntimeValue Arguments[] = {RuntimeValue::fromPointer(ExecutionPointer::fromNative(&Stored), Test.Bridge.lowerType(*Pointer)), RuntimeValue::fromString("retained", Test.Bridge.lowerType(*String))};
      ASSERT_TRUE(Machine.execute(Id, Arguments));
    }
    EXPECT_EQ(std::string_view(Stored.Data, Stored.Length), "retained");
  }
  // Native-layout frames are bounded before recursion can accumulate unbounded aggregate copies.
  TEST(NativeLayoutExecutionTest, BoundsActiveFrameBytesAndUnwindsCalls)
  {
    ArtifactContext Test;
    ExecutionLimits Limits;
    Limits.MaxStorageBytes = 128;
    Limits.MaxCallDepth = 1000;
    Limits.MaxEvaluationDepth = 1000;
    ExecutionEngine Engine(Test.Context, Limits);
    auto *Recursive = Test.function("recursive", Test.Int32);
    ASSERT_TRUE(Test.begin(*Recursive));
    const auto *Call = Test.Builder.createCallInstruction(*Recursive);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    EXPECT_EQ(Engine.execute(*Recursive).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }
} // namespace ink::execution::test
