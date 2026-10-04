#include "ink/execution/ffi/ffi_argument.h"
#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/ffi/external_function.h"
#include "ink/execution/ffi/native_symbol_cache.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

namespace ink::execution::test
{
  namespace
  {
#if defined(_WIN32) || defined(__linux__)
    extern "C" char *inkTestHeapReturnInterior(char *Text) noexcept
    {
      return Text + 1;
    }

    extern "C" std::int32_t inkTestHeapReadFirstByte(char *Text) noexcept
    {
      return static_cast<unsigned char>(*Text);
    }
#endif

    class ArgumentContext final
    {
      public:
        ArgumentContext()
            : Context(Compilation),
              Builder(Context),
              Heap(Context),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true)),
              UInt8(*Context.typePool().getType<ir::TypeKind::Integer>(8, false)),
              BytePointer(*Context.typePool().getType<ir::TypeKind::Pointer>(UInt8, ir::AccessKind::ReadWrite))
        {
        }

        const ir::IntegerConstant &integer(std::uint64_t Bits)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Bits));
        }

        const ir::StringConstant &string(std::string_view Text)
        {
          const auto *Type = Context.typePool().getType<ir::TypeKind::Slice>(UInt8, ir::AccessKind::ReadOnly);
          return *Context.constantPool().getStringConstant(*Type, Text);
        }

        std::unique_ptr<ir::Function> function()
        {
          const ir::Type *Parameters[] = {&Int32};
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(Int32, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern("ArgumentOwner"), *Signature) : nullptr;
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionHeap Heap;
        const ir::IntegerType &Int32;
        const ir::IntegerType &UInt8;
        const ir::PointerType &BytePointer;
    };
  } // namespace

  // Prepared argument addresses remain bound to their owning storage and cannot be copied or moved into aliases.
  static_assert(!std::is_copy_constructible_v<FfiArgument>);
  static_assert(!std::is_copy_assignable_v<FfiArgument>);
  static_assert(!std::is_move_constructible_v<FfiArgument>);
  static_assert(!std::is_move_assignable_v<FfiArgument>);

  // A scalar constant exposed only as Value prepares the exact native integer bytes at its argument address.
  TEST(FfiArgumentTest, PreparesScalarConstantThroughValueReference)
  {
    ArgumentContext Test;
    const ir::Value &Value = Test.integer(static_cast<std::uint32_t>(-19));
    FfiArgument Argument;
    EXPECT_EQ(Argument.address(), nullptr);
    ASSERT_EQ(Argument.prepare(Test.Heap, Test.Int32, Value), ExecutionStatus::Success);
    ASSERT_NE(Argument.address(), nullptr);
    std::int32_t Native = 0;
    std::memcpy(&Native, Argument.address(), sizeof(Native));
    EXPECT_EQ(Native, -19);
  }

  // Uncomputed IR values, different scalar types and foreign contexts each retain their distinct conversion failure.
  TEST(FfiArgumentTest, RejectsUnevaluatedMismatchedAndForeignValues)
  {
    ArgumentContext Test;
    ArgumentContext Other;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ASSERT_EQ(Function->parameters().size(), 1U);
    FfiArgument Argument;
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32, *Function->parameters().front()), ExecutionStatus::RuntimeValue);
    EXPECT_EQ(Argument.address(), nullptr);
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.BytePointer, *Function->parameters().front()), ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Argument.address(), nullptr);
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32, Test.Context.constantPool().getBoolConstant(true)), ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Argument.address(), nullptr);
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32, Other.integer(9)), ExecutionStatus::ForeignContext);
    EXPECT_EQ(Argument.address(), nullptr);
  }

  // Reusing a prepared holder after failure clears its stale address and permits a later successful conversion.
  TEST(FfiArgumentTest, FailedPreparationClearsPreviousAddressAndAllowsReuse)
  {
    ArgumentContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    FfiArgument Argument;
    ASSERT_EQ(Argument.prepare(Test.Heap, Test.Int32, Test.integer(42)), ExecutionStatus::Success);
    ASSERT_NE(Argument.address(), nullptr);
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32, *Function->parameters().front()), ExecutionStatus::RuntimeValue);
    EXPECT_EQ(Argument.address(), nullptr);
    ASSERT_EQ(Argument.prepare(Test.Heap, Test.BytePointer, Test.string("fresh")), ExecutionStatus::Success);
    ASSERT_NE(Argument.address(), nullptr);
    void *NativeBuffer = nullptr;
    std::memcpy(&NativeBuffer, Argument.address(), sizeof(NativeBuffer));
    ASSERT_NE(NativeBuffer, nullptr);
    EXPECT_EQ(std::string_view(static_cast<const char *>(NativeBuffer)), "fresh");
    EXPECT_EQ(Argument.prepare(Test.Heap, Test.Int32, Test.string("wrong")), ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Argument.address(), nullptr);
    ASSERT_EQ(Argument.prepare(Test.Heap, Test.Int32, Test.integer(7)), ExecutionStatus::Success);
    std::int32_t Native = 0;
    std::memcpy(&Native, Argument.address(), sizeof(Native));
    EXPECT_EQ(Native, 7);
  }

  // Separate holders copy the same embedded-NUL string into independent writable buffers with a trailing terminator.
  TEST(FfiArgumentTest, OwnsIndependentWritableStringBuffers)
  {
    ArgumentContext Test;
    const auto &String = Test.string(std::string_view("A\0B", 3));
    FfiArgument First;
    FfiArgument Second;
    ASSERT_EQ(First.prepare(Test.Heap, Test.BytePointer, String), ExecutionStatus::Success);
    ASSERT_EQ(Second.prepare(Test.Heap, Test.BytePointer, String), ExecutionStatus::Success);
    ASSERT_NE(First.address(), nullptr);
    ASSERT_NE(Second.address(), nullptr);
    void *FirstBuffer = nullptr;
    void *SecondBuffer = nullptr;
    std::memcpy(&FirstBuffer, First.address(), sizeof(FirstBuffer));
    std::memcpy(&SecondBuffer, Second.address(), sizeof(SecondBuffer));
    ASSERT_NE(FirstBuffer, nullptr);
    ASSERT_NE(SecondBuffer, nullptr);
    EXPECT_NE(FirstBuffer, SecondBuffer);
    auto *FirstBytes = static_cast<char *>(FirstBuffer);
    auto *SecondBytes = static_cast<char *>(SecondBuffer);
    EXPECT_EQ(std::string_view(FirstBytes, 3), std::string_view("A\0B", 3));
    EXPECT_EQ(std::string_view(SecondBytes, 3), std::string_view("A\0B", 3));
    EXPECT_EQ(FirstBytes[3], '\0');
    EXPECT_EQ(SecondBytes[3], '\0');
    FirstBytes[0] = 'Z';
    EXPECT_EQ(SecondBytes[0], 'A');
    EXPECT_EQ(String.value(), std::string_view("A\0B", 3));
  }

  // Resetting and destroying a string argument release its temporary allocation even if a weak buffer handle survives.
  TEST(FfiArgumentTest, ReclaimsTemporaryStringBuffersOnResetAndDestruction)
  {
    ArgumentContext Test;
    ExecutionStorageRef Retained;
    {
      FfiArgument Argument;
      ASSERT_EQ(Argument.prepare(Test.Heap, Test.BytePointer, Test.string("first")), ExecutionStatus::Success);
      Retained = Argument.bufferRef();
      EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
      ASSERT_EQ(Argument.prepare(Test.Heap, Test.Int32, Test.integer(7)), ExecutionStatus::Success);
      EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
      EXPECT_EQ(Retained.status(), ExecutionStatus::ExpiredPlace);
      ASSERT_EQ(Argument.prepare(Test.Heap, Test.BytePointer, Test.string("second")), ExecutionStatus::Success);
      Retained = Argument.bufferRef();
      EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
    }
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Retained.status(), ExecutionStatus::ExpiredPlace);
  }

