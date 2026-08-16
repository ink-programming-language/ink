#include "ink/execution/execution_engine.h"
#include "ink/ir/memory.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct MemoryExecutionTestContext
    {
      MemoryExecutionTestContext() = default;

      explicit MemoryExecutionTestContext(core::TargetContext Target) : Compilation(Target)
      {
      }

      core::CompilationContext Compilation;
      ir::IRContext IR{Compilation};
      ExecutionContext Execution{Compilation};
    };

    std::vector<std::uint8_t> CapturedManagedBytes;
    std::size_t NativePointerCallCount = 0;

    extern "C" std::int32_t readManagedBytes(std::int32_t Descriptor, std::uint8_t *Data, std::size_t Size)
    {
      constexpr std::array<std::uint8_t, 3> Bytes = {0x00, 0x7F, 0xFF};
      if (Descriptor != 0 || (Data == nullptr && Size != 0))
      {
        return -1;
      }
      const std::size_t Count = std::min(Size, Bytes.size());
      if (Count != 0)
      {
        std::copy_n(Bytes.begin(), Count, Data);
      }
      return static_cast<std::int32_t>(Count);
    }

    extern "C" std::int32_t writeManagedBytes(std::int32_t Descriptor, const std::uint8_t *Data, std::size_t Size)
    {
      if (Descriptor != 1 || (Data == nullptr && Size != 0))
      {
        return -1;
      }
      CapturedManagedBytes.clear();
      if (Size != 0)
      {
        CapturedManagedBytes.assign(Data, Data + Size);
      }
      return static_cast<std::int32_t>(Size);
    }

    extern "C" void consumeRawPointer(std::uint8_t *)
    {
      ++NativePointerCallCount;
    }

    ExecutionResult executeText(MemoryExecutionTestContext &Context, const std::string &Text, const std::vector<RuntimeValueRef> &Arguments = {})
    {
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      if (!Parsed.succeeded())
      {
        ADD_FAILURE() << "expected test InkIR to deserialize";
        return {};
      }
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      return Engine.execute("main", Arguments);
    }

    void expectIntegerResult(const ExecutionResult &Result, std::uint64_t Expected)
    {
      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      const std::optional<std::uint64_t> Value = Result.returnValue()->integer();
      ASSERT_TRUE(Value.has_value());
      EXPECT_EQ(*Value, Expected);
    }

    // Verifies that alloca zero-initializes bytes and that pointer-based store followed by load observes the mutation.
    TEST(MemoryExecutionTest, AllocatesZeroInitializedWritableStorage)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 2\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 1\n"
          "  %3 = load byte, byte* %2\n"
          "  store byte 171, byte* %2\n"
          "  %4 = load byte, byte* %2\n"
          "  ret byte %4\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 171);
    }

    // Verifies that bool, byte, i32, and ptrsize load and store widths come from the explicit access type at an unaligned byte-derived pointer.
    TEST(MemoryExecutionTest, RoundTripsAllSupportedScalarTypes)
    {
      struct Case
      {
        const char *Type;
        const char *StoredValue;
        std::uint64_t ExpectedValue;
      };
      const Case Cases[] = {
          {"bool", "1", 1},
          {"byte", "165", 165},
          {"i32", "-1985229329", static_cast<std::uint64_t>(static_cast<std::int64_t>(-1985229329))},
          {"ptrsize", "16909060", 16909060},
      };

      for (const Case &CaseValue : Cases)
      {
        MemoryExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine " + std::string(CaseValue.Type) + " @main() {\nentry:\n  %0 = alloca byte[] ptrsize " + std::to_string(1 + Context.Compilation.targetContext().pointerByteWidth()) + "\n  %1 = slice.data byte* byte[] %0\n  %2 = getelementptr byte, byte* %1, ptrsize 1\n  store " + std::string(CaseValue.Type) + " " + CaseValue.StoredValue + ", byte* %2\n  %3 = load " + CaseValue.Type + ", byte* %2\n  ret " + CaseValue.Type + " %3\n}\n";

        const ExecutionResult Result = executeText(Context, Text);

        expectIntegerResult(Result, CaseValue.ExpectedValue);
      }
    }

    // Verifies through ExecutionEngine that synthetic little- and big-endian targets produce distinct fixed bytes for the same i32 store.
    TEST(MemoryExecutionTest, ExecutesLoadAndStoreWithSyntheticTargetByteOrder)
    {
      struct Case
      {
        core::ByteOrder Order;
        std::uint64_t ExpectedFirstByte;
      };
      const Case Cases[] = {
          {core::ByteOrder::LittleEndian, 4},
          {core::ByteOrder::BigEndian, 1},
      };
      const std::string Text =
          "inkir 1\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 4\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  store i32 16909060, byte* %1\n"
          "  %2 = load byte, byte* %1\n"
          "  ret byte %2\n"
          "}\n";

      for (const Case &CaseValue : Cases)
      {
        MemoryExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits32, CaseValue.Order));
        const ExecutionResult Result = executeText(Context, Text);
        expectIntegerResult(Result, CaseValue.ExpectedFirstByte);
      }
    }

    // Verifies that a synthetic 64-bit target executes and preserves the full unsigned ptrsize payload through the result arena.
    TEST(MemoryExecutionTest, ReturnsMaximumTarget64PointerSize)
    {
      MemoryExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "define ptrsize @main() {\n"
          "entry:\n"
          "  ret ptrsize 18446744073709551615\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, std::numeric_limits<std::uint64_t>::max());
    }

    // Verifies that synthetic targets use logical slice-derived pointers for internal memory access without exposing a host ABI pointer.
    TEST(MemoryExecutionTest, ExecutesLogicalPointerForSyntheticTarget)
    {
      MemoryExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 4\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  store i32 16909060, byte* %1\n"
          "  %2 = load byte, byte* %1\n"
          "  ret byte %2\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 4);
    }

    // Verifies that loading a bool rejects a non-canonical byte representation written through a byte store.
    TEST(MemoryExecutionTest, RejectsInvalidBoolMemoryRepresentation)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define bool @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  store byte 2, byte* %1\n"
          "  %2 = load bool, byte* %1\n"
          "  ret bool %2\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryInvalidRepresentation);
    }

    // Verifies that slice.length exposes the exact allocation length including a zero-length allocation.
    TEST(MemoryExecutionTest, ReportsAllocatedSliceLength)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define ptrsize @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 0\n"
          "  %1 = slice.length byte[] %0\n"
          "  ret ptrsize %1\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 0);
    }

    // Verifies that a const slice borrowed from a different arena can be read without cloning or ownership transfer.
    TEST(MemoryExecutionTest, LoadsFromBorrowedConstSliceEntryArgument)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @main(const byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data const byte* const byte[] %0\n"
          "  %2 = getelementptr byte, const byte* %1, ptrsize 2\n"
          "  %3 = load byte, const byte* %2\n"
          "  ret byte %3\n"
          "}\n";
      const std::array<std::uint8_t, 3> Bytes = {2, 4, 8};
      RuntimeValueArena Arguments;
      RuntimeValueRef Slice = Arguments.byteSliceValue(Context.IR.getType(ir::TypeKind::ConstByteSlice), Bytes.data(), Bytes.size());

      const ExecutionResult Result = executeText(Context, Text, {Slice});

      expectIntegerResult(Result, 8);
    }

    // Verifies that a mutable slice borrowed from a different arena writes directly into its host buffer.
    TEST(MemoryExecutionTest, StoresIntoBorrowedMutableSliceEntryArgument)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main(byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 1\n"
          "  store byte 255, byte* %2\n"
          "  ret void\n"
          "}\n";
      std::array<std::uint8_t, 3> Bytes = {1, 2, 3};
      RuntimeValueArena Arguments;
      RuntimeValueRef Slice = Arguments.mutableByteSliceValue(Context.IR.getType(ir::TypeKind::ByteSlice), Bytes.data(), Bytes.size());

      const ExecutionResult Result = executeText(Context, Text, {Slice});

      ASSERT_TRUE(Result.succeeded());
      EXPECT_EQ(Bytes, (std::array<std::uint8_t, 3>{1, 255, 3}));
    }

    // Verifies that load and store reject both one-past byte pointers and multi-byte ranges that extend past their backing region with a precise bounds diagnostic.
    TEST(MemoryExecutionTest, ReportsLoadAndStoreBoundsFailures)
    {
      struct Case
      {
        const char *Instruction;
        const char *Operation;
        std::size_t SliceLength;
        std::size_t AccessSize;
        std::size_t Index;
      };
      const Case Cases[] = {
          {"%3 = load byte, byte* %2", "load", 1, 1, 1},
          {"store byte 1, byte* %2", "store", 1, 1, 1},
          {"%3 = load i32, byte* %2", "load", 4, 4, 1},
          {"store i32 1, byte* %2", "store", 4, 4, 1},
      };

      for (const Case &CaseValue : Cases)
      {
        MemoryExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n  %0 = alloca byte[] ptrsize " + std::to_string(CaseValue.SliceLength) + "\n  %1 = slice.data byte* byte[] %0\n  %2 = getelementptr byte, byte* %1, ptrsize 1\n  " + std::string(CaseValue.Instruction) + "\n  ret void\n}\n";
        const ExecutionResult Result = executeText(Context, Text);
        ASSERT_FALSE(Result.succeeded()) << CaseValue.Operation;
        ASSERT_EQ(Result.diagnostics().size(), 1u) << CaseValue.Operation;
        EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessOutOfBounds) << CaseValue.Operation;
        EXPECT_EQ(core::DiagnosticFormatter().format(Result.diagnostics()[0]).Message, std::string(CaseValue.Operation) + " in function @main attempted a " + std::to_string(CaseValue.AccessSize) + "-byte access at byte offset " + std::to_string(CaseValue.Index) + " in a memory region with length " + std::to_string(CaseValue.SliceLength));
      }
    }

    // Verifies that a valid 64-bit target index above UINT32_MAX reaches checked memory access and reports out of bounds without host-size narrowing.
    TEST(MemoryExecutionTest, ReportsTarget64LargeIndexAsBoundsFailure)
    {
      struct Case
      {
        const char *Instruction;
        const char *Operation;
      };
      const Case Cases[] = {
          {"%3 = load byte, byte* %2", "load"},
          {"store byte 1, byte* %2", "store"},
      };

      for (const Case &CaseValue : Cases)
      {
        MemoryExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
        const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n  %0 = alloca byte[] ptrsize 1\n  %1 = slice.data byte* byte[] %0\n  %2 = getelementptr byte, byte* %1, ptrsize 4294967296\n  " + std::string(CaseValue.Instruction) + "\n  ret void\n}\n";
        const ExecutionResult Result = executeText(Context, Text);

        ASSERT_FALSE(Result.succeeded()) << CaseValue.Operation;
        ASSERT_EQ(Result.diagnostics().size(), 1u) << CaseValue.Operation;
        EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessOutOfBounds) << CaseValue.Operation;
        EXPECT_EQ(core::DiagnosticFormatter().format(Result.diagnostics()[0]).Message, std::string(CaseValue.Operation) + " in function @main attempted a 1-byte access at byte offset 4294967296 in a memory region with length 1");
      }
    }

    // Verifies that the executor reports its defensive memory diagnostic if a verified module is corrupted before execution.
    TEST(MemoryExecutionTest, ReportsInvalidRuntimeMemoryValueAfterModuleMutation)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = load byte, byte* %1\n"
          "  ret byte %2\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &Load = static_cast<ir::LoadInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[2]);
      Load.Pointer = std::make_unique<ir::IntegerConstant>(Context.IR.getType(ir::TypeKind::Byte), 0);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidRuntimeMemoryValue);
    }

    // Verifies that load and store through a previously derived pointer, plus a late slice.data, all reject a managed allocation after lifetime.end.
    TEST(MemoryExecutionTest, RejectsDataAccessAfterLifetimeEnd)
    {
      struct Case
      {
        const char *BeforeLifetimeEnd;
        const char *AfterLifetimeEnd;
        const char *Operation;
      };
      const Case Cases[] = {
          {"  %1 = slice.data byte* byte[] %0\n", "  %2 = load byte, byte* %1\n", "load"},
          {"  %1 = slice.data byte* byte[] %0\n", "  store byte 1, byte* %1\n", "store"},
          {"", "  %1 = slice.data byte* byte[] %0\n", "slice.data"},
      };

      for (const Case &CaseValue : Cases)
      {
        MemoryExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n  %0 = alloca byte[] ptrsize 1\n" + std::string(CaseValue.BeforeLifetimeEnd) + "  lifetime.end byte[] %0\n" + CaseValue.AfterLifetimeEnd + "  ret void\n}\n";
        const ExecutionResult Result = executeText(Context, Text);
        ASSERT_FALSE(Result.succeeded()) << CaseValue.Operation;
        ASSERT_EQ(Result.diagnostics().size(), 1u) << CaseValue.Operation;
        EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryLifetimeEnded) << CaseValue.Operation;
      }
    }

    // Verifies that ending the same managed lifetime twice reports the second operation instead of silently succeeding.
    TEST(MemoryExecutionTest, RejectsRepeatedLifetimeEnd)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  lifetime.end byte[] %0\n"
          "  lifetime.end byte[] %0\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryLifetimeEnded);
    }

    // Verifies that lifetime.end invalidates storage access while leaving immutable slice length metadata available.
    TEST(MemoryExecutionTest, PreservesSliceLengthMetadataAfterLifetimeEnd)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define ptrsize @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 3\n"
          "  lifetime.end byte[] %0\n"
          "  %1 = slice.length byte[] %0\n"
          "  ret ptrsize %1\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 3);
    }

    // Verifies that lifetime.end cannot claim storage borrowed from the entry caller.
    TEST(MemoryExecutionTest, RejectsLifetimeEndForBorrowedSlice)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main(byte[] %0) {\n"
          "entry:\n"
          "  lifetime.end byte[] %0\n"
          "  ret void\n"
          "}\n";
      std::uint8_t Byte = 0;
      RuntimeValueArena Arguments;
      RuntimeValueRef Slice = Arguments.mutableByteSliceValue(Context.IR.getType(ir::TypeKind::ByteSlice), &Byte, 1);

      const ExecutionResult Result = executeText(Context, Text, {Slice});

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryLifetimeNotOwned);
    }

    // Verifies that an internal callee can mutate caller-owned storage while ownership remains with the caller frame.
    TEST(MemoryExecutionTest, SharesManagedSliceWithInternalCallee)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @set(byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  store byte 42, byte* %1\n"
          "  ret void\n"
          "}\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  call void @set(byte[] %0)\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = load byte, byte* %1\n"
          "  ret byte %2\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 42);
    }

    // Verifies that a managed slice defined in the entry block remains available in a dominated successor block.
    TEST(MemoryExecutionTest, UsesDominatingManagedSliceAcrossBlocks)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  store byte 77, byte* %1\n"
          "  br next\n"
          "next:\n"
          "  %2 = load byte, byte* %1\n"
          "  ret byte %2\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 77);
    }

    // Verifies that a callee cannot end a byte allocation owned by its caller frame.
    TEST(MemoryExecutionTest, RejectsCalleeEndingCallerLifetime)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @end(byte[] %0) {\n"
          "entry:\n"
          "  lifetime.end byte[] %0\n"
          "  ret void\n"
          "}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  call void @end(byte[] %0)\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryLifetimeNotOwned);
    }

    // Verifies that a single alloca larger than 16 MiB fails before allocating host storage.
    TEST(MemoryExecutionTest, ReportsPerAllocationSizeLimit)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 16777217\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAllocationLimitExceeded);
    }

    // Verifies that a 64-bit target alloca size above UINT32_MAX reaches the allocation limiter before any host-size conversion.
    TEST(MemoryExecutionTest, ReportsAllocationLimitBeforeHostSizeNarrowing)
    {
      MemoryExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 4294967296\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAllocationLimitExceeded);
      EXPECT_EQ(core::DiagnosticFormatter().format(Result.diagnostics()[0]).Message, "requested byte allocation size 4294967296 exceeds the per-allocation limit 16777216");
    }

    // Verifies that cumulative storage exhaustion is distinguished from the per-allocation size limit.
    TEST(MemoryExecutionTest, ReportsCumulativeStorageLimit)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 16777216\n"
          "  %1 = alloca byte[] ptrsize 16777216\n"
          "  %2 = alloca byte[] ptrsize 16777216\n"
          "  %3 = alloca byte[] ptrsize 16777216\n"
          "  %4 = alloca byte[] ptrsize 1\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryStorageLimitExceeded);
      EXPECT_EQ(core::DiagnosticFormatter().format(Result.diagnostics()[0]).Message, "a 1-byte allocation would exceed the execution byte-storage limit of 67108864 bytes");
    }

    // Verifies that repeated zero-byte frame allocations stop at the independent allocation-count limit before the instruction limit.
    TEST(MemoryExecutionTest, ReportsAllocationCountLimit)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @allocate() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 0\n"
          "  ret void\n"
          "}\n"
          "define void @main() {\n"
          "entry:\n"
          "  br loop\n"
          "loop:\n"
          "  %0 = phi i32 [0, entry], [%2, body]\n"
          "  %1 = icmp lt i32 %0, i32 65537\n"
          "  condbr bool %1, body, exit\n"
          "body:\n"
          "  call void @allocate()\n"
          "  %2 = add i32 %0, i32 1\n"
          "  br loop\n"
          "exit:\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAllocationCountLimitExceeded);
    }

    // Verifies that a pointer returned from a callee retains allocation provenance and cannot load after the callee frame lifetime ends.
    TEST(MemoryExecutionTest, RejectsLoadThroughPointerReturnedFromExpiredFrame)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte* @local() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  ret byte* %1\n"
          "}\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = call byte* @local()\n"
          "  %1 = load byte, byte* %0\n"
          "  ret byte %1\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryLifetimeEnded);
    }

    // Verifies that returning a raw pointer derived from caller-borrowed storage preserves the caller-owned address.
    TEST(MemoryExecutionTest, ReturnsPointerToBorrowedEntryStorage)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte* @main(byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  ret byte* %1\n"
          "}\n";
      std::uint8_t Byte = 9;
      RuntimeValueArena Arguments;
      RuntimeValueRef Slice = Arguments.mutableByteSliceValue(Context.IR.getType(ir::TypeKind::ByteSlice), &Byte, 1);

      const ExecutionResult Result = executeText(Context, Text, {Slice});

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->mutablePointer(), &Byte);
    }

    // Verifies that an entry function cannot export a pointer to its own managed allocation after the entry frame has ended.
    TEST(MemoryExecutionTest, RejectsPointerToExpiredEntryAllocationAsResult)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte* @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  ret byte* %1\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::UnsupportedRuntimeValueKind);
    }

    // Verifies that an expired managed pointer cannot cross the native ABI boundary as either a stale address or an accidental null pointer.
    TEST(MemoryExecutionTest, RejectsExpiredManagedPointerAtNativeBoundary)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "declare extern \"C\" void @consume(byte*) [sideeffect]\n"
          "define byte* @local() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  ret byte* %1\n"
          "}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = call byte* @local()\n"
          "  call void @consume(byte* %0)\n"
          "  ret void\n"
          "}\n";
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("consume", reinterpret_cast<NativeFunctionAddress>(&consumeRawPointer)));
      NativePointerCallCount = 0;

      const ExecutionResult Result = executeText(Context, Text);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_EQ(NativePointerCallCount, 0u);
    }

    // Verifies that an expired logical pointer preserves identity for equality comparison without dereferencing its backing allocation.
    TEST(MemoryExecutionTest, ComparesExpiredLogicalPointerByIdentity)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte* @local() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  ret byte* %1\n"
          "}\n"
          "define bool @main() {\n"
          "entry:\n"
          "  %0 = call byte* @local()\n"
          "  %1 = icmp eq byte* %0, byte* %0\n"
          "  ret bool %1\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 1);
    }

    // Verifies that slice.data accepts a live managed entry slice owned by the caller and passes its raw address to native code.
    TEST(MemoryExecutionTest, PassesCallerManagedSliceDataAtNativeBoundary)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "declare extern \"C\" void @consume(byte*) [sideeffect]\n"
          "define void @main(byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  call void @consume(byte* %1)\n"
          "  ret void\n"
          "}\n";
      RuntimeValueArena Arguments;
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::InvalidValue;
      const RuntimeValueRef Slice = Arguments.allocateByteSlice(Context.IR.getType(ir::TypeKind::ByteSlice), 1, 901, Status);
      ASSERT_EQ(Status, RuntimeMemoryStatus::Ok);
      ASSERT_NE(Slice, nullptr);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("consume", reinterpret_cast<NativeFunctionAddress>(&consumeRawPointer)));
      NativePointerCallCount = 0;

      const ExecutionResult Result = executeText(Context, Text, {Slice});

      EXPECT_TRUE(Result.succeeded());
      EXPECT_EQ(NativePointerCallCount, 1u);
      EXPECT_TRUE(Slice->memoryAlive());
    }

    // Verifies that alloca buffers can cross the explicit raw-pointer FFI boundary for native read and write calls.
    TEST(MemoryExecutionTest, UsesManagedBufferWithNativeReadAndWrite)
    {
      MemoryExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "declare extern \"C\" i32 @read(i32, byte*, ptrsize) [sideeffect]\n"
          "declare extern \"C\" i32 @write(i32, const byte*, ptrsize) [sideeffect]\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 3\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = slice.length byte[] %0\n"
          "  %3 = call i32 @read(i32 0, byte* %1, ptrsize %2)\n"
          "  %4 = slice.data const byte* byte[] %0\n"
          "  %5 = call i32 @write(i32 1, const byte* %4, ptrsize %2)\n"
          "  %6 = getelementptr byte, byte* %1, ptrsize 2\n"
          "  %7 = load byte, byte* %6\n"
          "  ret byte %7\n"
          "}\n";
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("read", reinterpret_cast<NativeFunctionAddress>(&readManagedBytes)));
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("write", reinterpret_cast<NativeFunctionAddress>(&writeManagedBytes)));
      CapturedManagedBytes.clear();

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 255);
      EXPECT_EQ(CapturedManagedBytes, (std::vector<std::uint8_t>{0x00, 0x7F, 0xFF}));
    }
  } // namespace
} // namespace ink::execution
