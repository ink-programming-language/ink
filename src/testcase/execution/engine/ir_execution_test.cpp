#include "../ffi/external_call_test_support.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/memory/execution_heap.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace ink::execution::test
{
  namespace
  {
    class IRExecutionContext final
    {
      public:
        explicit IRExecutionContext(ExecutionLimits Limits = {})
            : Context(Compilation),
              Builder(Context),
              Engine(Context, Limits),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        ExecutionValueRef integer(std::uint64_t Bits)
        {
          return Engine.heap().integer(Int32, ExecutionInteger(32, Bits));
        }

        const ir::IntegerConstant &constant(std::uint64_t Bits)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Bits));
        }

        std::unique_ptr<ir::Function> function(std::string_view Name, const ir::Type &ReturnType, std::span<const ir::Type *const> Parameters = {})
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Name), *Signature) : nullptr;
        }

        bool begin(ir::Function &Function)
        {
          auto *Body = Builder.createFunctionBody(Function);
          return Body && Builder.setInsertPoint(*Body);
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
    };

    void expectInteger(const ExecutionValueResult &Result, std::uint64_t Expected, std::uint32_t Width = 32)
    {
      ASSERT_TRUE(Result);
      ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Integer);
      const auto Bits = Result.Value.integer().bits();
      EXPECT_EQ(Bits.bitWidth(), Width);
      ASSERT_EQ(Bits.words().size(), 1U);
      EXPECT_EQ(Bits.words().front(), Expected);
    }
  } // namespace

  // Runtime parameters flow through memory and wrapping arithmetic with independent storage and no new pooled constants.
  TEST(IRExecutionTest, ExecutesMemoryInstructionsAndKeepsReturnedValuesIndependent)
  {
    IRExecutionContext Test;
    const ir::Type *Parameters[] = {&Test.Int32, &Test.Int32};
    auto Function = Test.function("AddLocals", Test.Int32, Parameters);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, *Function->parameters()[0]), nullptr);
    auto *First = Test.Builder.createLoadInstruction(*Slot);
    ASSERT_NE(First, nullptr);
    auto *Sum = Test.Builder.createAddInstruction(*First, *Function->parameters()[1]);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, *Sum), nullptr);
    auto *Stored = Test.Builder.createLoadInstruction(*Slot);
    ASSERT_NE(Stored, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Stored), nullptr);
    ExecutionValueRef Arguments[] = {Test.integer(7), Test.integer(9)};
    const auto ConstantCount = Test.Context.constantPool().size();
    const auto Result = Test.Engine.execute(*Function, Arguments);
    expectInteger(Result, 16);
    Arguments[0] = Test.integer(0x7fffffff);
    Arguments[1] = Test.integer(1);
    expectInteger(Test.Engine.execute(*Function, Arguments), 0x80000000);
    expectInteger(Result, 16);
    EXPECT_EQ(Test.Context.constantPool().size(), ConstantCount);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 3U);
  }

  // A load instruction keeps the value observed at its execution point after a later store replaces the same slot.
  TEST(IRExecutionTest, LoadResultsRemainSnapshotsAfterLaterStores)
  {
    IRExecutionContext Test;
    auto Function = Test.function("Snapshot", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, Test.constant(11)), nullptr);
    auto *Snapshot = Test.Builder.createLoadInstruction(*Slot);
    ASSERT_NE(Snapshot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, Test.constant(99)), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Snapshot), nullptr);
    expectInteger(Test.Engine.execute(*Function), 11);
  }

  // Loading an allocated runtime slot before any store reports an explicit uninitialized-value error.
  TEST(IRExecutionTest, RejectsUninitializedLoad)
  {
    IRExecutionContext Test;
    auto Function = Test.function("ReadEmpty", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Slot, nullptr);
    auto *Loaded = Test.Builder.createLoadInstruction(*Slot);
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::Uninitialized);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

  // A returned stack place retains its identity but becomes unreadable after the allocating function exits.
  TEST(IRExecutionTest, RejectsLoadThroughEscapedStackPlace)
  {
    IRExecutionContext Test;
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    ASSERT_NE(Pointer, nullptr);
    auto Escape = Test.function("Escape", *Pointer);
    ASSERT_NE(Escape, nullptr);
    ASSERT_TRUE(Test.begin(*Escape));
    auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, Test.constant(41)), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Slot), nullptr);
    const ir::Type *Parameters[] = {Pointer};
    auto Read = Test.function("Read", Test.Int32, Parameters);
    ASSERT_NE(Read, nullptr);
    ASSERT_TRUE(Test.begin(*Read));
    auto *Loaded = Test.Builder.createLoadInstruction(*Read->parameters().front());
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    const auto Escaped = Test.Engine.execute(*Escape);
    ASSERT_TRUE(Escaped);
    ASSERT_EQ(Escaped.Value.kind(), ExecutionValueKind::Pointer);
    EXPECT_EQ(Escaped.Value.pointer().kind(), ExecutionPointer::Kind::Place);
    const ExecutionValueRef Arguments[] = {Escaped.Value};
    EXPECT_EQ(Test.Engine.execute(*Read, Arguments).Status, ExecutionStatus::ExpiredPlace);
  }

  // Entry execution validates body availability, argument count, exact types and context ownership before interpreting instructions.
  TEST(IRExecutionTest, RejectsInvalidEntryBoundaries)
  {
    IRExecutionContext Test;
    IRExecutionContext Other;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto Missing = Test.function("Missing", Test.Int32, Parameters);
    auto Identity = Test.function("Identity", Test.Int32, Parameters);
    auto Foreign = Other.function("Foreign", Other.Int32);
    ASSERT_NE(Missing, nullptr);
    ASSERT_NE(Identity, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_TRUE(Test.begin(*Identity));
    ASSERT_NE(Test.Builder.createReturnInstruction(Identity->parameters().front().get()), nullptr);
    const ExecutionValueRef Arguments[] = {Test.integer(3)};
    const ExecutionValueRef Wrong[] = {Test.Engine.heap().boolean(Test.Context.typePool().getType<ir::TypeKind::Bool>(), true)};
    const ExecutionValueRef ForeignArguments[] = {Other.integer(3)};
    const ExecutionValueRef Invalid[] = {ExecutionValueRef{}};
    EXPECT_EQ(Test.Engine.execute(*Missing, Arguments).Status, ExecutionStatus::MissingBody);
    EXPECT_EQ(Test.Engine.execute(*Identity).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Test.Engine.execute(*Identity, Invalid).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Test.Engine.execute(*Identity, Wrong).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.execute(*Identity, ForeignArguments).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.execute(*Foreign).Status, ExecutionStatus::ForeignContext);
    expectInteger(Test.Engine.execute(*Identity, Arguments), 3);
  }

  // Recursive IR calls consume the shared dynamic call-depth budget and permanently stop further execution when exhausted.
  TEST(IRExecutionTest, RecursiveIRCallsRespectTheSharedDepthBudget)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 3;
    IRExecutionContext Test(Limits);
    auto Function = Test.function("Recurse", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Call = Test.Builder.createCallInstruction(*Function);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::BudgetExceeded);
  }

  // A cancelled engine rejects a valid entry before evaluating even its constant return.
  TEST(IRExecutionTest, CancellationPreventsEntryEvaluation)
  {
    IRExecutionContext Test;
    auto Function = Test.function("Entry", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(7)), nullptr);
    Test.Engine.cancel();
    EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::Cancelled);
  }

  // Ending a parent frame physically reclaims its local and descendant cells while preserving loaded snapshots.
  TEST(IRExecutionTest, FrameExitImmediatelyReclaimsLocalStorage)
  {
    IRExecutionContext Test;
    auto *Parent = Test.Engine.createFrame(ExecutionFrameKind::Analysis);
    ASSERT_NE(Parent, nullptr);
    auto *Child = Test.Engine.createFrame(ExecutionFrameKind::Block, Parent);
    ASSERT_NE(Child, nullptr);
    int ParentBinding = 0;
    int ChildBinding = 0;
    const auto ParentCell = Test.Engine.allocate(*Parent, &ParentBinding, Test.Int32, true, &Test.constant(11));
    const auto ChildCell = Test.Engine.allocate(*Child, &ChildBinding, Test.Int32, true, &Test.constant(23));
    ASSERT_TRUE(ParentCell);
    ASSERT_TRUE(ChildCell);
    auto Snapshot = Test.Engine.loadValue(ChildCell.Place);
    ASSERT_TRUE(Snapshot);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 2U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 2U);
    ASSERT_EQ(Test.Engine.endFrame(*Parent), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 1U);
    EXPECT_EQ(Test.Engine.loadValue(ParentCell.Place).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Engine.loadValue(ChildCell.Place).Status, ExecutionStatus::ExpiredPlace);
    expectInteger(Snapshot, 23);
    Snapshot.Value = {};
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

  // Reconstructing an engine at the same host address cannot make an old place refer to its new first cell.
  TEST(IRExecutionTest, RejectsPlacesFromAnEngineRebuiltAtTheSameAddress)
  {
    IRExecutionContext Test;
    std::optional<ExecutionEngine> Owner;
    Owner.emplace(Test.Context);
    const ExecutionEngine *OriginalAddress = &*Owner;
    auto *FirstFrame = Owner->createFrame(ExecutionFrameKind::Analysis);
    ASSERT_NE(FirstFrame, nullptr);
    int Binding = 0;
    const auto Old = Owner->allocate(*FirstFrame, &Binding, Test.Int32, true, &Test.constant(3));
    ASSERT_TRUE(Old);
    Owner.reset();
    Owner.emplace(Test.Context);
    EXPECT_EQ(&*Owner, OriginalAddress);
    auto *NewFrame = Owner->createFrame(ExecutionFrameKind::Analysis);
    ASSERT_NE(NewFrame, nullptr);
    const auto Current = Owner->allocate(*NewFrame, &Binding, Test.Int32, true, &Test.constant(19));
    ASSERT_TRUE(Current);
    EXPECT_EQ(Owner->loadValue(Old.Place).Status, ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Owner->store(Old.Place, Test.constant(5)), ExecutionStatus::InvalidPlace);
    expectInteger(Owner->loadValue(Current.Place), 19);
  }

  // A returned scalar stays usable after its execution engine dies while its IR context and type remain alive.
  TEST(IRExecutionTest, ReturnedScalarOutlivesItsEngine)
  {
    IRExecutionContext Test;
    auto Function = Test.function("RetainedResult", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(67)), nullptr);
    ExecutionValueResult Result;
    {
      ExecutionEngine Temporary(Test.Context);
      Result = Temporary.execute(*Function);
      ASSERT_TRUE(Result);
      EXPECT_EQ(Temporary.heap().liveStorageCount(), 0U);
    }
    expectInteger(Result, 67);
    EXPECT_EQ(Result.Value.toConstant(Test.Context), &Test.constant(67));
  }

