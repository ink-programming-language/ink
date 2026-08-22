#include "ink/ir/analysis/verifier.h"
#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"

#include "ir_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    using ConstantIrTestContext = test::IRTestContext;
    using ink::test::hasDiagnostic;

    struct CapturedVerificationResult
    {
        VerificationResult Status;
        std::vector<core::Diagnostic> Diagnostics;

        bool succeeded() const noexcept
        {
          return Status.succeeded();
        }

        const std::vector<core::Diagnostic> &diagnostics() const noexcept
        {
          return Diagnostics;
        }
    };

    void expectCanonicalRoundTrip(IRContext &Context, const std::string &Text)
    {
      ink::test::DiagnosticCapture Diagnostics(Context.compilationContext());
      DeserializeResult Parsed = deserialize(Context, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      EXPECT_TRUE(Diagnostics.diagnostics().empty());

      SerializeResult Serialized = serialize(Context, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
      EXPECT_EQ(*Serialized.text(), Text);
    }

    CapturedVerificationResult verifyReturnedConstant(ConstantIrTestContext &Context, const Type &ResultType, const Constant &ConstantValue, std::vector<const StructType *> StructTypes = {})
    {
      Module ModuleValue(Context.IR);
      ModuleValue.StructTypes = std::move(StructTypes);
      Function Main(ResultType);
      Main.Name = "main";
      BasicBlock Entry;
      Entry.Name = "entry";
      auto Return = std::make_unique<ReturnInstruction>();
      Return->ReturnValue = ConstantValue;
      Entry.Instructions.push_back(std::move(Return));
      Main.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Main));
      Context.Diagnostics.clear();
      VerificationResult Result = verify(Context.IR, ModuleValue);
      return {Result, Context.Diagnostics.diagnostics()};
    }

    const ReturnInstruction &returnInstruction(const Module &ModuleValue, std::size_t FunctionIndex)
    {
      return static_cast<const ReturnInstruction &>(*ModuleValue.Functions[FunctionIndex].Blocks[0].Instructions.back());
    }

    // Verifies that every constant kind shares the Constant base and retains its type-specific payload through Value.
    TEST(ConstantIrValueTest, ExposesTypedPayloadsForEveryConstantKind)
    {
      ConstantIrTestContext Context;
      static_assert(std::is_base_of_v<Value, Constant>);
      static_assert(std::is_base_of_v<Constant, IntegerConstant>);
      static_assert(std::is_base_of_v<Constant, FloatConstant>);
      static_assert(std::is_base_of_v<Constant, StringConstant>);
      static_assert(std::is_base_of_v<Constant, NullConstant>);
      static_assert(std::is_base_of_v<Constant, ZeroInitializer>);
      static_assert(std::is_base_of_v<Constant, AggregateConstant>);
      const Type &F16Type = Context.IR.getType(TypeKind::F16);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const Type &F64Type = Context.IR.getType(TypeKind::F64);
      const Type &ConstByteSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &F32Type});
      ConstantPool &Pool = Context.IR.constantPool();
      const FloatConstant &Half = Pool.getFloatConstant(F16Type, FloatFormat::F16, 0x3C00U);
      const FloatConstant &Single = Pool.getFloatConstant(F32Type, FloatFormat::F32, 0x80000000ULL);
      const FloatConstant &Double = Pool.getFloatConstant(F64Type, FloatFormat::F64, 0x7FF8000000000042ULL);
      const StringConstant &String = Pool.getStringConstant(ConstByteSliceType, std::string("\0\"\\\xFF", 4));
      const NullConstant &Null = Pool.getNullConstant(ConstBytePointerType);
      const IntegerConstant &Seven = Pool.getIntegerConstant(I32Type, 7);
      const FloatConstant &One = Pool.getFloatConstant(F32Type, FloatFormat::F32, 0x3F800000ULL);
      const AggregateConstant &Aggregate = Pool.getAggregateConstant(PairType, {Seven, One});

      EXPECT_EQ(Half.kind(), ValueKind::FloatConstant);
      EXPECT_EQ(&Half.type(), &F16Type);
      EXPECT_EQ(Half.format(), FloatFormat::F16);
      EXPECT_EQ(Half.bitPattern(), 0x3C00U);
      EXPECT_EQ(Single.format(), FloatFormat::F32);
      EXPECT_EQ(Single.bitPattern(), 0x80000000ULL);
      EXPECT_EQ(Double.format(), FloatFormat::F64);
      EXPECT_EQ(Double.bitPattern(), 0x7FF8000000000042ULL);
      EXPECT_EQ(String.kind(), ValueKind::StringConstant);
      EXPECT_EQ(&String.type(), &ConstByteSliceType);
      EXPECT_EQ(String.data(), std::string("\0\"\\\xFF", 4));
      EXPECT_EQ(Null.kind(), ValueKind::NullConstant);
      EXPECT_EQ(&Null.type(), &ConstBytePointerType);
      EXPECT_EQ(Aggregate.kind(), ValueKind::AggregateConstant);
      EXPECT_EQ(&Aggregate.type(), &PairType);
      ASSERT_EQ(Aggregate.elements().size(), 2U);
      EXPECT_EQ(Aggregate.elements()[0].get().kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(Aggregate.elements()[1].get().kind(), ValueKind::FloatConstant);
      EXPECT_EQ(static_cast<const FloatConstant &>(Aggregate.elements()[1].get()).bitPattern(), 0x3F800000ULL);
    }

    // Verifies that f16, both signed f32 zeroes, and an f64 NaN payload use fixed-width canonical bit-pattern text.
    TEST(ConstantIrSerializationTest, RoundTripsFloatingSpecialValuesWithoutChangingBits)
    {
      ConstantIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "define f16 @half_one() {\n"
          "entry:\n"
          "  ret f16 floatbits(f16,0x3C00)\n"
          "}\n"
          "\n"
          "define f32 @positive_zero() {\n"
          "entry:\n"
          "  ret f32 floatbits(f32,0x00000000)\n"
          "}\n"
          "\n"
          "define f32 @negative_zero() {\n"
          "entry:\n"
          "  ret f32 floatbits(f32,0x80000000)\n"
          "}\n"
          "\n"
          "define f64 @nan_payload() {\n"
          "entry:\n"
          "  ret f64 floatbits(f64,0x7FF8000000000042)\n"
          "}\n";

      expectCanonicalRoundTrip(Context.IR, Text);
      DeserializeResult Parsed = deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->Functions.size(), 4U);
      const FloatConstant &Half = static_cast<const FloatConstant &>(*returnInstruction(*Parsed.module(), 0).ReturnValue);
      const FloatConstant &PositiveZero = static_cast<const FloatConstant &>(*returnInstruction(*Parsed.module(), 1).ReturnValue);
      const FloatConstant &NegativeZero = static_cast<const FloatConstant &>(*returnInstruction(*Parsed.module(), 2).ReturnValue);
      const FloatConstant &NanPayload = static_cast<const FloatConstant &>(*returnInstruction(*Parsed.module(), 3).ReturnValue);

      EXPECT_EQ(Half.format(), FloatFormat::F16);
      EXPECT_EQ(Half.bitPattern(), 0x3C00U);
      EXPECT_EQ(PositiveZero.bitPattern(), 0U);
      EXPECT_EQ(NegativeZero.bitPattern(), 0x80000000ULL);
      EXPECT_EQ(NanPayload.format(), FloatFormat::F64);
      EXPECT_EQ(NanPayload.bitPattern(), 0x7FF8000000000042ULL);
    }

    // Verifies that inline string constants preserve NUL, quote, backslash, and non-ASCII bytes through canonical escaping.
    TEST(ConstantIrSerializationTest, RoundTripsEveryEscapedInlineStringByte)
    {
      ConstantIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "define ptrsize @main() {\n"
          "entry:\n"
          "  %0 = slice.length const byte slice c\"\\00\\22\\5C\\FF\"\n"
          "  ret ptrsize %0\n"
          "}\n";

      expectCanonicalRoundTrip(Context.IR, Text);
      DeserializeResult Parsed = deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      const SliceLengthInstruction &Length = static_cast<const SliceLengthInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      ASSERT_TRUE(Length.Slice);
      ASSERT_EQ(Length.Slice->kind(), ValueKind::StringConstant);
      EXPECT_EQ(static_cast<const StringConstant &>(*Length.Slice).data(), std::string("\0\"\\\xFF", 4));
    }

    // Verifies that mutable and const byte-pointer null constants have distinct typed canonical forms.
    TEST(ConstantIrSerializationTest, RoundTripsBothNullPointerTypes)
    {
      ConstantIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "define byte* @mutable_null() {\n"
          "entry:\n"
          "  ret byte* null\n"
          "}\n"
          "\n"
          "define const byte* @const_null() {\n"
          "entry:\n"
          "  ret const byte* null\n"
          "}\n";

      expectCanonicalRoundTrip(Context.IR, Text);
      DeserializeResult Parsed = deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(returnInstruction(*Parsed.module(), 0).ReturnValue->kind(), ValueKind::NullConstant);
      ASSERT_EQ(returnInstruction(*Parsed.module(), 1).ReturnValue->kind(), ValueKind::NullConstant);
      EXPECT_EQ(returnInstruction(*Parsed.module(), 0).ReturnValue->type().kind(), TypeKind::BytePointer);
      EXPECT_EQ(returnInstruction(*Parsed.module(), 1).ReturnValue->type().kind(), TypeKind::ConstBytePointer);
    }

    // Verifies that nested aggregate constants recursively round-trip floating-point and null leaves in field order.
    TEST(ConstantIrSerializationTest, RoundTripsNestedAggregateConstants)
    {
      ConstantIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "%Inner = type {f32, const byte*}\n"
          "\n"
          "%Outer = type {i32, %Inner}\n"
          "\n"
          "define %Outer @main() {\n"
          "entry:\n"
          "  ret %Outer {i32 7, %Inner {f32 floatbits(f32,0x7FC00042), const byte* null}}\n"
          "}\n";

      expectCanonicalRoundTrip(Context.IR, Text);
      DeserializeResult Parsed = deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      const AggregateConstant &Outer = static_cast<const AggregateConstant &>(*returnInstruction(*Parsed.module(), 0).ReturnValue);
      ASSERT_EQ(Outer.elements().size(), 2U);
      ASSERT_EQ(Outer.elements()[1].get().kind(), ValueKind::AggregateConstant);
      const AggregateConstant &Inner = static_cast<const AggregateConstant &>(Outer.elements()[1].get());
      ASSERT_EQ(Inner.elements().size(), 2U);
      EXPECT_EQ(static_cast<const FloatConstant &>(Inner.elements()[0].get()).bitPattern(), 0x7FC00042ULL);
      EXPECT_EQ(Inner.elements()[1].get().kind(), ValueKind::NullConstant);
    }

    // Verifies that every new constant kind is rejected when its declared IR type belongs to a different value category.
    TEST(ConstantIrVerifierTest, RejectsConstantsWithInvalidTypes)
    {
      ConstantIrTestContext Context;
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      ConstantPool &Pool = Context.IR.constantPool();
      const IntegerConstant &One = Pool.getIntegerConstant(I32Type, 1);

      const CapturedVerificationResult FloatResult = verifyReturnedConstant(Context, I32Type, Pool.getFloatConstant(I32Type, FloatFormat::F32, 0x3F800000ULL));
      const CapturedVerificationResult StringResult = verifyReturnedConstant(Context, I32Type, Pool.getStringConstant(I32Type, "text"));
      const CapturedVerificationResult NullResult = verifyReturnedConstant(Context, I32Type, Pool.getNullConstant(I32Type));
      const CapturedVerificationResult AggregateResult = verifyReturnedConstant(Context, I32Type, Pool.getAggregateConstant(I32Type, {One}));

      EXPECT_FALSE(FloatResult.succeeded());
      EXPECT_FALSE(StringResult.succeeded());
      EXPECT_FALSE(NullResult.succeeded());
      EXPECT_FALSE(AggregateResult.succeeded());
      EXPECT_TRUE(hasDiagnostic(FloatResult.diagnostics(), core::DiagnosticKind::IrFloatConstantInvalidType));
      EXPECT_TRUE(hasDiagnostic(StringResult.diagnostics(), core::DiagnosticKind::IrStringConstantInvalidType));
      EXPECT_TRUE(hasDiagnostic(NullResult.diagnostics(), core::DiagnosticKind::IrNullConstantInvalidType));
      EXPECT_TRUE(hasDiagnostic(AggregateResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantInvalidType));
    }

    // Verifies that a float constant cannot disagree with its type format or carry bits above that format's fixed width.
    TEST(ConstantIrVerifierTest, RejectsMismatchedFloatFormatsAndOversizedBitPatterns)
    {
      ConstantIrTestContext Context;
      const Type &F16Type = Context.IR.getType(TypeKind::F16);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);

      const CapturedVerificationResult FormatMismatch = verifyReturnedConstant(Context, F32Type, Context.IR.constantPool().getFloatConstant(F32Type, FloatFormat::F64, 0x3FF0000000000000ULL));
      const CapturedVerificationResult OversizedBits = verifyReturnedConstant(Context, F16Type, Context.IR.constantPool().getFloatConstant(F16Type, FloatFormat::F16, 0x10000ULL));

      EXPECT_FALSE(FormatMismatch.succeeded());
      EXPECT_FALSE(OversizedBits.succeeded());
      EXPECT_TRUE(hasDiagnostic(FormatMismatch.diagnostics(), core::DiagnosticKind::IrFloatConstantFormatMismatch));
      EXPECT_TRUE(hasDiagnostic(OversizedBits.diagnostics(), core::DiagnosticKind::IrFloatConstantBitPatternOutOfRange));
    }

    // Verifies that an instruction cannot borrow a constant from a pool other than its module IR context.
    TEST(ConstantIrVerifierTest, RejectsConstantFromAnotherPool)
    {
      ConstantIrTestContext Context;
      ConstantPool ForeignPool;
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const IntegerConstant &ForeignConstant = ForeignPool.getIntegerConstant(I32Type, 5);

      const CapturedVerificationResult Result = verifyReturnedConstant(Context, I32Type, ForeignConstant);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrConstantPoolMismatch));
    }

    // Verifies that aggregate constants reject missing, mistyped, and recursively malformed fields while null and nonconstant fields are unrepresentable.
    TEST(ConstantIrVerifierTest, RejectsEveryMalformedAggregateShape)
    {
      ConstantIrTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &F32Type});
      ConstantPool &Pool = Context.IR.constantPool();
      const IntegerConstant &I32One = Pool.getIntegerConstant(I32Type, 1);
      const IntegerConstant &ByteOne = Pool.getIntegerConstant(ByteType, 1);
      const FloatConstant &ValidFloat = Pool.getFloatConstant(F32Type, FloatFormat::F32, 0);
      const FloatConstant &MalformedFloat = Pool.getFloatConstant(F32Type, FloatFormat::F64, 0);
      const auto VerifyElements = [&](const std::vector<std::reference_wrapper<const Constant>> &Elements)
      {
        return verifyReturnedConstant(Context, PairType, Pool.getAggregateConstant(PairType, Elements), {&PairType});
      };

      const CapturedVerificationResult MissingResult = VerifyElements({I32One});
      const CapturedVerificationResult MistypedResult = VerifyElements({ByteOne, ValidFloat});
      const CapturedVerificationResult MalformedLeafResult = VerifyElements({I32One, MalformedFloat});

      EXPECT_FALSE(MissingResult.succeeded());
      EXPECT_FALSE(MistypedResult.succeeded());
      EXPECT_FALSE(MalformedLeafResult.succeeded());
      EXPECT_TRUE(hasDiagnostic(MissingResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantFieldCountMismatch));
      EXPECT_TRUE(hasDiagnostic(MistypedResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantElementTypeMismatch));
      EXPECT_TRUE(hasDiagnostic(MalformedLeafResult.diagnostics(), core::DiagnosticKind::IrFloatConstantFormatMismatch));
    }

    // Verifies that each floatbits spelling contains exactly the hexadecimal digit count required by its declared format.
    TEST(ConstantIrDeserializationTest, RejectsNonCanonicalFloatingBitWidths)
    {
      const char *InvalidValues[] = {
          "f16 floatbits(f16,0x000)",
          "f16 floatbits(f16,0x00000)",
          "f32 floatbits(f32,0x0000000)",
          "f32 floatbits(f32,0x000000000)",
          "f64 floatbits(f64,0x000000000000000)",
          "f64 floatbits(f64,0x00000000000000000)",
      };
      for (const char *InvalidValue : InvalidValues)
      {
        ConstantIrTestContext Context;
        const std::string TypeName(InvalidValue, 3);
        const std::string Text = "inkir 1\ndefine " + TypeName + " @main() {\nentry:\n  ret " + std::string(InvalidValue) + "\n}\n";

        const DeserializeResult Result = deserialize(Context.IR, Text);

        EXPECT_FALSE(Result.succeeded()) << InvalidValue;
        EXPECT_TRUE(hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrFloatBitPatternWidthMismatch)) << InvalidValue;
      }
    }
  } // namespace
} // namespace ink::ir
