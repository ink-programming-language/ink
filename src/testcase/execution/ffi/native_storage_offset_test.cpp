#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/ffi/external_function.h"
#include "ink/execution/ffi/ffi_argument.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <string_view>

#if defined(_WIN32) || defined(__linux__)
namespace ink::execution::test
{
  namespace
  {
    extern "C" const void *inkTestNativeStorageInteriorAddress(const std::int32_t *Value) noexcept
    {
      return reinterpret_cast<const unsigned char *>(Value) + 1;
    }

    extern "C" const void *inkTestNativeStorageOnePastAddress(const std::int32_t *Value) noexcept
    {
      return Value + 1;
    }

    extern "C" const std::int32_t *inkTestNativeStorageOffsetIdentity(const std::int32_t *Value) noexcept
    {
      return Value;
    }

    class NativeOffsetContext final
    {
      public:
        NativeOffsetContext()
            : Context(Compilation),
              Builder(Context),
              Engine(Context),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true)),
              UInt8(*Context.typePool().getType<ir::TypeKind::Integer>(8, false)),
              Int32Pointer(*Context.typePool().getType<ir::TypeKind::Pointer>(Int32, ir::AccessKind::ReadWrite)),
              ReadPointer(*Context.typePool().getType<ir::TypeKind::Pointer>(Int32, ir::AccessKind::ReadOnly)),
              BytePointer(*Context.typePool().getType<ir::TypeKind::Pointer>(UInt8, ir::AccessKind::ReadWrite))
        {
        }

        ExecutionPlaceResult cell(bool Writable = true)
        {
          return Engine.heap().allocateCell(Int32, Writable, Engine.heap().integer(Int32, ExecutionInteger(32, 0x12345678)));
        }

        ExecutionValueResult nativeAddress(ExecutionPlace Place, const ir::PointerType &ResultType, NativeSymbol Symbol)
        {
          const ir::Type *Parameters[] = {&ReadPointer};
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(ResultType, Parameters);
          if (!Signature)
          {
            return {ExecutionStatus::InvalidArguments};
          }
          auto Function = Builder.createFunction(Context.namePool().intern("inkTestNativeStorageOffset"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Import);
          if (!Function)
          {
            return {ExecutionStatus::InvalidArguments};
          }
          NativeSymbolCache Cache([Symbol](std::string_view) -> NativeSymbol
                                  {
                                    return Symbol;
                                  });
          const ExecutionValueRef Arguments[] = {Engine.heap().pointer(ReadPointer, ExecutionPointer::fromPlace(Place))};
          return callExternalFunction(Engine.heap(), Cache, *Function, Arguments);
        }

        std::unique_ptr<ir::Function> reader(const ir::PointerType &PointerType)
        {
          const ir::Type *Parameters[] = {&PointerType};
          auto Function = function("ReadOffset", PointerType.pointeeType(), Parameters);
          if (!Function || !begin(*Function))
          {
            return nullptr;
          }
          auto *Loaded = Builder.createLoadInstruction(*Function->parameters().front());
          if (!Loaded || !Builder.createReturnInstruction(Loaded))
          {
            return nullptr;
          }
          return Function;
        }

        std::unique_ptr<ir::Function> writer(const ir::PointerType &PointerType)
        {
          const ir::Type *Parameters[] = {&PointerType, &PointerType.pointeeType()};
          auto Function = function("WriteOffset", Context.typePool().getType<ir::TypeKind::Void>(), Parameters);
          if (!Function || !begin(*Function))
          {
            return nullptr;
          }
          if (!Builder.createStoreInstruction(*Function->parameters()[0], *Function->parameters()[1]) || !Builder.createReturnInstruction())
          {
            return nullptr;
          }
          return Function;
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
        const ir::IntegerType &UInt8;
        const ir::PointerType &Int32Pointer;
        const ir::PointerType &ReadPointer;
        const ir::PointerType &BytePointer;

      private:
        std::unique_ptr<ir::Function> function(std::string_view Name, const ir::Type &ResultType, std::span<const ir::Type *const> Parameters)
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(ResultType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Name), *Signature) : nullptr;
        }

