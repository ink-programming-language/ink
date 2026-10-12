#include "ink/target/target_context.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/string_type.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Factory-created strings occupy a target-sized pointer and length regardless of byte order, with the matching length limit.
  TEST(IRStringTest, ViewLayoutAndLengthLimitFollowTarget)
  {
    for (const target::PointerWidth Width : {target::PointerWidth::Bits32, target::PointerWidth::Bits64})
    {
      for (const target::ByteOrder Order : {target::ByteOrder::LittleEndian, target::ByteOrder::BigEndian})
      {
        const target::TargetContext Target(Width, Order);
        TypeContext Types(Target);
        const auto String = Types.getStringType();
        ASSERT_NE(String, nullptr);
        const Type *TypeValue = String;
        EXPECT_EQ(TypeValue->kind(), ValueKind::String);
        EXPECT_TRUE(TypeValue->hasLayout());
        EXPECT_EQ(TypeValue->size(), Width == target::PointerWidth::Bits32 ? 8U : 16U);
        EXPECT_EQ(TypeValue->alignment(), Width == target::PointerWidth::Bits32 ? 4U : 8U);
        EXPECT_EQ(String->maximumByteLength(), Width == target::PointerWidth::Bits32 ? std::numeric_limits<std::uint32_t>::max() : std::numeric_limits<std::uint64_t>::max());
      }
    }
  }

  // UTF-8 content includes empty text, multibyte characters and scalar boundaries while length counts bytes rather than storage layout.
  TEST(IRStringTest, ContentLengthCountsUtf8Bytes)
  {
    TypeContext Types(Target64);
    auto &Constants = Types.constantPool();
    const auto String = Types.getStringType();
    constexpr std::string_view Contents[] = {
        "",
        "hello",
        "\xE4\xBD\xA0\xE5\xA5\xBD",
        "\xF0\x9F\x98\x80",
        "\x7F\xC2\x80\xDF\xBF\xE0\xA0\x80\xED\x9F\xBF\xEE\x80\x80\xEF\xBF\xBF\xF0\x90\x80\x80\xF4\x8F\xBF\xBF",
    };
    for (const std::string_view Expected : Contents)
    {
      const auto Text = Constants.getStringConstant(std::string(Expected));
      ASSERT_NE(Text, nullptr);
      EXPECT_EQ(Text->type(), String);
      EXPECT_EQ(Text->value(), Expected);
      EXPECT_EQ(Text->byteLength(), Expected.size());
      EXPECT_EQ(Text->type()->size(), 16U);
    }
    const auto Chinese = Constants.getStringConstant("\xE4\xBD\xA0\xE5\xA5\xBD");
    ASSERT_NE(Chinese, nullptr);
    EXPECT_EQ(Chinese->byteLength(), 6U);
  }

  // String constants preserve embedded NUL, BOM, literal escape text and distinct normalization forms without rewriting content.
  TEST(IRStringTest, PreservesExactValidBytes)
  {
    TypeContext Types(Target64);
    auto &Constants = Types.constantPool();
    const std::string_view EmbeddedNul("a\0b", 3);
    const auto BinarySafe = Constants.getStringConstant(std::string(EmbeddedNul));
    const auto Bom = Constants.getStringConstant("\xEF\xBB\xBF");
    const auto EscapeText = Constants.getStringConstant("\\u4E2D\\n");
    const auto Composed = Constants.getStringConstant("\xC3\xA9");
    const auto Decomposed = Constants.getStringConstant("e\xCC\x81");
    ASSERT_NE(BinarySafe, nullptr);
    ASSERT_NE(Bom, nullptr);
    ASSERT_NE(EscapeText, nullptr);
    ASSERT_NE(Composed, nullptr);
    ASSERT_NE(Decomposed, nullptr);
    EXPECT_EQ(BinarySafe->value(), EmbeddedNul);
    EXPECT_EQ(BinarySafe->byteLength(), 3U);
    EXPECT_EQ(Bom->value(), "\xEF\xBB\xBF");
    EXPECT_EQ(EscapeText->value(), "\\u4E2D\\n");
    EXPECT_EQ(Composed->value(), "\xC3\xA9");
    EXPECT_EQ(Decomposed->value(), "e\xCC\x81");
    EXPECT_NE(Composed->value(), Decomposed->value());
  }

  // Constants own both copied and moved text after the caller changes or destroys its source buffer.
  TEST(IRStringTest, OwnsCopiedAndMovedContent)
  {
    TypeContext Types(Target64);
    auto &Constants = Types.constantPool();
    std::string Source = "original";
    const auto Copied = Constants.getStringConstant(Source);
    ASSERT_NE(Copied, nullptr);
    Source.assign("changed");
    EXPECT_EQ(Copied->value(), "original");

    const StringConstant *Moved = nullptr;
    {
      std::string Temporary(256, 'x');
      Moved = Constants.getStringConstant(std::move(Temporary));
    }
    ASSERT_NE(Moved, nullptr);
    EXPECT_EQ(Moved->value(), std::string(256, 'x'));
    EXPECT_EQ(Moved->byteLength(), 256U);
  }

  // Construction rejects malformed UTF-8, including invalid bytes after NUL and Unicode surrogate or range violations.
  TEST(IRStringTest, RejectsInvalidUtf8)
  {
    TypeContext Types(Target64);
    auto &Constants = Types.constantPool();
    const std::string_view InvalidContents[] = {
        "\x80",
        "\xBF",
        "\xFF",
        "\xC0\x80",
        "\xC1\xBF",
        "\xC2",
        "\xC2\x20",
        "\xE0\x80\x80",
        "\xE2\x82",
        "\xED\xA0\x80",
        "\xED\xBF\xBF",
        "\xF0\x80\x80\x80",
        "\xF0\x90\x80",
        "\xF4\x90\x80\x80",
        "\xF5\x80\x80\x80",
        "\xF8\x88\x80\x80\x80",
        std::string_view("ok\0\xFF", 4),
    };
    for (std::size_t Index = 0; Index < std::size(InvalidContents); ++Index)
    {
      EXPECT_EQ(Constants.getStringConstant(std::string(InvalidContents[Index])), nullptr) << "invalid sequence " << Index;
    }
  }

  // String types compose with arrays and function signatures, and string constants retain element identity in array constants.
  TEST(IRStringTest, ComposesWithArraysAndFunctionSignatures)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      TypeContext Types(Target);
      auto &Constants = Types.constantPool();
      const auto String = Types.getStringType();
      ASSERT_NE(String, nullptr);
      const auto First = Constants.getStringConstant("first");
      const auto Second = Constants.getStringConstant("second");
      const auto Array = Types.getArrayType(String, 2);
      const auto Function = Types.getFunctionType(String, {String});
      ASSERT_NE(First, nullptr);
      ASSERT_NE(Second, nullptr);
      ASSERT_NE(Array, nullptr);
      ASSERT_NE(Function, nullptr);
      const auto Values = Constants.getArrayConstant(Array, {First, Second});
      ASSERT_NE(Values, nullptr);
      EXPECT_EQ(Array->size(), String->size() * 2);
      EXPECT_EQ(Array->alignment(), String->alignment());
      ASSERT_EQ(Values->elementCount(), 2U);
      EXPECT_EQ(Values->elements()[0], First);
      EXPECT_EQ(Values->elements()[1], Second);
      EXPECT_EQ(Function->returnType(), String);
      ASSERT_EQ(Function->parameterCount(), 1U);
      EXPECT_EQ(Function->parameters()[0], String);
    }
  }
} // namespace ink::ir::test
