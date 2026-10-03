#include "../artifact/artifact_test_support.h"
#include "ink/execution/bytecode/execution_compiler.h"

#include <gtest/gtest.h>
#include <llvm/Support/Endian.h>

#include <array>

namespace ink::execution::test
{
  // Array construction keeps source slot order, repetition uses one operand, and element addresses disable local-cell elision.
  TEST(ArrayBytecodeTest, LowersArrayOperationsAndEscapingElementAddresses)
  {
    ArtifactContext Test;
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 2);
    const ir::Type *Parameters[] = {&Test.Int32};
    auto *Function = Test.function("array", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Elements[] = {Function->parameters()[0].get(), &Test.integer(7)};
    const auto *Construct = Test.Builder.createArrayInstruction(*Array, Elements);
    const ir::Value *Repeated[] = {Function->parameters()[0].get()};
    const auto *Repeat = Test.Builder.createArrayInstruction(*Array, Repeated, true);
    const auto *Storage = Test.Builder.createAllocaInstruction(*Array);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Construct), nullptr);
    const auto *Address = Test.Builder.createArrayElementPointerInstruction(*Storage, Test.integer(1));
    const auto *Extract = Test.Builder.createArrayExtractInstruction(*Repeat, Test.integer(0));
    ASSERT_NE(Address, nullptr);
    ASSERT_NE(Extract, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Extract), nullptr);
    auto Compiled = ExecutionCompiler{}.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    const auto &Code = Compiled.Function->Code;
    ASSERT_EQ(Code.size(), 7U);
    EXPECT_EQ(Code[0].Code, BytecodeOpcode::Array);
    EXPECT_EQ(Code[1].Code, BytecodeOpcode::ArrayRepeat);
    EXPECT_EQ(Code[2].Code, BytecodeOpcode::Alloca);
    EXPECT_EQ(Code[4].Code, BytecodeOpcode::ArrayElementPointer);
    EXPECT_EQ(Code[5].Code, BytecodeOpcode::ArrayExtract);
    EXPECT_EQ(Code[0].Operands[2], 8U);
    const auto *Slots = Compiled.Function->ConstantData.data() + Code[0].Operands[1];
    EXPECT_EQ(llvm::support::endian::read32le(Slots), 0U);
    const auto Second = llvm::support::endian::read32le(Slots + 4);
    EXPECT_EQ(Compiled.Function->InitialSlots[Second].Bits, 7U);
    EXPECT_EQ(Code[1].Operands[1], 0U);
    EXPECT_EQ(ExecutionCompiler{}.verify(*Compiled.Function), ExecutionStatus::Success);
  }

  // Corrupt array source tables, mismatched operand slots and non-integer indices are rejected before VM dispatch.
  TEST(ArrayBytecodeTest, VerifierRejectsInvalidArraySlotsTypesAndSourceTables)
  {
    ArtifactContext Test;
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 2);
    const ir::Type *Parameters[] = {&Test.Int32, &Test.Bool};
    auto *Function = Test.function("verify", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Elements[] = {Function->parameters()[0].get(), &Test.integer(7)};
    const auto *Construct = Test.Builder.createArrayInstruction(*Array, Elements);
    const ir::Value *Repeated[] = {Function->parameters()[0].get()};
    ASSERT_NE(Test.Builder.createArrayInstruction(*Array, Repeated, true), nullptr);
    const auto *Storage = Test.Builder.createAllocaInstruction(*Array);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Construct), nullptr);
    ASSERT_NE(Test.Builder.createArrayElementPointerInstruction(*Storage, Test.integer(0)), nullptr);
    const auto *Extract = Test.Builder.createArrayExtractInstruction(*Construct, Test.integer(1));
    ASSERT_NE(Test.Builder.createReturnInstruction(Extract), nullptr);
    auto Compiled = ExecutionCompiler{}.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    const auto OriginalCode = Image.Code;
    const auto OriginalData = Image.ConstantData;
    const auto Reject = [&]()
    {
      EXPECT_NE(ExecutionCompiler{}.verify(Image), ExecutionStatus::Success);
      Image.Code = OriginalCode;
      Image.ConstantData = OriginalData;
    };
    for (const std::size_t Position : {0U, 1U, 4U, 5U})
    {
      Image.Code[Position].Operands[0] = InvalidSlot;
      Reject();
      Image.Code[Position].Operands[0] = 1;
      Reject();
    }
    Image.Code[0].Operands[1] = static_cast<std::uint32_t>(Image.ConstantData.size() + 1);
    Reject();
    Image.Code[0].Operands[2] = 7;
    Reject();
    Image.Code[0].Operands[2] = 4;
    Reject();
    llvm::support::endian::write32le(Image.ConstantData.data() + OriginalCode[0].Operands[1], InvalidSlot);
    Reject();
    llvm::support::endian::write32le(Image.ConstantData.data() + OriginalCode[0].Operands[1], 1);
    Reject();
    Image.Code[1].Operands[1] = 1;
    Reject();
    Image.Code[4].Operands[1] = 0;
    Reject();
    Image.Code[4].Operands[2] = 1;
    Reject();
    Image.Code[5].Operands[1] = 0;
    Reject();
    Image.Code[5].Operands[2] = 1;
    Reject();
    EXPECT_EQ(ExecutionCompiler{}.verify(Image), ExecutionStatus::Success);
  }

  // Signed, unsigned and wide dynamic indices share strict bounds checks for both value extraction and writable addresses.
  TEST(ArrayBytecodeTest, ChecksNegativeWideAndOutOfRangeIndices)
  {
    for (const bool Address : {false, true})
    {
      for (const std::uint32_t Width : {8U, 64U, 128U})
      {
        for (const bool Signed : {false, true})
        {
          ArtifactContext Test;
          ExecutionEngine Engine(Test.Context);
          const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 2);
          const auto *IndexType = Test.Context.typePool().getType<ir::TypeKind::Integer>(Width, Signed);
          const ir::Type *Parameters[] = {IndexType};
          auto *Function = Test.function("index", Test.Int32, Parameters);
          ASSERT_TRUE(Test.begin(*Function));
          const ir::Constant *Elements[] = {&Test.integer(19), &Test.integer(23)};
          const auto *Constant = Test.Context.constantPool().getArrayConstant(*Array, Elements);
          const ir::Value *Value = nullptr;
          if (Address)
          {
            const auto *Storage = Test.Builder.createAllocaInstruction(*Array);
            ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Constant), nullptr);
            const auto *Pointer = Test.Builder.createArrayElementPointerInstruction(*Storage, *Function->parameters()[0]);
            Value = Test.Builder.createLoadInstruction(*Pointer);
          }
          else
          {
            Value = Test.Builder.createArrayExtractInstruction(*Constant, *Function->parameters()[0]);
          }
          ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
          for (const std::uint64_t Index : {0ULL, 1ULL, 2ULL, 255ULL})
          {
            const ExecutionValueRef Arguments[] = {Engine.heap().integer(*IndexType, ExecutionInteger(Width, Index))};
            const auto Result = Engine.execute(*Function, Arguments);
            if (Index < 2)
            {
              ASSERT_TRUE(Result);
              EXPECT_EQ(Result.Value.integer().lowWord(), Index ? 23U : 19U);
            }
            else
            {
              EXPECT_EQ(Result.Status, ExecutionStatus::IndexOutOfBounds);
            }
          }
          if (Width == 128)
          {
            const std::uint64_t Words[] = {0, Signed ? (std::uint64_t{1} << 63) : 1};
            const ExecutionValueRef Arguments[] = {Engine.heap().integer(*IndexType, ExecutionInteger(ir::IntegerBits(128, Words)))};
            EXPECT_EQ(Engine.execute(*Function, Arguments).Status, ExecutionStatus::IndexOutOfBounds);
          }
          EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
        }
      }
    }
  }

  // Arrays returned by value keep their original elements after a mutable copy changes through an indexed address.
  TEST(ArrayBytecodeTest, ArrayCopiesRemainIndependentOfIndexedMutation)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 2);
    auto *Function = Test.function("copy", *Array);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Elements[] = {&Test.integer(19), &Test.integer(23)};
    const auto *Original = Test.Builder.createArrayInstruction(*Array, Elements);
    const auto *Storage = Test.Builder.createAllocaInstruction(*Array);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Original), nullptr);
    const auto *Snapshot = Test.Builder.createLoadInstruction(*Storage);
    const auto *Pointer = Test.Builder.createArrayElementPointerInstruction(*Storage, Test.integer(0));
    ASSERT_NE(Test.Builder.createStoreInstruction(*Pointer, Test.integer(42)), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Snapshot), nullptr);
    const auto Result = Engine.execute(*Function);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.array().size(), 2U);
    EXPECT_EQ(Result.Value.array()[0].integer().lowWord(), 19U);
    EXPECT_EQ(Result.Value.array()[1].integer().lowWord(), 23U);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }

  // A returned array cannot conceal a pointer into the just-ended function activation.
  TEST(ArrayBytecodeTest, RejectsEscapedLocalPointerInsideArray)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(*Pointer, 1);
    auto *Function = Test.function("escape", *Array);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Storage = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, Test.integer(42)), nullptr);
    const ir::Value *Elements[] = {Storage};
    const auto *Result = Test.Builder.createArrayInstruction(*Array, Elements);
    ASSERT_NE(Test.Builder.createReturnInstruction(Result), nullptr);
    EXPECT_EQ(Engine.execute(*Function).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }

  // Empty repeated arrays still evaluate their initializer once and accept a single operand without materializing elements.
  TEST(ArrayBytecodeTest, EmptyRepeatEvaluatesInitializerExactlyOnce)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    const ir::Type *Parameters[] = {Pointer};
    auto *Initializer = Test.function("increment", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Initializer));
    const auto *Old = Test.Builder.createLoadInstruction(*Initializer->parameters()[0]);
    const auto *Next = Test.Builder.createAddInstruction(*Old, Test.integer(1));
    ASSERT_NE(Test.Builder.createStoreInstruction(*Initializer->parameters()[0], *Next), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Next), nullptr);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *Counter = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Counter, Test.integer(0)), nullptr);
    const ir::Value *Arguments[] = {Counter};
    const auto *Call = Test.Builder.createCallInstruction(*Initializer, Arguments);
    const ir::Value *Repeated[] = {Call};
    const auto *Empty = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 0);
    ASSERT_NE(Test.Builder.createArrayInstruction(*Empty, Repeated, true), nullptr);
    const auto *Result = Test.Builder.createLoadInstruction(*Counter);
    ASSERT_NE(Test.Builder.createReturnInstruction(Result), nullptr);
    const auto Executed = Engine.execute(*Main);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.integer().lowWord(), 1U);
  }
} // namespace ink::execution::test
