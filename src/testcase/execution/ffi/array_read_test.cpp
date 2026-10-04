#include "external_call_test_support.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#if defined(_WIN32) || defined(__linux__)
namespace ink::execution::test
{
  // The real platform read function preserves binary NUL/high bytes, short reads and EOF in an interior array view.
  TEST(ExecutionArrayReadTest, ReadsBinaryPipeIntoManagedArrayAndPreservesSnapshots)
  {
    for (const bool VoidBuffer : {false, true})
    {
      NativePipe Pipe;
      ASSERT_TRUE(Pipe.valid());
      const std::string Payload("\0\x7f\x80\xff\n", 5);
#ifdef _WIN32
      ASSERT_EQ(_write(Pipe.writer(), Payload.data(), static_cast<unsigned>(Payload.size())), static_cast<int>(Payload.size()));
      constexpr std::string_view ReadSymbol = "_read";
#else
      ASSERT_EQ(write(Pipe.writer(), Payload.data(), Payload.size()), static_cast<ssize_t>(Payload.size()));
      constexpr std::string_view ReadSymbol = "read";
#endif
      Pipe.closeWriter();
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      ir::IRBuilder Builder(Context);
      ExecutionEngine Engine(Context);
      ExecutionHeap &Heap = Engine.heap();
      const auto &FdType = *Context.typePool().getType<ir::TypeKind::Integer>(32, true);
      const auto &Byte = *Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto &CountType = *Context.typePool().getType<ir::TypeKind::Integer>(WriteCountWidth, false);
      const auto &ReturnType = *Context.typePool().getType<ir::TypeKind::Integer>(WriteReturnWidth, true);
      const ir::Type &Pointee = VoidBuffer ? Context.typePool().getType<ir::TypeKind::Void>() : static_cast<const ir::Type &>(Byte);
      const auto &PointerType = *Context.typePool().getType<ir::TypeKind::Pointer>(Pointee, ir::AccessKind::ReadWrite);
      const auto &ArrayType = *Context.typePool().getType<ir::TypeKind::Array>(Byte, 8);
      const ir::Type *Parameters[] = {&FdType, &PointerType, &CountType};
      const auto &Signature = *Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
      auto Read = Builder.createFunction(Context.namePool().intern(ReadSymbol), Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Import);
      ASSERT_NE(Read, nullptr);
      const auto Initial = Heap.array(ArrayType, std::vector<ExecutionValueRef>(8, Heap.integer(Byte, ExecutionInteger(8, 0x55))));
      const auto Storage = Heap.allocateCell(ArrayType, true, Initial);
      ASSERT_TRUE(Storage);
      const auto Snapshot = Heap.load(Storage.Place);
      ASSERT_TRUE(Snapshot);
      const auto Address = Heap.pointer(PointerType, ExecutionPointer::fromPlace(Storage.Place, 2));
      ASSERT_TRUE(Address);
      ExecutionValueRef Arguments[] = {Heap.integer(FdType, ExecutionInteger(32, Pipe.reader())), Address, Heap.integer(CountType, ExecutionInteger(WriteCountWidth, 6))};
      const auto Received = Engine.execute(*Read, Arguments);
      ASSERT_TRUE(Received);
      EXPECT_EQ(Received.Value.integer().lowWord(), Payload.size());
      const auto Loaded = Heap.load(Storage.Place);
      ASSERT_TRUE(Loaded);
      EXPECT_EQ(Loaded.Value.array()[0].integer().lowWord(), 0x55U);
      EXPECT_EQ(Loaded.Value.array()[1].integer().lowWord(), 0x55U);
      EXPECT_EQ(Loaded.Value.array()[7].integer().lowWord(), 0x55U);
      for (std::size_t Index = 0; Index < Payload.size(); ++Index)
      {
        EXPECT_EQ(Loaded.Value.array()[Index + 2].integer().lowWord(), static_cast<unsigned char>(Payload[Index]));
        EXPECT_EQ(Snapshot.Value.array()[Index + 2].integer().lowWord(), 0x55U);
      }
      const auto End = Engine.execute(*Read, Arguments);
      ASSERT_TRUE(End);
      EXPECT_EQ(End.Value.integer().lowWord(), 0U);
      const auto AfterEnd = Heap.load(Storage.Place);
      ASSERT_TRUE(AfterEnd);
      EXPECT_EQ(AfterEnd.Value.toConstant(Context), Loaded.Value.toConstant(Context));
      Arguments[2] = Heap.integer(CountType, ExecutionInteger(WriteCountWidth, 0));
      const auto Zero = Engine.execute(*Read, Arguments);
      ASSERT_TRUE(Zero);
      EXPECT_EQ(Zero.Value.integer().lowWord(), 0U);
      EXPECT_EQ(Address.pointer().kind(), ExecutionPointer::Kind::Native);
      ASSERT_EQ(Heap.release(Storage.Place), ExecutionStatus::Success);
      EXPECT_EQ(Address.pointer().status(), ExecutionStatus::Success);
      // The released address is never passed back to a native memory operation.
    }
  }
} // namespace ink::execution::test
#endif