        bool begin(ir::Function &Function)
        {
          auto *Body = Builder.createFunctionBody(Function);
          return Body && Builder.setInsertPoint(*Body);
        }
    };
  } // namespace

  // A native interior address reads and writes only its selected object-representation byte through Ink IR.
  TEST(ExecutionNativeOffsetTest, ReturnedInteriorByteAddressReadsAndWritesSelectedByte)
  {
    NativeOffsetContext Test;
    const auto Cell = Test.cell();
    ASSERT_TRUE(Cell);
    const auto Before = Test.Engine.heap().load(Cell.Place);
    ASSERT_TRUE(Before);
    const auto Address = Test.nativeAddress(Cell.Place, Test.BytePointer, reinterpret_cast<NativeSymbol>(&inkTestNativeStorageInteriorAddress));
    ASSERT_TRUE(Address);
    EXPECT_EQ(Address.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(ExecutionPlace(Test.Engine.heap().memoryManager().storageFromAddress(Address.Value.pointer().address())), Cell.Place);
    std::array<unsigned char, sizeof(std::int32_t)> ExpectedBytes{};
    const std::uint32_t InitialBits = 0x12345678;
    std::memcpy(ExpectedBytes.data(), &InitialBits, sizeof(InitialBits));
    auto Reader = Test.reader(Test.BytePointer);
    auto Writer = Test.writer(Test.BytePointer);
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const ExecutionValueRef ReadArguments[] = {Address.Value};
    const auto Read = Test.Engine.execute(*Reader, ReadArguments);
    ASSERT_TRUE(Read);
    EXPECT_EQ(Read.Value.integer().bits(), ir::IntegerBits(8, ExpectedBytes[1]));
    const ExecutionValueRef WriteArguments[] = {Address.Value, Test.Engine.heap().integer(Test.UInt8, ExecutionInteger(8, 0xAB))};
    ASSERT_TRUE(Test.Engine.execute(*Writer, WriteArguments));
    ExpectedBytes[1] = 0xAB;
    std::uint32_t ExpectedBits = 0;
    std::memcpy(&ExpectedBits, ExpectedBytes.data(), sizeof(ExpectedBits));
    const auto After = Test.Engine.heap().load(Cell.Place);
    ASSERT_TRUE(After);
    EXPECT_EQ(After.Value.integer().bits(), ir::IntegerBits(32, ExpectedBits));
    EXPECT_EQ(Before.Value.integer().bits(), ir::IntegerBits(32, InitialBits));
  }

  // A misdeclared typed pointer returned inside a scalar cannot silently load or overwrite the complete original cell.
  TEST(ExecutionNativeOffsetTest, RejectsTypedDereferenceOfReturnedInteriorAddress)
  {
    NativeOffsetContext Test;
    const auto Cell = Test.cell();
    ASSERT_TRUE(Cell);
    const auto Address = Test.nativeAddress(Cell.Place, Test.Int32Pointer, reinterpret_cast<NativeSymbol>(&inkTestNativeStorageInteriorAddress));
    ASSERT_TRUE(Address);
    auto Reader = Test.reader(Test.Int32Pointer);
    auto Writer = Test.writer(Test.Int32Pointer);
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const ExecutionValueRef ReadArguments[] = {Address.Value};
    const ExecutionValueRef WriteArguments[] = {Address.Value, Test.Engine.heap().integer(Test.Int32, ExecutionInteger(32, 99))};
    EXPECT_EQ(Test.Engine.execute(*Reader, ReadArguments).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::TypeMismatch);
    const auto Loaded = Test.Engine.heap().load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 0x12345678));
  }

  // A native one-past cell address preserves provenance but cannot be dereferenced for either a byte load or byte store.
  TEST(ExecutionNativeOffsetTest, RejectsDereferenceOfReturnedOnePastAddress)
  {
    NativeOffsetContext Test;
    const auto Cell = Test.cell();
    ASSERT_TRUE(Cell);
    const auto Address = Test.nativeAddress(Cell.Place, Test.BytePointer, reinterpret_cast<NativeSymbol>(&inkTestNativeStorageOnePastAddress));
    ASSERT_TRUE(Address);
    EXPECT_EQ(Address.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(ExecutionPlace(Test.Engine.heap().memoryManager().storageFromAddress(Address.Value.pointer().address())), Cell.Place);
    EXPECT_EQ(Address.Value.pointer().status(), ExecutionStatus::Success);
    auto Reader = Test.reader(Test.BytePointer);
    auto Writer = Test.writer(Test.BytePointer);
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const ExecutionValueRef ReadArguments[] = {Address.Value};
    const ExecutionValueRef WriteArguments[] = {Address.Value, Test.Engine.heap().integer(Test.UInt8, ExecutionInteger(8, 0xAB))};
    EXPECT_EQ(Test.Engine.execute(*Reader, ReadArguments).Status, ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::InvalidPlace);
    const auto Loaded = Test.Engine.heap().load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 0x12345678));
  }

  // Returning a mutable-looking byte pointer never grants write access to the readonly cell behind that native address.
  TEST(ExecutionNativeOffsetTest, ReturnedInteriorAddressPreservesCellReadOnlyProtection)
  {
    NativeOffsetContext Test;
    const auto Cell = Test.cell(false);
    ASSERT_TRUE(Cell);
    const auto Address = Test.nativeAddress(Cell.Place, Test.BytePointer, reinterpret_cast<NativeSymbol>(&inkTestNativeStorageInteriorAddress));
    ASSERT_TRUE(Address);
    auto Reader = Test.reader(Test.BytePointer);
    auto Writer = Test.writer(Test.BytePointer);
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const ExecutionValueRef ReadArguments[] = {Address.Value};
    ASSERT_TRUE(Test.Engine.execute(*Reader, ReadArguments));
    const ExecutionValueRef WriteArguments[] = {Address.Value, Test.Engine.heap().integer(Test.UInt8, ExecutionInteger(8, 0xAB))};
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::ReadOnly);
    const auto Loaded = Test.Engine.heap().load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 0x12345678));
  }

  // A typed one-past cell pointer can cross an identity C call while remaining forbidden as an Ink load or store address.
  TEST(ExecutionNativeOffsetTest, TypedOnePastPointerCanCrossNativeCallWithoutDereference)
  {
    NativeOffsetContext Test;
    const auto Cell = Test.cell();
    ASSERT_TRUE(Cell);
    const auto Address = Test.nativeAddress(Cell.Place, Test.Int32Pointer, reinterpret_cast<NativeSymbol>(&inkTestNativeStorageOnePastAddress));
    ASSERT_TRUE(Address);
    const ir::Type *Parameters[] = {&Test.Int32Pointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32Pointer, Parameters);
    ASSERT_NE(Signature, nullptr);
    auto Identity = Test.Builder.createFunction(Test.Context.namePool().intern("inkTestNativeStorageOnePastIdentity"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Import);
    ASSERT_NE(Identity, nullptr);
    NativeSymbolCache Cache([](std::string_view) -> NativeSymbol
                            {
                              return reinterpret_cast<NativeSymbol>(&inkTestNativeStorageOffsetIdentity);
                            });
    const ExecutionValueRef Arguments[] = {Address.Value};
    const auto Returned = callExternalFunction(Test.Engine.heap(), Cache, *Identity, Arguments);
    ASSERT_TRUE(Returned);
    EXPECT_EQ(Returned.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(ExecutionPlace(Test.Engine.heap().memoryManager().storageFromAddress(Returned.Value.pointer().address())), Cell.Place);
    EXPECT_EQ(Returned.Value.pointer().address(), Address.Value.pointer().address());
    auto Reader = Test.reader(Test.Int32Pointer);
    auto Writer = Test.writer(Test.Int32Pointer);
    ASSERT_NE(Reader, nullptr);
    ASSERT_NE(Writer, nullptr);
    const ExecutionValueRef ReadArguments[] = {Returned.Value};
    const ExecutionValueRef WriteArguments[] = {Returned.Value, Test.Engine.heap().integer(Test.Int32, ExecutionInteger(32, 99))};
    EXPECT_EQ(Test.Engine.execute(*Reader, ReadArguments).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.execute(*Writer, WriteArguments).Status, ExecutionStatus::TypeMismatch);
    const auto Loaded = Test.Engine.heap().load(Cell.Place);
    ASSERT_TRUE(Loaded);
    EXPECT_EQ(Loaded.Value.integer().bits(), ir::IntegerBits(32, 0x12345678));
  }

  // Marshalling transports raw pointer bits; valid alignment and access extent remain the caller's responsibility.
  TEST(ExecutionNativeOffsetTest, MarshalsRawAddressesWithoutInferringBufferElementTypes)
  {
    NativeOffsetContext Test;
    const auto Buffer = Test.Engine.heap().allocateBuffer("A", false);
    ASSERT_TRUE(Buffer.valid());
    const auto Forged = Test.Engine.heap().pointer(Test.Int32Pointer, ExecutionPointer::fromBuffer(Buffer));
    ASSERT_TRUE(Forged);
    FfiArgument Argument;
    EXPECT_EQ(Argument.prepare(Test.Engine.heap(), Test.Int32Pointer, Forged), ExecutionStatus::Success);
    EXPECT_NE(Argument.address(), nullptr);
    const auto BytePointer = Test.Engine.heap().pointer(Test.BytePointer, ExecutionPointer::fromBuffer(Buffer));
    ASSERT_TRUE(BytePointer);
    EXPECT_EQ(Argument.prepare(Test.Engine.heap(), Test.BytePointer, BytePointer), ExecutionStatus::Success);
    ASSERT_NE(Argument.address(), nullptr);
    EXPECT_EQ(Buffer.buffer()->data()[0], 'A');
  }
} // namespace ink::execution::test
#endif
