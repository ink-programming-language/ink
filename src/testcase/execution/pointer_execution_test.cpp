#include "ink/execution/execution_engine.h"
#include "ink/ir/memory.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace ink::execution
{
  namespace
  {
    struct PointerExecutionTestContext
    {
      PointerExecutionTestContext() = default;

      explicit PointerExecutionTestContext(core::TargetContext Target) : Compilation(Target)
      {
      }

      core::CompilationContext Compilation;
      ir::IRContext IR{Compilation};
      ExecutionContext Execution{Compilation};
    };

    ExecutionResult executePointerText(PointerExecutionTestContext &Context, const std::string &Text)
    {
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      if (!Parsed.succeeded())
      {
        ADD_FAILURE() << "expected pointer test InkIR to deserialize";
        return {};
      }
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      return Engine.execute("main");
    }

    void expectPointerIntegerResult(const ExecutionResult &Result, std::uint64_t Expected)
    {
      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      const std::optional<std::uint64_t> Value = Result.returnValue()->integer();
      ASSERT_TRUE(Value.has_value());
      EXPECT_EQ(*Value, Expected);
    }

    // Verifies that every element type with a runtime layout advances GEP by exactly twice its target-specific stride.
    TEST(PointerExecutionTest, ExecutesEverySupportedRootElementTypeFormat)
    {
      struct Case
      {
        const char *ElementType;
        std::uint64_t ExpectedByteOffset;
      };
      const Case Cases[] = {
          {"bool", 2},
          {"byte", 2},
          {"f16", 4},
          {"i32", 8},
          {"f32", 8},
          {"f64", 16},
          {"ptrsize", 16},
          {"byte*", 16},
          {"const byte*", 16},
          {"%Pair", 16},
      };

      for (const Case &CaseValue : Cases)
      {
        PointerExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
        const std::string Text = "inkir 1\n%Pair = type {byte, i32}\ndefine bool @main() {\nentry:\n  %0 = alloca byte[] ptrsize 24\n  %1 = slice.data byte* byte[] %0\n  %2 = getelementptr " + std::string(CaseValue.ElementType) + ", byte* %1, ptrsize 2\n  %3 = getelementptr byte, byte* %1, ptrsize " + std::to_string(CaseValue.ExpectedByteOffset) + "\n  %4 = icmp eq byte* %2, byte* %3\n  ret bool %4\n}\n";

        const ExecutionResult Result = executePointerText(Context, Text);

        ASSERT_TRUE(Result.succeeded()) << CaseValue.ElementType;
        ASSERT_NE(Result.returnValue(), nullptr) << CaseValue.ElementType;
        EXPECT_EQ(Result.returnValue()->integer(), 1U) << CaseValue.ElementType;
      }
    }

    // Verifies zeroinitializer in the root-index and both pointer operand forms, plus the zero-valued first struct-field index form, through ExecutionEngine.
    TEST(PointerExecutionTest, ExecutesZeroInitializerAndFirstStructFieldFormats)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define bool @zero_index() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize zeroinitializer\n"
          "  %3 = icmp eq byte* %1, byte* %2\n"
          "  ret bool %3\n"
          "}\n"
          "define bool @mutable_zero_pointer() {\n"
          "entry:\n"
          "  %0 = getelementptr byte, byte* zeroinitializer, ptrsize 0\n"
          "  %1 = icmp eq byte* %0, byte* null\n"
          "  ret bool %1\n"
          "}\n"
          "define bool @const_zero_pointer() {\n"
          "entry:\n"
          "  %0 = getelementptr byte, const byte* zeroinitializer, ptrsize 0\n"
          "  %1 = icmp eq const byte* %0, const byte* null\n"
          "  ret bool %1\n"
          "}\n"
          "define bool @first_field() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 16\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Pair, byte* %1, ptrsize 1, i32 0\n"
          "  %3 = getelementptr byte, byte* %1, ptrsize 8\n"
          "  %4 = icmp eq byte* %2, byte* %3\n"
          "  ret bool %4\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult ZeroIndex = Engine.execute("zero_index");
      const ExecutionResult MutableZeroPointer = Engine.execute("mutable_zero_pointer");
      const ExecutionResult ConstZeroPointer = Engine.execute("const_zero_pointer");
      const ExecutionResult FirstField = Engine.execute("first_field");

      expectPointerIntegerResult(ZeroIndex, 1);
      expectPointerIntegerResult(MutableZeroPointer, 1);
      expectPointerIntegerResult(ConstZeroPointer, 1);
      expectPointerIntegerResult(FirstField, 1);
    }

    // Verifies that GEP uses the padded eight-byte stride instead of the five-byte struct size and that a whole struct can be loaded from the derived element address.
    TEST(PointerExecutionTest, UsesPaddedStructStrideForWholeSecondElementLoad)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {i32, byte}\n"
          "define bool @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 16\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Pair, byte* %1, ptrsize 1\n"
          "  store %Pair {i32 16909060, byte 7}, byte* %2\n"
          "  %3 = load %Pair, byte* %2\n"
          "  %4 = extractvalue %Pair %3, 0\n"
          "  %5 = getelementptr byte, byte* %1, ptrsize 8\n"
          "  %6 = load i32, byte* %5\n"
          "  %7 = icmp eq i32 %4, i32 %6\n"
          "  ret bool %7\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 1);
    }

    // Verifies that a struct field GEP uses the field layout offset rather than treating the field number as a byte offset.
    TEST(PointerExecutionTest, AddressesPaddedStructFieldByLayoutOffset)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Pair, byte* %1, ptrsize 0, i32 1\n"
          "  %3 = getelementptr byte, byte* %1, ptrsize 4\n"
          "  store i32 16909060, byte* %3\n"
          "  %4 = load i32, byte* %2\n"
          "  ret i32 %4\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 16909060);
    }

    // Verifies that storing through a padded struct field pointer does not overwrite the padding bytes before that field.
    TEST(PointerExecutionTest, PreservesPaddingBeforeDirectStructFieldStore)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 1\n"
          "  store byte 165, byte* %2\n"
          "  %3 = getelementptr %Pair, byte* %1, ptrsize 0, i32 1\n"
          "  store i32 9, byte* %3\n"
          "  %4 = load byte, byte* %2\n"
          "  ret byte %4\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 165);
    }

    // Verifies that a dynamic outer element index and two nested field indices compose the outer stride with both struct field offsets.
    TEST(PointerExecutionTest, AddressesNestedStructFieldInDynamicOuterElement)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {byte, i32}\n"
          "%Outer = type {byte, %Inner, f64}\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 36\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = add ptrsize 0, ptrsize 1\n"
          "  %3 = getelementptr %Outer, byte* %1, ptrsize %2, i32 1, i32 1\n"
          "  store i32 287454020, byte* %3\n"
          "  %4 = getelementptr byte, byte* %1, ptrsize 32\n"
          "  %5 = load i32, byte* %4\n"
          "  ret i32 %5\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 287454020);
    }

    // Verifies that a nested field path from a const byte-slice argument preserves pointer constness and remains readable through typed load.
    TEST(PointerExecutionTest, LoadsNestedStructFieldThroughConstPointer)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {byte, i32}\n"
          "%Outer = type {byte, %Inner}\n"
          "define i32 @main(const byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data const byte* const byte[] %0\n"
          "  %2 = getelementptr %Outer, const byte* %1, ptrsize 0, i32 1, i32 1\n"
          "  %3 = load i32, const byte* %2\n"
          "  ret i32 %3\n"
          "}\n";
      const std::array<std::uint8_t, 12> Bytes = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1};
      RuntimeValueArena Arguments;
      const RuntimeValueRef Slice = Arguments.byteSliceValue(Context.IR.getType(ir::TypeKind::ConstByteSlice), Bytes.data(), Bytes.size());

      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      const ExecutionResult Result = Engine.execute("main", {Slice});

      expectPointerIntegerResult(Result, 16843009);
    }

    // Verifies that a tracked const byte-pointer entry argument retains its backing provenance through ExecutionEngine cloning and GEP.
    TEST(PointerExecutionTest, LoadsThroughTrackedConstEntryPointer)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @main(const byte* %0) {\n"
          "entry:\n"
          "  %1 = getelementptr byte, const byte* %0, ptrsize 2\n"
          "  %2 = load byte, const byte* %1\n"
          "  ret byte %2\n"
          "}\n";
      const std::array<std::uint8_t, 3> Bytes = {2, 4, 8};
      RuntimeValueArena Arguments;
      const RuntimeValueRef Slice = Arguments.byteSliceValue(Context.IR.getType(ir::TypeKind::ConstByteSlice), Bytes.data(), Bytes.size());
      ASSERT_NE(Slice, nullptr);
      const RuntimeValueRef Pointer = Arguments.pointerFromByteSlice(Context.IR.getType(ir::TypeKind::ConstBytePointer), *Slice);
      ASSERT_NE(Pointer, nullptr);
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult Result = Engine.execute("main", {Pointer});

      expectPointerIntegerResult(Result, 8);
    }

    // Verifies that nested GEP layout uses the configured 32-bit target pointer width rather than the host pointer width.
    TEST(PointerExecutionTest, UsesSynthetic32BitLayoutForNestedFieldPath)
    {
      PointerExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {byte, ptrsize}\n"
          "%Outer = type {byte, %Inner}\n"
          "define ptrsize @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 24\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Outer, byte* %1, ptrsize 1, i32 1, i32 1\n"
          "  store ptrsize 16909060, byte* %2\n"
          "  %3 = getelementptr byte, byte* %1, ptrsize 20\n"
          "  %4 = load ptrsize, byte* %3\n"
          "  ret ptrsize %4\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 16909060);
    }

    // Verifies that whole-struct load recursively reconstructs a nested aggregate whose leaves can be extracted normally.
    TEST(PointerExecutionTest, LoadsWholeNestedStructValue)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {byte, i32}\n"
          "%Outer = type {i32, %Inner}\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 12\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  store %Outer {i32 11, %Inner {byte 22, i32 33}}, byte* %1\n"
          "  %2 = load %Outer, byte* %1\n"
          "  %3 = extractvalue %Outer %2, 1\n"
          "  %4 = extractvalue %Inner %3, 1\n"
          "  ret i32 %4\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 33);
    }

    // Verifies that typed struct store preserves pre-existing padding bytes rather than overwriting bytes outside field representations.
    TEST(PointerExecutionTest, PreservesStructPaddingBytesOnStore)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 1\n"
          "  store byte 165, byte* %2\n"
          "  store %Pair {byte 7, i32 9}, byte* %1\n"
          "  %3 = load byte, byte* %2\n"
          "  ret byte %3\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 165);
    }

    // Verifies that typed floating-point memory access preserves a NaN payload bit-for-bit without numeric conversion.
    TEST(PointerExecutionTest, RoundTripsFloatingPointBitPattern)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define f32 @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 5\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 1\n"
          "  store f32 floatbits(f32,0x7FC00042), byte* %2\n"
          "  %3 = load f32, byte* %2\n"
          "  ret f32 %3\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->floatingPointBits(), 0x7FC00042ULL);
    }

    // Verifies that a one-past logical pointer may be formed and compared without being dereferenced.
    TEST(PointerExecutionTest, AllowsOnePastPointerConstruction)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define bool @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Pair, byte* %1, ptrsize 1\n"
          "  %3 = icmp eq byte* %2, byte* %2\n"
          "  ret bool %3\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      expectPointerIntegerResult(Result, 1);
    }

    // Verifies that GEP may form a field address beyond a one-past struct element while the subsequent typed load performs the bounds rejection.
    TEST(PointerExecutionTest, RejectsLoadThroughFieldPathBeyondOnePastElement)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Pair, byte* %1, ptrsize 1, i32 1\n"
          "  %3 = load i32, byte* %2\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessOutOfBounds);
    }

    // Verifies that GEP rejects an element-stride multiplication that overflows the configured 64-bit target address width.
    TEST(PointerExecutionTest, ReportsTargetAddressOverflow)
    {
      PointerExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr f64, byte* %1, ptrsize 2305843009213693952\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAddressOverflow);
    }

    // Verifies that GEP rejects overflow when a struct field offset is added to an already maximally offset tracked pointer.
    TEST(PointerExecutionTest, ReportsTrackedPointerFieldOffsetOverflow)
    {
      PointerExecutionTestContext Context(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian));
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 18446744073709551615\n"
          "  %3 = getelementptr %Pair, byte* %2, ptrsize 0, i32 1\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAddressOverflow);
    }

    // Verifies that execution rejects a GEP root index whose declared type is corrupted after module verification.
    TEST(PointerExecutionTest, RejectsMutatedRootIndexIrTypeAtRuntime)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr %Pair, byte* %1, ptrsize 0, i32 1\n"
          "  ret void\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &GetElementPointer = static_cast<ir::GetElementPointerInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[2]);
      GetElementPointer.Index = std::make_unique<ir::IntegerConstant>(Context.IR.getType(ir::TypeKind::Byte), 0);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidRuntimeMemoryValue);
    }

    // Verifies that execution rejects a ptrsize-typed GEP operand if module corruption makes it resolve to a byte-typed runtime SSA value.
    TEST(PointerExecutionTest, RejectsMutatedRootIndexRuntimeType)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = add byte 0, byte 0\n"
          "  %1 = alloca byte[] ptrsize 8\n"
          "  %2 = slice.data byte* byte[] %1\n"
          "  %3 = getelementptr %Pair, byte* %2, ptrsize 0, i32 1\n"
          "  ret void\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &GetElementPointer = static_cast<ir::GetElementPointerInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[3]);
      GetElementPointer.Index = std::make_unique<ir::ValueOperand>(Context.IR.getType(ir::TypeKind::PointerSize), ir::ValueId{0});

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidRuntimeMemoryValue);
    }

    // Verifies that recursively loading a struct reports an invalid representation when an embedded bool byte is not zero or one.
    TEST(PointerExecutionTest, RejectsInvalidNestedBoolRepresentation)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {bool}\n"
          "%Outer = type {i32, %Inner}\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = alloca byte[] ptrsize 8\n"
          "  %1 = slice.data byte* byte[] %0\n"
          "  %2 = getelementptr byte, byte* %1, ptrsize 4\n"
          "  store byte 2, byte* %2\n"
          "  %3 = load %Outer, byte* %1\n"
          "  ret void\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryInvalidRepresentation);
    }

    // Verifies that typed load rejects an untracked raw pointer produced from null-pointer arithmetic instead of dereferencing host address one.
    TEST(PointerExecutionTest, RejectsTypedLoadThroughUntrackedRawPointer)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = getelementptr byte, byte* null, ptrsize 1\n"
          "  %1 = load byte, byte* %0\n"
          "  ret byte %1\n"
          "}\n";

      const ExecutionResult Result = executePointerText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessRequiresTrackedPointer);
    }

    // Verifies that GEP cannot turn an untracked native entry pointer into a loadable or writable pointer and that a rejected store leaves host memory unchanged.
    TEST(PointerExecutionTest, RejectsTypedAccessThroughUntrackedEntryPointer)
    {
      PointerExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte @read(byte* %0) {\n"
          "entry:\n"
          "  %1 = getelementptr byte, byte* %0, ptrsize 0\n"
          "  %2 = load byte, byte* %1\n"
          "  ret byte %2\n"
          "}\n"
          "define void @write(byte* %0) {\n"
          "entry:\n"
          "  %1 = getelementptr byte, byte* %0, ptrsize 0\n"
          "  store byte 42, byte* %1\n"
          "  ret void\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      std::uint8_t Byte = 19;
      RuntimeValueArena Arguments;
      const RuntimeValueRef Pointer = Arguments.mutablePointerValue(Context.IR.getType(ir::TypeKind::BytePointer), &Byte);
      ASSERT_NE(Pointer, nullptr);

      const ExecutionResult LoadResult = Engine.execute("read", {Pointer});
      const ExecutionResult StoreResult = Engine.execute("write", {Pointer});

      ASSERT_FALSE(LoadResult.succeeded());
      ASSERT_EQ(LoadResult.diagnostics().size(), 1u);
      EXPECT_EQ(LoadResult.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessRequiresTrackedPointer);
      ASSERT_FALSE(StoreResult.succeeded());
      ASSERT_EQ(StoreResult.diagnostics().size(), 1u);
      EXPECT_EQ(StoreResult.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessRequiresTrackedPointer);
      EXPECT_EQ(Byte, 19);
    }
  } // namespace
} // namespace ink::execution
