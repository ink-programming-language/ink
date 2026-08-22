#include "ink/ir/serialization.h"

#include "ir_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace ink::ir
{
  namespace
  {
    void expectLegacySliceSpellingRejected(std::string_view TypeText)
    {
      test::IRTestContext Context;
      const std::string Text = "inkir 1\ndefine void @consume(" + std::string(TypeText) + " %0) {\nentry:\n  ret void\n}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(ink::test::hasDiagnostic(Context.Diagnostics.diagnostics(), core::DiagnosticKind::IrExpected));
    }

    // Verifies that mutable and const byte slices use the slice keyword in every canonical type position.
    TEST(IrTypeSerializationTest, RoundTripsSliceKeywordTypes)
    {
      test::IRTestContext Context;
      const std::string Input =
          "inkir 1\n"
          "define void @consume(byte slice %0, const byte slice %1) {\n"
          "entry:\n"
          "  %2 = slice.length byte slice %0\n"
          "  %3 = slice.length const byte slice %1\n"
          "  ret void\n"
          "}\n";
      const std::string Canonical =
          "inkir 1\n"
          "\n"
          "define void @consume(byte slice %0, const byte slice %1) {\n"
          "entry:\n"
          "  %2 = slice.length byte slice %0\n"
          "  %3 = slice.length const byte slice %1\n"
          "  ret void\n"
          "}\n";

      DeserializeResult Parsed = deserialize(Context.IR, Input);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->Functions.size(), 1U);
      ASSERT_EQ(Parsed.module()->Functions[0].Parameters.size(), 2U);
      EXPECT_EQ(Parsed.module()->Functions[0].Parameters[0].type()->kind(), TypeKind::ByteSlice);
      EXPECT_EQ(Parsed.module()->Functions[0].Parameters[1].type()->kind(), TypeKind::ConstByteSlice);
      EXPECT_STREQ(typeKindName(TypeKind::ByteSlice), "byte slice");
      EXPECT_STREQ(typeKindName(TypeKind::ConstByteSlice), "const byte slice");

      const SerializeResult Serialized = serialize(Context.IR, *Parsed.module());

      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), Canonical);
    }

    // Verifies that brackets after a byte field now form an empty attribute list instead of a slice type suffix.
    TEST(IrTypeSerializationTest, ParsesEmptyByteFieldAttributeListWithoutTypeAmbiguity)
    {
      test::IRTestContext Context;
      const std::string Input = "inkir 1\n%Record = type {Value: byte[]}\n";
      const std::string Canonical = "inkir 1\n\n%Record = type {Value: byte}\n";

      DeserializeResult Parsed = deserialize(Context.IR, Input);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->StructTypes.size(), 1U);
      const StructType &Record = *Parsed.module()->StructTypes[0];
      ASSERT_EQ(Record.fieldCount(), 1U);
      EXPECT_EQ(Record.field(0).type()->kind(), TypeKind::Byte);
      EXPECT_TRUE(Record.field(0).attributes().empty());

      const SerializeResult Serialized = serialize(Context.IR, *Parsed.module());

      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), Canonical);
    }

    // Verifies that the removed bracket spellings are rejected where an attribute list cannot follow a type.
    TEST(IrTypeDeserializationTest, RejectsLegacyBracketSliceSpellings)
    {
      expectLegacySliceSpellingRejected("byte[]");
      expectLegacySliceSpellingRejected("const byte[]");
    }
  } // namespace
} // namespace ink::ir