#if defined(_WIN32) || defined(__linux__)
  // A native interior pointer return preserves its temporary string allocation until the heap owner explicitly releases it.
  TEST(FfiArgumentTest, NativeInteriorReturnPromotesTemporaryBuffer)
  {
    ArgumentContext Test;
    const ir::Type *Parameters[] = {&Test.BytePointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.BytePointer, Parameters);
    ASSERT_NE(Signature, nullptr);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("inkTestHeapReturnInterior"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&inkTestHeapReturnInterior);
    });
    const ExecutionValueRef Arguments[] = {Test.Heap.fromConstant(Test.string("ABC"))};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Pointer);
    EXPECT_EQ(Result.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 1U);
    ASSERT_NE(Result.Value.pointer().address(), nullptr);
    EXPECT_EQ(std::string_view(static_cast<const char *>(Result.Value.pointer().address())), "BC");
    ASSERT_EQ(Test.Heap.release(Test.Heap.memoryManager().storageFromAddress(Result.Value.pointer().address())), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Result.Value.pointer().status(), ExecutionStatus::Success);
  }

  // A scalar native return leaves no temporary string allocation behind after argument marshalling finishes.
  TEST(FfiArgumentTest, NativeScalarReturnReclaimsUnescapedTemporaryBuffer)
  {
    ArgumentContext Test;
    const ir::Type *Parameters[] = {&Test.BytePointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, Parameters);
    ASSERT_NE(Signature, nullptr);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("inkTestHeapReadFirstByte"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&inkTestHeapReadFirstByte);
    });
    const ExecutionValueRef Arguments[] = {Test.Heap.fromConstant(Test.string("ABC"))};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(32, 'A'));
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Test.Heap.allocatedStorageCount(), 1U);
  }

  // An alias returned for an existing buffer retains its original release boundary instead of allocating or owning storage.
  TEST(FfiArgumentTest, NativeInteriorReturnPreservesExistingBufferLifetime)
  {
    ArgumentContext Test;
    const ir::Type *Parameters[] = {&Test.BytePointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.BytePointer, Parameters);
    ASSERT_NE(Signature, nullptr);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("inkTestHeapReturnInterior"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    ASSERT_NE(Function, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&inkTestHeapReturnInterior);
    });
    const auto Buffer = Test.Heap.allocateBuffer("ABC");
    const ExecutionValueRef Arguments[] = {Test.Heap.pointer(Test.BytePointer, ExecutionPointer::fromBuffer(Buffer))};
    const auto Result = callExternalFunction(Test.Heap, Cache, *Function, Arguments);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(Test.Heap.memoryManager().storageFromAddress(Result.Value.pointer().address()).buffer(), Buffer.buffer());
    EXPECT_EQ(Test.Heap.allocatedStorageCount(), 1U);
    ASSERT_EQ(Test.Heap.release(Buffer), ExecutionStatus::Success);
    EXPECT_EQ(Test.Heap.liveStorageCount(), 0U);
    EXPECT_EQ(Result.Value.pointer().status(), ExecutionStatus::Success);
  }
#endif
} // namespace ink::execution::test