#if defined(_WIN32) || defined(__linux__)
  // Exhausting the heap during FFI string marshalling permanently stops later allocation-free calls before their native writes.
  TEST(IRExecutionTest, ExternalBufferBudgetExhaustionPermanentlyStopsExecution)
  {
    for (const std::size_t MaxObjects : {0U, 1U})
    {
      NativePipe Pipe;
      ASSERT_TRUE(Pipe.valid());
      ExecutionLimits Limits;
      Limits.MaxObjects = MaxObjects;
      IRExecutionContext Test(Limits);
      if (MaxObjects != 0)
      {
        const auto Exhausted = Test.Engine.heap().allocateCell(Test.Int32);
        ASSERT_TRUE(Exhausted);
        ASSERT_EQ(Test.Engine.heap().release(Exhausted.Place), ExecutionStatus::Success);
      }
      const auto *UInt8 = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto *BufferType = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*UInt8, ir::AccessKind::ReadWrite);
      const auto *CountType = Test.Context.typePool().getType<ir::TypeKind::Integer>(WriteCountWidth, false);
      const auto *ReturnType = Test.Context.typePool().getType<ir::TypeKind::Integer>(WriteReturnWidth, true);
      const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*UInt8, ir::AccessKind::ReadOnly);
      const ir::Type *Parameters[] = {&Test.Int32, BufferType, CountType};
      const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(*ReturnType, Parameters);
      ASSERT_NE(Signature, nullptr);
      auto Write = Test.Builder.createFunction(Test.Context.namePool().intern(WriteSymbol), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C);
      ASSERT_NE(Write, nullptr);
      ExecutionValueRef Arguments[] = {Test.integer(Pipe.writer()), Test.Engine.heap().string(*Slice, "X"), Test.Engine.heap().integer(*CountType, ExecutionInteger(WriteCountWidth, 1))};
      EXPECT_EQ(Test.Engine.execute(*Write, Arguments).Status, ExecutionStatus::BudgetExceeded);
      EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
      char NativeByte = 'Y';
      Arguments[1] = Test.Engine.heap().pointer(*BufferType, ExecutionPointer::fromNative(&NativeByte));
      ASSERT_TRUE(Arguments[1].valid());
      EXPECT_EQ(Test.Engine.execute(*Write, Arguments).Status, ExecutionStatus::BudgetExceeded);
      auto Scalar = Test.function("ConstantAfterBudgetExhaustion", Test.Int32);
      ASSERT_NE(Scalar, nullptr);
      ASSERT_TRUE(Test.begin(*Scalar));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(7)), nullptr);
      EXPECT_EQ(Test.Engine.execute(*Scalar).Status, ExecutionStatus::BudgetExceeded);
      std::string Output;
      ASSERT_TRUE(Pipe.readAll(Output));
      EXPECT_TRUE(Output.empty());
    }
  }

  // Calling a function-valued parameter executes one native write even when the same call instruction is used by both add operands.
  TEST(IRExecutionTest, IndirectCallResultIsExecutedOnceAndReusedByIdentity)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    IRExecutionContext Test;
    const auto *UInt8 = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *BufferType = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*UInt8, ir::AccessKind::ReadWrite);
    const auto *CountType = Test.Context.typePool().getType<ir::TypeKind::Integer>(WriteCountWidth, false);
    const auto *ReturnType = Test.Context.typePool().getType<ir::TypeKind::Integer>(WriteReturnWidth, true);
    const ir::Type *WriteParameters[] = {&Test.Int32, BufferType, CountType};
    const auto *WriteType = Test.Context.typePool().getType<ir::TypeKind::Function>(*ReturnType, WriteParameters);
    ASSERT_NE(WriteType, nullptr);
    auto Write = Test.Builder.createFunction(Test.Context.namePool().intern(WriteSymbol), *WriteType, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C);
    ASSERT_NE(Write, nullptr);
    const ir::Type *Parameters[] = {WriteType, &Test.Int32};
    auto Function = Test.function("CallIndirect", *ReturnType, Parameters);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*UInt8, ir::AccessKind::ReadOnly);
    const auto *Text = Test.Context.constantPool().getStringConstant(*Slice, "X");
    ASSERT_NE(Text, nullptr);
    auto *Buffer = Test.Builder.createCStringInstruction(*Text);
    ASSERT_NE(Buffer, nullptr);
    const auto *Count = Test.Context.constantPool().getIntegerConstant(*CountType, ir::IntegerBits(WriteCountWidth, 1));
    ASSERT_NE(Count, nullptr);
    const ir::Value *CallArguments[] = {Function->parameters()[1].get(), Buffer, Count};
    auto *Call = Test.Builder.createCallInstruction(*Function->parameters()[0], CallArguments);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), nullptr);
    auto *Sum = Test.Builder.createAddInstruction(*Call, *Call);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    const ExecutionValueRef Arguments[] = {Test.Engine.heap().function(*Write), Test.integer(Pipe.writer())};
    expectInteger(Test.Engine.execute(*Function, Arguments), 2, WriteReturnWidth);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "X");
  }

  // Native writes to a CString buffer are visible to a following IR load while the originating string constant stays unchanged.
  TEST(IRExecutionTest, LoadsNativeCStringMutationsWithoutChangingConstants)
  {
    IRExecutionContext Test;
    const auto *UInt8 = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *UInt32 = Test.Context.typePool().getType<ir::TypeKind::Integer>(32, false);
    const auto *BufferType = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*UInt8, ir::AccessKind::ReadWrite);
    const ir::Type *Parameters[] = {BufferType};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(*UInt32, Parameters);
    ASSERT_NE(Signature, nullptr);
    auto Mutate = Test.Builder.createFunction(Test.Context.namePool().intern("inkTestExternalMutate"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C);
    ASSERT_NE(Mutate, nullptr);
    auto Function = Test.function("LoadMutated", *UInt8);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*UInt8, ir::AccessKind::ReadOnly);
    const auto *Text = Test.Context.constantPool().getStringConstant(*Slice, "ABC");
    ASSERT_NE(Text, nullptr);
    auto *Buffer = Test.Builder.createCStringInstruction(*Text);
    ASSERT_NE(Buffer, nullptr);
    const ir::Value *Arguments[] = {Buffer};
    ASSERT_NE(Test.Builder.createCallInstruction(*Mutate, Arguments), nullptr);
    auto *Loaded = Test.Builder.createLoadInstruction(*Buffer);
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    expectInteger(Test.Engine.execute(*Function), 'Z', 8);
    EXPECT_EQ(Text->value(), "ABC");
  }
#endif
} // namespace ink::execution::test
