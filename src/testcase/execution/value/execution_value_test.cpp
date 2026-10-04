#include "ink/execution/memory/execution_heap.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace ink::execution::test
{
  namespace
  {
    class ValueContext final
    {
      public:
        ValueContext()
            : Context(Compilation),
              Builder(Context),
              Heap(Context),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true)),
              UInt8(*Context.typePool().getType<ir::TypeKind::Integer>(8, false)),
              Bytes(*Context.typePool().getType<ir::TypeKind::Slice>(UInt8, ir::AccessKind::ReadOnly)),
              BytePointer(*Context.typePool().getType<ir::TypeKind::Pointer>(UInt8, ir::AccessKind::ReadWrite))
        {
        }

        const ir::IntegerConstant &integer(std::uint64_t Bits)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Bits));
        }

        const ir::StringConstant &string(std::string_view Text)
        {
          return *Context.constantPool().getStringConstant(Bytes, Text);
        }

        std::unique_ptr<ir::Function> function()
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(Int32);
          return Signature ? Builder.createFunction(Context.namePool().intern("ExecutionValueFunction"), *Signature) : nullptr;
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionHeap Heap;
        const ir::IntegerType &Int32;
        const ir::IntegerType &UInt8;
        const ir::SliceType &Bytes;
        const ir::PointerType &BytePointer;
    };
  } // namespace

  // Heap objects retain stable identities while immutable value handles can share their payload.
  static_assert(!std::is_copy_constructible_v<ExecutionBuffer>);
  static_assert(!std::is_move_constructible_v<ExecutionBuffer>);
  static_assert(std::is_abstract_v<ExecutionValue>);
  static_assert(std::is_copy_constructible_v<ExecutionValueRef>);
  static_assert(std::is_move_constructible_v<ExecutionValueRef>);

  // Integer values retain all 128 bits after their source payload changes and freeze to canonical typed constants.
  TEST(ExecutionValueTest, OwnsWideIntegerPayloadAndPreservesExactBits)
  {
    ValueContext Test;
    const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, true);
    ASSERT_NE(Type, nullptr);
    const std::uint64_t Words[] = {0xfedcba9876543210ULL, 0x8123456789abcdefULL};
    const ir::IntegerBits Bits(128, Words);
    ExecutionInteger Source(Bits);
    const ExecutionValueRef Value = Test.Heap.integer(*Type, Source);
    ASSERT_TRUE(Value.valid());
    EXPECT_EQ(Value.kind(), ExecutionValueKind::Integer);
    EXPECT_EQ(Value.type(), Type);
    ASSERT_EQ(Source.negate(Source), ExecutionStatus::Success);
    EXPECT_NE(Source.bits(), Bits);
    EXPECT_EQ(Value.integer().bits(), Bits);
    const ir::Constant *Frozen = Value.toConstant(Test.Context);
    ASSERT_NE(Frozen, nullptr);
    ASSERT_TRUE(ir::IntegerConstant::classof(Frozen));
    EXPECT_EQ(static_cast<const ir::IntegerConstant *>(Frozen)->value(), Bits);
    const ExecutionValueRef Restored = Test.Heap.fromConstant(*Frozen);
    ASSERT_TRUE(Restored.valid());
    EXPECT_EQ(Restored.integer().bits(), Bits);
    EXPECT_EQ(Restored.toConstant(Test.Context), Frozen);
  }

  // Half subnormals, float NaN payloads and double negative zero survive execution storage and constant freezing unchanged.
  TEST(ExecutionValueTest, PreservesFloatingPointEncodingsWithoutConversion)
  {
    ValueContext Test;
    const ir::FloatBits Cases[] = {
        ir::FloatBits(16, 0x8001),
        ir::FloatBits(32, 0x7fc12345),
        ir::FloatBits(64, 0x8000000000000000ULL),
    };
    for (const ir::FloatBits Bits : Cases)
    {
      const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Float>(Bits.bitWidth());
      ASSERT_NE(Type, nullptr);
      const ExecutionValueRef Value = Test.Heap.floating(*Type, Bits);
      ASSERT_TRUE(Value.valid());
      EXPECT_EQ(Value.kind(), ExecutionValueKind::Float);
      EXPECT_EQ(Value.floating(), Bits);
      const ir::Constant *Frozen = Value.toConstant(Test.Context);
      ASSERT_NE(Frozen, nullptr);
      ASSERT_TRUE(ir::FloatConstant::classof(Frozen));
      EXPECT_EQ(static_cast<const ir::FloatConstant *>(Frozen)->value(), Bits);
      const ExecutionValueRef Restored = Test.Heap.fromConstant(*Frozen);
      ASSERT_TRUE(Restored.valid());
      EXPECT_EQ(Restored.floating(), Bits);
    }
  }

  // Embedded-NUL strings own bytes independently of their source and share immutable payloads when handles are copied.
  TEST(ExecutionValueTest, OwnsStringBytesAndSharesImmutableCopies)
  {
    ValueContext Test;
    std::string Source(80, 'x');
    Source[3] = '\0';
    const std::string Expected = Source;
    const ExecutionValueRef Direct = Test.Heap.string(Test.Bytes, Source);
    const auto &Constant = Test.string(Source);
    const ExecutionValueRef Restored = Test.Heap.fromConstant(Constant);
    const ExecutionValueRef Copy = Restored;
    Source.assign(80, 'z');
    ASSERT_TRUE(Direct.valid());
    ASSERT_TRUE(Restored.valid());
    ASSERT_TRUE(Copy.valid());
    EXPECT_EQ(Direct.kind(), ExecutionValueKind::String);
    EXPECT_EQ(Direct.string(), Expected);
    EXPECT_EQ(Restored.string(), Expected);
    EXPECT_EQ(Copy.string(), Expected);
    EXPECT_NE(Direct.string().data(), Source.data());
    EXPECT_NE(Restored.string().data(), Constant.value().data());
    EXPECT_EQ(Copy.string().data(), Restored.string().data());
    EXPECT_EQ(Copy.toConstant(Test.Context), &Constant);
  }

  // Only local scalar and string payloads freeze; pointers, functions, void and foreign contexts never enter a constant pool.
  TEST(ExecutionValueTest, FreezesSafeLocalValuesAndRejectsExecutionOnlyPayloads)
  {
    ValueContext Test;
    ValueContext Other;
    const auto &Boolean = Test.Context.constantPool().getBoolConstant(true);
    const ExecutionValueRef Scalar = Test.Heap.fromConstant(Boolean);
    ASSERT_TRUE(Scalar.valid());
    EXPECT_EQ(Scalar.kind(), ExecutionValueKind::Boolean);
    EXPECT_TRUE(Scalar.boolean());
    EXPECT_EQ(Scalar.toConstant(Test.Context), &Boolean);
    const ExecutionValueRef Integer = Test.Heap.fromConstant(Test.integer(17));
    EXPECT_EQ(Integer.toConstant(Test.Context), &Test.integer(17));
    const ExecutionValueRef String = Test.Heap.fromConstant(Test.string("owned"));
    EXPECT_EQ(String.toConstant(Test.Context), &Test.string("owned"));
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    const ExecutionValueRef FunctionValue = Test.Heap.function(*Function);
    const ExecutionValueRef Void = Test.Heap.voidValue(Test.Context.typePool().getType<ir::TypeKind::Void>());
    char Native = 'A';
    const ExecutionValueRef Pointer = Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromNative(&Native));
    const ExecutionValueRef Null = Test.Heap.pointer(Test.BytePointer, {});
    ASSERT_TRUE(FunctionValue.valid());
    ASSERT_TRUE(Void.valid());
    ASSERT_TRUE(Pointer.valid());
    ASSERT_TRUE(Null.valid());
    EXPECT_EQ(FunctionValue.function(), Function.get());
    EXPECT_EQ(FunctionValue.type(), &Function->functionType());
    const std::size_t LocalSize = Test.Context.constantPool().size();
    const std::size_t OtherSize = Other.Context.constantPool().size();
    EXPECT_EQ(FunctionValue.toConstant(Test.Context), nullptr);
    EXPECT_EQ(Void.toConstant(Test.Context), nullptr);
    EXPECT_EQ(Pointer.toConstant(Test.Context), nullptr);
    EXPECT_EQ(Null.toConstant(Test.Context), nullptr);
    EXPECT_EQ(Scalar.toConstant(Other.Context), nullptr);
    EXPECT_EQ(Integer.toConstant(Other.Context), nullptr);
    EXPECT_EQ(String.toConstant(Other.Context), nullptr);
    EXPECT_EQ(Test.Context.constantPool().size(), LocalSize);
    EXPECT_EQ(Other.Context.constantPool().size(), OtherSize);
  }

  // Inconsistent kinds, integer widths, floating encodings and writable string slices fail validation and freezing.
  TEST(ExecutionValueTest, RejectsMismatchedTypesAndMalformedPayloads)
  {
    ValueContext Test;
    const auto *Float32 = Test.Context.typePool().getType<ir::TypeKind::Float>(32);
    const auto *WritableBytes = Test.Context.typePool().getType<ir::TypeKind::Slice>(Test.UInt8, ir::AccessKind::ReadWrite);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(WritableBytes, nullptr);
    const ExecutionValueRef Invalid[] = {
        ExecutionValueRef{},
        Test.Heap.boolean(Test.Int32, true),
        Test.Heap.integer(Test.Int32, ExecutionInteger(16, 1)),
        Test.Heap.integer(Test.Int32, ExecutionInteger(0)),
        Test.Heap.floating(*Float32, ir::FloatBits(64, 0)),
        Test.Heap.floating(*Float32, ir::FloatBits(32, 0x100000000ULL)),
        Test.Heap.floating(Test.Int32, ir::FloatBits(32, 0)),
        Test.Heap.string(Test.Int32, "x"),
        Test.Heap.string(*WritableBytes, "x"),
        Test.Heap.pointer(Test.Int32, {}),
        Test.Heap.voidValue(Test.Int32),
    };
    const std::size_t PoolSize = Test.Context.constantPool().size();
    for (const ExecutionValueRef &Value : Invalid)
    {
      EXPECT_FALSE(Value.valid());
      EXPECT_EQ(Value.toConstant(Test.Context), nullptr);
    }
    EXPECT_EQ(Invalid[0].kind(), ExecutionValueKind::Invalid);
    EXPECT_EQ(Invalid[0].type(), nullptr);
    EXPECT_EQ(Test.Context.constantPool().size(), PoolSize);
  }

  // Raw pointer copies preserve the machine address without owning the pointed-to buffer.
  TEST(ExecutionValueTest, RawPointersShareAddressesWithoutOwningStorage)
  {
    ValueContext Test;
    const auto Buffer = Test.Heap.allocateBuffer("ABC");
    ASSERT_TRUE(Buffer.valid());
    void *Address = Buffer.buffer()->data();
    const ExecutionPointer Start = ExecutionPointer::fromBuffer(Buffer);
    const ExecutionPointer Interior = Start.offsetBy(1);
    const auto Value = Test.Heap.pointer(Test.BytePointer, Interior);
    ASSERT_TRUE(Value);
    EXPECT_EQ(sizeof(ExecutionPointer), sizeof(void *));
    EXPECT_EQ(Start.address(), Address);
    EXPECT_EQ(Value.pointer().address(), static_cast<char *>(Address) + 1);
    *static_cast<char *>(Interior.address()) = 'Z';
    EXPECT_EQ(Buffer.buffer()->data()[1], 'Z');
    EXPECT_EQ(Value.toConstant(Test.Context), nullptr);
    ASSERT_EQ(Test.Heap.release(Buffer), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Start.address(), Address);
    EXPECT_TRUE(Value.valid());
    // Freed addresses may be transported, but must never be dereferenced.
    EXPECT_EQ(Test.Heap.pointer(Test.BytePointer, Value.pointer()).pointer().address(), Interior.address());
  }

  // Raw one-past addresses retain their bytes, while buffer allocation still controls optional terminators.
  TEST(ExecutionValueTest, RepresentsOnePastAddressesAndOptionalTerminators)
  {
    ValueContext Test;
    const auto Terminated = Test.Heap.allocateBuffer("ab");
    const auto Raw = Test.Heap.allocateBuffer("ab", false);
    ASSERT_EQ(Terminated.buffer()->size(), 3U);
    ASSERT_EQ(Raw.buffer()->size(), 2U);
    EXPECT_EQ(Terminated.buffer()->data()[2], '\0');
    const auto End = ExecutionPointer::fromBuffer(Raw, 2);
    EXPECT_EQ(End.address(), Raw.buffer()->data() + 2);
    EXPECT_EQ(Test.Heap.pointerFromAddress(End.address()).address(), End.address());
  }

  // Null and native pointers use the same single-address representation, without allocation metadata.
  TEST(ExecutionValueTest, RepresentsNullAndNativePointers)
  {
    const ExecutionPointer Null;
    EXPECT_EQ(Null.kind(), ExecutionPointer::Kind::Null);
    EXPECT_TRUE(Null.valid());
    EXPECT_EQ(Null.address(), nullptr);
    char Storage = 'x';
    const auto Pointer = ExecutionPointer::fromNative(&Storage);
    EXPECT_EQ(Pointer.kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(Pointer.address(), &Storage);
    EXPECT_EQ(ExecutionPointer::fromPlace({}).address(), nullptr);
  }

  // Copying immutable handles does not allocate another value, and the last owner immediately destroys its payload.
  TEST(ExecutionHeapTest, ReleasesImmutableValuesAfterTheirLastHandle)
  {
    ValueContext Test;
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
    ExecutionValueRef Retained;
    {
      const auto Original = Test.Heap.string(Test.Bytes, std::string(128, 'x'));
      ASSERT_TRUE(Original.valid());
      Retained = Original;
      EXPECT_EQ(Retained.get(), Original.get());
      EXPECT_EQ(Test.Heap.liveValueCount(), 1U);
    }
    EXPECT_EQ(Retained.string(), std::string(128, 'x'));
    EXPECT_EQ(Test.Heap.liveValueCount(), 1U);
    Retained = {};
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
  }

  // Native cells keep bytes instead of a value owner, and loaded snapshots survive storage release.
  TEST(ExecutionHeapTest, ReclaimsCellsWithoutDiscardingLoadedSnapshots)
  {
    ValueContext Test;
    const auto Cell = Test.Heap.allocateCell(Test.Int32, true, Test.Heap.fromConstant(Test.integer(41)));
    ASSERT_TRUE(Cell);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
    auto Loaded = Test.Heap.load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 41));
    ASSERT_EQ(Test.Heap.release(Cell.Place), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Test.Heap.liveValueCount(), 1U);
    EXPECT_EQ(Test.Heap.load(Cell.Place).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Heap.release(Cell.Place), ExecutionStatus::ExpiredPlace);
    Loaded.Value = {};
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
  }

  // Heap ownership and generation checks prevent foreign and already released handles from naming another allocation.
  TEST(ExecutionHeapTest, RejectsForeignAndReusedStorageIdentities)
  {
    ValueContext Test;
    ExecutionHeap Other(Test.Context);
    const auto First = Test.Heap.allocateCell(Test.Int32);
    ASSERT_TRUE(First);
    EXPECT_EQ(Other.load(First.Place).Status, ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Other.release(First.Place), ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Test.Heap.release(ExecutionPlace{}), ExecutionStatus::InvalidPlace);
    ASSERT_EQ(Test.Heap.release(First.Place), ExecutionStatus::Success);
    const auto Second = Test.Heap.allocateCell(Test.Int32, true, Test.Heap.fromConstant(Test.integer(29)));
    ASSERT_TRUE(Second);
    EXPECT_NE(First.Place, Second.Place);
    EXPECT_EQ(Test.Heap.load(First.Place).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Heap.store(First.Place, Test.Heap.fromConstant(Test.integer(7))), ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Heap.release(First.Place), ExecutionStatus::ExpiredPlace);
    const auto Current = Test.Heap.load(Second.Place);
    ASSERT_TRUE(Current);
    EXPECT_EQ(Current.Value.integer().bits(), ir::IntegerBits(32, 29));
    EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
    EXPECT_EQ(Test.Heap.allocatedStorageCount(), 2U);

    const auto Buffer = Test.Heap.allocateBuffer("first");
    const auto Pointer = ExecutionPointer::fromBuffer(Buffer);
    EXPECT_EQ(Other.release(Buffer), ExecutionStatus::InvalidPlace);
    ASSERT_EQ(Test.Heap.release(Buffer), ExecutionStatus::Success);
    const auto Replacement = Test.Heap.allocateBuffer("second");
    ASSERT_NE(Replacement.buffer(), nullptr);
    EXPECT_EQ(Pointer.status(), ExecutionStatus::Success);
    EXPECT_NE(Pointer.address(), nullptr);
    EXPECT_EQ(Test.Heap.release(Buffer), ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(std::string_view(Replacement.buffer()->data()), "second");
  }

  // Releasing a storage slot does not reset the cumulative allocation budget or revive earlier handles.
  TEST(ExecutionHeapTest, CountsReleasedAllocationsTowardTheStorageBudget)
  {
    ValueContext Test;
    ExecutionHeap Limited(Test.Context, 1);
    const auto First = Limited.allocateCell(Test.Int32);
    ASSERT_TRUE(First);
    ASSERT_EQ(Limited.release(First.Place), ExecutionStatus::Success);
    EXPECT_EQ(Limited.liveStorageCount(), 0U);
    EXPECT_EQ(Limited.allocatedStorageCount(), 1U);
    EXPECT_EQ(Limited.allocateCell(Test.Int32).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Limited.load(First.Place).Status, ExecutionStatus::ExpiredPlace);
  }

  // Runtime pointer cycles retain no semantic value objects and loaded pointer snapshots preserve unowned machine addresses.
  TEST(ExecutionHeapTest, ReleasesMutuallyReferencingCellsWithoutTracing)
  {
    ValueContext Test;
    const auto First = Test.Heap.allocateCell(Test.BytePointer);
    const auto Second = Test.Heap.allocateCell(Test.BytePointer);
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    ASSERT_EQ(Test.Heap.store(First.Place, Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromPlace(Second.Place))), ExecutionStatus::Success);
    ASSERT_EQ(Test.Heap.store(Second.Place, Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromPlace(First.Place))), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 2U);
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
    ASSERT_EQ(Test.Heap.release(First.Place), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
    EXPECT_EQ(Test.Heap.load(Second.Place).Value.pointer().status(), ExecutionStatus::Success);
    ASSERT_EQ(Test.Heap.release(Second.Place), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
  }

  // Immutable scalars retain their payload after heap destruction, whereas pointers lose access to heap-owned storage.
  TEST(ExecutionHeapTest, KeepsScalarOwnersAndExpiresStorageAfterHeapDestruction)
  {
    ValueContext Test;
    ExecutionValueRef Scalar;
    ExecutionValueRef Pointer;
    {
      ExecutionHeap Temporary(Test.Context);
      Scalar = Temporary.fromConstant(Test.integer(73));
      Pointer = Temporary.pointer(Test.BytePointer, ExecutionPointer::fromBuffer(Temporary.allocateBuffer("temporary")));
      ASSERT_TRUE(Pointer.valid());
    }
    ASSERT_TRUE(Scalar.valid());
    EXPECT_EQ(Scalar.integer().bits(), ir::IntegerBits(32, 73));
    EXPECT_EQ(Scalar.toConstant(Test.Context), &Test.integer(73));
    EXPECT_EQ(Pointer.pointer().status(), ExecutionStatus::Success);
    EXPECT_NE(Pointer.pointer().address(), nullptr);
  }

  // Invalid factories leave no live value and report the rejected type or context explicitly.
  TEST(ExecutionHeapTest, ReportsFactoryFailuresWithoutCreatingObjects)
  {
    ValueContext Test;
    ValueContext Other;
    EXPECT_FALSE(Test.Heap.boolean(Test.Int32, true).valid());
    EXPECT_EQ(Test.Heap.lastStatus(), ExecutionStatus::TypeMismatch);
    EXPECT_FALSE(Test.Heap.fromConstant(Other.integer(9)).valid());
    EXPECT_EQ(Test.Heap.lastStatus(), ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Heap.liveValueCount(), 0U);
    const auto Valid = Test.Heap.fromConstant(Test.integer(9));
    EXPECT_TRUE(Valid.valid());
    EXPECT_EQ(Test.Heap.lastStatus(), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveValueCount(), 1U);
  }

  // Result construction never reports success without an owned value, and failed results discard supplied payloads.
  TEST(ExecutionValueTest, RequiresAnOwnedValueForSuccessfulResults)
  {
    ValueContext Test;
    const ExecutionValueResult Default;
    const ExecutionValueResult StatusOnly(ExecutionStatus::Success);
    const ExecutionValueResult MissingValue(ExecutionStatus::Success, {});
    EXPECT_FALSE(Default.succeeded());
    EXPECT_FALSE(static_cast<bool>(Default));
    EXPECT_EQ(Default.Status, ExecutionStatus::InvalidArguments);
    EXPECT_FALSE(StatusOnly.succeeded());
    EXPECT_EQ(StatusOnly.Status, ExecutionStatus::InvalidArguments);
    EXPECT_FALSE(MissingValue.succeeded());
    EXPECT_EQ(MissingValue.Status, ExecutionStatus::InvalidArguments);
    const auto Value = Test.Heap.fromConstant(Test.integer(17));
    const ExecutionValueResult Success(ExecutionStatus::Success, Value);
    EXPECT_TRUE(Success.succeeded());
    EXPECT_TRUE(static_cast<bool>(Success));
    EXPECT_EQ(Success.Value.get(), Value.get());
    const ExecutionValueResult Failure(ExecutionStatus::TypeMismatch, Value);
    EXPECT_FALSE(Failure.succeeded());
    EXPECT_EQ(Failure.Status, ExecutionStatus::TypeMismatch);
    EXPECT_FALSE(static_cast<bool>(Failure.Value));
    ExecutionValueResult Modified;
    Modified.Status = ExecutionStatus::Success;
    EXPECT_FALSE(Modified.succeeded());
  }
} // namespace ink::execution::test
