#include "ink/ir/analysis/verifier.h"
#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct ConstantIrTestContext
    {
        core::CompilationContext Compilation;
        IRContext IR{Compilation};
    };

    void expectCanonicalRoundTrip(IRContext &Context, const std::string &Text)
    {
      DeserializeResult Parsed = deserialize(Context, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      EXPECT_TRUE(Parsed.diagnostics().empty());

      SerializeResult Serialized = serialize(Context, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_TRUE(Serialized.diagnostics().empty());
      EXPECT_EQ(*Serialized.text(), Text);
    }

    VerificationResult verifyReturnedConstant(ConstantIrTestContext &Context, const Type &ResultType, std::unique_ptr<Value> Constant, std::vector<const StructType *> StructTypes = {})
    {
      Module ModuleValue(Context.IR);
      ModuleValue.StructTypes = std::move(StructTypes);
      Function Main(ResultType);
      Main.Name = "main";
      BasicBlock Entry;
      Entry.Name = "entry";
      auto Return = std::make_unique<ReturnInstruction>();
      Return->ReturnValue = std::move(Constant);
      Entry.Instructions.push_back(std::move(Return));
      Main.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Main));
      return verify(Context.IR, ModuleValue);
    }

    bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      for (const core::Diagnostic &DiagnosticEntry : Diagnostics)
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    const ReturnInstruction &returnInstruction(const Module &ModuleValue, std::size_t FunctionIndex)
    {
      return static_cast<const ReturnInstruction &>(*ModuleValue.Functions[FunctionIndex].Blocks[0].Instructions.back());
    }

    // Verifies that all four constant classes retain their type-specific payloads through the abstract Value interface.
    TEST(ConstantIrValueTest, ExposesTypedPayloadsForEveryConstantKind)
    {
      ConstantIrTestContext Context;
      static_assert(std::is_base_of_v<Value, FloatConstant>);
      static_assert(std::is_base_of_v<Value, StringConstant>);
      static_assert(std::is_base_of_v<Value, NullConstant>);
      static_assert(std::is_base_of_v<Value, AggregateConstant>);
      const Type &F16Type = Context.IR.getType(TypeKind::F16);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const Type &F64Type = Context.IR.getType(TypeKind::F64);
      const Type &ConstByteSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &F32Type});
      FloatConstant Half(F16Type, FloatFormat::F16, 0x3C00U);
      FloatConstant Single(F32Type, FloatFormat::F32, 0x80000000ULL);
      FloatConstant Double(F64Type, FloatFormat::F64, 0x7FF8000000000042ULL);
      StringConstant String(ConstByteSliceType, std::string("\0\"\\\xFF", 4));
      NullConstant Null(ConstBytePointerType);
      std::vector<std::unique_ptr<Value>> Elements;
      Elements.push_back(std::make_unique<IntegerConstant>(I32Type, 7));
      Elements.push_back(std::make_unique<FloatConstant>(F32Type, FloatFormat::F32, 0x3F800000ULL));
      AggregateConstant Aggregate(PairType, std::move(Elements));

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
      EXPECT_EQ(Aggregate.elements()[0]->kind(), ValueKind::IntegerConstant);
      EXPECT_EQ(Aggregate.elements()[1]->kind(), ValueKind::FloatConstant);
      EXPECT_EQ(static_cast<const FloatConstant &>(*Aggregate.elements()[1]).bitPattern(), 0x3F800000ULL);
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
          "  %0 = slice.length const byte[] c\"\\00\\22\\5C\\FF\"\n"
          "  ret ptrsize %0\n"
          "}\n";

      expectCanonicalRoundTrip(Context.IR, Text);
      DeserializeResult Parsed = deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      const SliceLengthInstruction &Length = static_cast<const SliceLengthInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      ASSERT_NE(Length.Slice, nullptr);
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
      ASSERT_EQ(Outer.elements()[1]->kind(), ValueKind::AggregateConstant);
      const AggregateConstant &Inner = static_cast<const AggregateConstant &>(*Outer.elements()[1]);
      ASSERT_EQ(Inner.elements().size(), 2U);
      EXPECT_EQ(static_cast<const FloatConstant &>(*Inner.elements()[0]).bitPattern(), 0x7FC00042ULL);
      EXPECT_EQ(Inner.elements()[1]->kind(), ValueKind::NullConstant);
    }

    // Verifies that every new constant kind is rejected when its declared IR type belongs to a different value category.
    TEST(ConstantIrVerifierTest, RejectsConstantsWithInvalidTypes)
    {
      ConstantIrTestContext Context;
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      std::vector<std::unique_ptr<Value>> Elements;
      Elements.push_back(std::make_unique<IntegerConstant>(I32Type, 1));

      const VerificationResult FloatResult = verifyReturnedConstant(Context, I32Type, std::make_unique<FloatConstant>(I32Type, FloatFormat::F32, 0x3F800000ULL));
      const VerificationResult StringResult = verifyReturnedConstant(Context, I32Type, std::make_unique<StringConstant>(I32Type, "text"));
      const VerificationResult NullResult = verifyReturnedConstant(Context, I32Type, std::make_unique<NullConstant>(I32Type));
      const VerificationResult AggregateResult = verifyReturnedConstant(Context, I32Type, std::make_unique<AggregateConstant>(I32Type, std::move(Elements)));

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

      const VerificationResult FormatMismatch = verifyReturnedConstant(Context, F32Type, std::make_unique<FloatConstant>(F32Type, FloatFormat::F64, 0x3FF0000000000000ULL));
      const VerificationResult OversizedBits = verifyReturnedConstant(Context, F16Type, std::make_unique<FloatConstant>(F16Type, FloatFormat::F16, 0x10000ULL));

      EXPECT_FALSE(FormatMismatch.succeeded());
      EXPECT_FALSE(OversizedBits.succeeded());
      EXPECT_TRUE(hasDiagnostic(FormatMismatch.diagnostics(), core::DiagnosticKind::IrFloatConstantFormatMismatch));
      EXPECT_TRUE(hasDiagnostic(OversizedBits.diagnostics(), core::DiagnosticKind::IrFloatConstantBitPatternOutOfRange));
    }

    // Verifies that aggregate constants reject missing, null, mistyped, and recursively malformed fields.
    TEST(ConstantIrVerifierTest, RejectsEveryMalformedAggregateShape)
    {
      ConstantIrTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &F32Type});
      const auto VerifyElements = [&](std::vector<std::unique_ptr<Value>> Elements)
      {
        return verifyReturnedConstant(Context, PairType, std::make_unique<AggregateConstant>(PairType, std::move(Elements)), {&PairType});
      };

      std::vector<std::unique_ptr<Value>> Missing;
      Missing.push_back(std::make_unique<IntegerConstant>(I32Type, 1));
      std::vector<std::unique_ptr<Value>> Null;
      Null.push_back(std::make_unique<IntegerConstant>(I32Type, 1));
      Null.push_back(nullptr);
      std::vector<std::unique_ptr<Value>> Mistyped;
      Mistyped.push_back(std::make_unique<IntegerConstant>(ByteType, 1));
      Mistyped.push_back(std::make_unique<FloatConstant>(F32Type, FloatFormat::F32, 0));
      std::vector<std::unique_ptr<Value>> MalformedLeaf;
      MalformedLeaf.push_back(std::make_unique<IntegerConstant>(I32Type, 1));
      MalformedLeaf.push_back(std::make_unique<FloatConstant>(F32Type, FloatFormat::F64, 0));

      std::vector<std::unique_ptr<Value>> NonConstant;
      NonConstant.push_back(std::make_unique<ValueOperand>(I32Type, ValueId{0}));
      NonConstant.push_back(std::make_unique<FloatConstant>(F32Type, FloatFormat::F32, 0));

      const VerificationResult MissingResult = VerifyElements(std::move(Missing));
      const VerificationResult NullResult = VerifyElements(std::move(Null));
      const VerificationResult MistypedResult = VerifyElements(std::move(Mistyped));
      const VerificationResult MalformedLeafResult = VerifyElements(std::move(MalformedLeaf));
      const VerificationResult NonConstantResult = VerifyElements(std::move(NonConstant));

      EXPECT_FALSE(MissingResult.succeeded());
      EXPECT_FALSE(NullResult.succeeded());
      EXPECT_FALSE(MistypedResult.succeeded());
      EXPECT_FALSE(MalformedLeafResult.succeeded());
      EXPECT_FALSE(NonConstantResult.succeeded());
      EXPECT_TRUE(hasDiagnostic(MissingResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantFieldCountMismatch));
      EXPECT_TRUE(hasDiagnostic(NullResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantNullElement));
      EXPECT_TRUE(hasDiagnostic(MistypedResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantElementTypeMismatch));
      EXPECT_TRUE(hasDiagnostic(MalformedLeafResult.diagnostics(), core::DiagnosticKind::IrFloatConstantFormatMismatch));
      EXPECT_TRUE(hasDiagnostic(NonConstantResult.diagnostics(), core::DiagnosticKind::IrAggregateConstantNonConstantElement));
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
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrFloatBitPatternWidthMismatch)) << InvalidValue;
      }
    }
  } // namespace
} // namespace ink::ir
