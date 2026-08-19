#include "ink/execution/execution_engine.h"
#include "ink/ir/model/context.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

namespace ink::execution
{
  namespace
  {
    struct ConstantExecutionTestContext
    {
        ConstantExecutionTestContext()
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ~ConstantExecutionTestContext()
        {
          Compilation.diagnosticEngine().removeConsumer(Diagnostics);
        }

        core::CompilationContext Compilation;
        ir::IRContext IR{Compilation};
        ExecutionContext Execution{Compilation};
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    // Verifies that f16, both signed f32 zeroes, and an f64 NaN payload reach RuntimeValue without numeric conversion.
    TEST(ConstantExecutionTest, ReturnsFloatingPointConstantsWithExactBits)
    {
      ConstantExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define f16 @half_one() {\n"
          "entry:\n"
          "  ret f16 floatbits(f16,0x3C00)\n"
          "}\n"
          "define f32 @positive_zero() {\n"
          "entry:\n"
          "  ret f32 floatbits(f32,0x00000000)\n"
          "}\n"
          "define f32 @negative_zero() {\n"
          "entry:\n"
          "  ret f32 floatbits(f32,0x80000000)\n"
          "}\n"
          "define f64 @nan_payload() {\n"
          "entry:\n"
          "  ret f64 floatbits(f64,0x7FF8000000000042)\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      struct Case
      {
          const char *FunctionName;
          ir::TypeKind Type;
          std::uint64_t Bits;
      };
      const Case Cases[] = {
          {"half_one", ir::TypeKind::F16, 0x3C00U},
          {"positive_zero", ir::TypeKind::F32, 0U},
          {"negative_zero", ir::TypeKind::F32, 0x80000000ULL},
          {"nan_payload", ir::TypeKind::F64, 0x7FF8000000000042ULL},
      };

      for (const Case &CaseValue : Cases)
      {
        const ExecutionResult Result = Engine.execute(CaseValue.FunctionName);

        ASSERT_TRUE(Result.succeeded()) << CaseValue.FunctionName;
        ASSERT_NE(Result.returnValue(), nullptr);
        EXPECT_EQ(Result.returnValue()->kind(), RuntimeValueKind::FloatingPoint);
        EXPECT_EQ(Result.returnValue()->type().kind(), CaseValue.Type);
        ASSERT_TRUE(Result.returnValue()->floatingPointBits().has_value());
        EXPECT_EQ(*Result.returnValue()->floatingPointBits(), CaseValue.Bits);
      }
    }

    // Verifies that an inline string constant reports its embedded-NUL byte length and exposes its final non-ASCII byte to load.
    TEST(ConstantExecutionTest, ReadsInlineStringLengthAndBytes)
    {
      ConstantExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define ptrsize @string_length() {\n"
          "entry:\n"
          "  %0 = slice.length const byte[] c\"\\00\\22\\5C\\FF\"\n"
          "  ret ptrsize %0\n"
          "}\n"
          "define byte @string_last() {\n"
          "entry:\n"
          "  %0 = slice.data const byte* const byte[] c\"\\00\\22\\5C\\FF\"\n"
          "  %1 = getelementptr byte, const byte* %0, ptrsize 3\n"
          "  %2 = load byte, const byte* %1\n"
          "  ret byte %2\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult Length = Engine.execute("string_length");
      const ExecutionResult Last = Engine.execute("string_last");

      ASSERT_TRUE(Length.succeeded());
      ASSERT_NE(Length.returnValue(), nullptr);
      EXPECT_EQ(Length.returnValue()->integer(), 4U);
      ASSERT_TRUE(Last.succeeded());
      ASSERT_NE(Last.returnValue(), nullptr);
      EXPECT_EQ(Last.returnValue()->integer(), 255U);
    }

    // Verifies that a global byte address carries bounded backing information through GEP so typed load can read the selected constant byte.
    TEST(ConstantExecutionTest, LoadsThroughBoundedGlobalAddress)
    {
      ConstantExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "@data = private constant [4 x byte] c\"\\0A\\14\\1E\\28\"\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = getelementptr byte, const byte* @data[0], ptrsize 2\n"
          "  %1 = load byte, const byte* %0\n"
          "  ret byte %1\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->integer(), 30U);
    }

    // Verifies that all addresses of one global share provenance, equivalent offsets compare equal, and a one-past address cannot be loaded.
    TEST(ConstantExecutionTest, PreservesGlobalPointerIdentityAndBounds)
    {
      ConstantExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "@data = private constant [2 x byte] c\"ab\"\n"
          "define bool @same_address() {\n"
          "entry:\n"
          "  %0 = icmp eq const byte* @data[0], const byte* @data[0]\n"
          "  ret bool %0\n"
          "}\n"
          "define bool @equivalent_offset() {\n"
          "entry:\n"
          "  %0 = getelementptr byte, const byte* @data[0], ptrsize 1\n"
          "  %1 = icmp eq const byte* %0, const byte* @data[1]\n"
          "  ret bool %1\n"
          "}\n"
          "define void @out_of_bounds() {\n"
          "entry:\n"
          "  %0 = getelementptr byte, const byte* @data[0], ptrsize 2\n"
          "  %1 = load byte, const byte* %0\n"
          "  ret void\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult SameAddress = Engine.execute("same_address");
      const ExecutionResult EquivalentOffset = Engine.execute("equivalent_offset");
      const ExecutionResult OutOfBounds = Engine.execute("out_of_bounds");

      ASSERT_TRUE(SameAddress.succeeded());
      ASSERT_NE(SameAddress.returnValue(), nullptr);
      EXPECT_EQ(SameAddress.returnValue()->integer(), 1U);
      ASSERT_TRUE(EquivalentOffset.succeeded());
      ASSERT_NE(EquivalentOffset.returnValue(), nullptr);
      EXPECT_EQ(EquivalentOffset.returnValue()->integer(), 1U);
      ASSERT_FALSE(OutOfBounds.succeeded());
      ASSERT_EQ(Context.Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Context.Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::MemoryAccessOutOfBounds);
    }

    // Verifies that both raw byte-pointer null constant types materialize as pointer RuntimeValues with null addresses.
    TEST(ConstantExecutionTest, MaterializesBothNullPointerTypes)
    {
      ConstantExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define byte* @mutable_null() {\n"
          "entry:\n"
          "  ret byte* null\n"
          "}\n"
          "define const byte* @const_null() {\n"
          "entry:\n"
          "  ret const byte* null\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult Mutable = Engine.execute("mutable_null");
      const ExecutionResult Const = Engine.execute("const_null");

      ASSERT_TRUE(Mutable.succeeded());
      ASSERT_NE(Mutable.returnValue(), nullptr);
      EXPECT_EQ(Mutable.returnValue()->kind(), RuntimeValueKind::Pointer);
      EXPECT_EQ(Mutable.returnValue()->type().kind(), ir::TypeKind::BytePointer);
      EXPECT_EQ(Mutable.returnValue()->pointer(), nullptr);
      EXPECT_EQ(Mutable.returnValue()->mutablePointer(), nullptr);
      ASSERT_TRUE(Const.succeeded());
      ASSERT_NE(Const.returnValue(), nullptr);
      EXPECT_EQ(Const.returnValue()->kind(), RuntimeValueKind::Pointer);
      EXPECT_EQ(Const.returnValue()->type().kind(), ir::TypeKind::ConstBytePointer);
      EXPECT_EQ(Const.returnValue()->pointer(), nullptr);
      EXPECT_EQ(Const.returnValue()->mutablePointer(), nullptr);
    }

    // Verifies that nested aggregate constants recursively materialize integer, NaN payload, and null-pointer leaves.
    TEST(ConstantExecutionTest, MaterializesNestedAggregateConstants)
    {
      ConstantExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Inner = type {f32, const byte*}\n"
          "%Outer = type {i32, %Inner}\n"
          "define %Outer @main() {\n"
          "entry:\n"
          "  ret %Outer {i32 7, %Inner {f32 floatbits(f32,0x7FC00042), const byte* null}}\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->kind(), RuntimeValueKind::Aggregate);
      ASSERT_EQ(Result.returnValue()->fieldCount(), 2U);
      ASSERT_NE(Result.returnValue()->field(0), nullptr);
      EXPECT_EQ(Result.returnValue()->field(0)->integer(), 7U);
      const RuntimeValue *Inner = Result.returnValue()->field(1);
      ASSERT_NE(Inner, nullptr);
      EXPECT_EQ(Inner->kind(), RuntimeValueKind::Aggregate);
      ASSERT_EQ(Inner->fieldCount(), 2U);
      ASSERT_NE(Inner->field(0), nullptr);
      ASSERT_TRUE(Inner->field(0)->floatingPointBits().has_value());
      EXPECT_EQ(*Inner->field(0)->floatingPointBits(), 0x7FC00042ULL);
      ASSERT_NE(Inner->field(1), nullptr);
      EXPECT_EQ(Inner->field(1)->kind(), RuntimeValueKind::Pointer);
      EXPECT_EQ(Inner->field(1)->pointer(), nullptr);
    }
  } // namespace
} // namespace ink::execution
