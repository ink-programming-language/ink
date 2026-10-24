#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/module/module.h"
#include "ink/target/target_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <iterator>
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

  // String storage is an exact-size u8 array with no implicit terminator on either pointer width or byte order.
  TEST(IRStringTest, ByteArrayLayoutIsIndependentOfTarget)
  {
    for (const target::PointerWidth Width : {target::PointerWidth::Bits32, target::PointerWidth::Bits64})
    {
      for (const target::ByteOrder Order : {target::ByteOrder::LittleEndian, target::ByteOrder::BigEndian})
      {
        const target::TargetContext Target(Width, Order);
        IRContext TypesContext(Target);
        Module Types(TypesContext);
        const auto Text = Types.constantPool().get<ink::ir::StringConstant>("hello, world\n");
        ASSERT_NE(Text, nullptr);
        const Type *TypeValue = Text->type();
        EXPECT_EQ(TypeValue->kind(), ValueKind::Array);
        EXPECT_TRUE(TypeValue->hasLayout());
        EXPECT_EQ(TypeValue->size(), 13U);
        EXPECT_EQ(TypeValue->alignment(), 1U);
        EXPECT_EQ(TypeValue, Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), 13));
        EXPECT_EQ(Text->byteLength(), 13U);
        EXPECT_EQ(Text->value(), "hello, world\n");
      }
    }
  }

  // Empty text and UTF-8 scalar boundaries produce arrays whose element counts and storage sizes equal the encoded byte length.
  TEST(IRStringTest, ContentLengthCountsUtf8Bytes)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    constexpr std::string_view Contents[] = {
        "",
        "hello",
        "\xE4\xBD\xA0\xE5\xA5\xBD",
        "\xF0\x9F\x98\x80",
        "\x7F\xC2\x80\xDF\xBF\xE0\xA0\x80\xED\x9F\xBF\xEE\x80\x80\xEF\xBF\xBF\xF0\x90\x80\x80\xF4\x8F\xBF\xBF",
    };
    for (const std::string_view Expected : Contents)
    {
      const auto Text = Constants.get<ink::ir::StringConstant>(std::string(Expected));
      ASSERT_NE(Text, nullptr);
      EXPECT_EQ(Text->type(), Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), Expected.size()));
      EXPECT_EQ(Text->value(), Expected);
      EXPECT_EQ(Text->byteLength(), Expected.size());
      EXPECT_EQ(Text->type()->size(), Expected.size());
      ASSERT_TRUE(ArrayType::classof(Text->type()));
      const auto Array = static_cast<const ArrayType *>(Text->type());
      EXPECT_EQ(Array->elementType(), Types.typePool().get<ink::ir::IntegerType>(8, false));
      EXPECT_EQ(Array->elementCount(), Expected.size());
    }
    const auto Chinese = Constants.get<ink::ir::StringConstant>("\xE4\xBD\xA0\xE5\xA5\xBD");
    ASSERT_NE(Chinese, nullptr);
    EXPECT_EQ(Chinese->byteLength(), 6U);
    const auto Empty = Constants.get<ink::ir::StringConstant>("");
    ASSERT_NE(Empty, nullptr);
    EXPECT_EQ(Empty->type()->size(), 0U);
    EXPECT_EQ(Empty->type()->alignment(), 1U);
  }

  // String constants preserve embedded NUL, BOM, literal escape text and distinct normalization forms without rewriting content.
  TEST(IRStringTest, PreservesExactValidBytes)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const std::string_view EmbeddedNul("a\0b", 3);
    const auto BinarySafe = Constants.get<ink::ir::StringConstant>(std::string(EmbeddedNul));
    const auto Bom = Constants.get<ink::ir::StringConstant>("\xEF\xBB\xBF");
    const auto EscapeText = Constants.get<ink::ir::StringConstant>("\\u4E2D\\n");
    const auto Composed = Constants.get<ink::ir::StringConstant>("\xC3\xA9");
    const auto Decomposed = Constants.get<ink::ir::StringConstant>("e\xCC\x81");
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
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    std::string Source = "original";
    const auto Copied = Constants.get<ink::ir::StringConstant>(Source);
    ASSERT_NE(Copied, nullptr);
    Source.assign("changed");
    EXPECT_EQ(Copied->value(), "original");

    const StringConstant *Moved = nullptr;
    {
      std::string Temporary(256, 'x');
      Moved = Constants.get<ink::ir::StringConstant>(std::move(Temporary));
    }
    ASSERT_NE(Moved, nullptr);
    EXPECT_EQ(Moved->value(), std::string(256, 'x'));
    EXPECT_EQ(Moved->byteLength(), 256U);
  }

  // Construction rejects malformed UTF-8, including invalid bytes after NUL and Unicode surrogate or range violations.
  TEST(IRStringTest, RejectsInvalidUtf8)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
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
      EXPECT_EQ(Constants.get<ink::ir::StringConstant>(std::string(InvalidContents[Index])), nullptr) << "invalid sequence " << Index;
    }
  }

  // Equal-length strings share array types while distinct lengths cannot be mixed as elements of the same nested array.
  TEST(IRStringTest, ComposesWithArraysAndFunctionSignatures)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      Module Types(TypesContext);
      auto &Constants = Types.constantPool();
      const auto First = Constants.get<ink::ir::StringConstant>("first");
      const auto Second = Constants.get<ink::ir::StringConstant>("other");
      const auto Longer = Constants.get<ink::ir::StringConstant>("second");
      ASSERT_NE(First, nullptr);
      ASSERT_NE(Second, nullptr);
      ASSERT_NE(Longer, nullptr);
      const auto Bytes = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), 5);
      EXPECT_EQ(First->type(), Bytes);
      EXPECT_EQ(Second->type(), Bytes);
      EXPECT_NE(First, Second);
      EXPECT_NE(Longer->type(), Bytes);
      const auto Array = Types.typePool().get<ink::ir::ArrayType>(Bytes, 2);
      const auto Function = Types.typePool().get<ink::ir::FunctionType>(Bytes, {Bytes});
      ASSERT_NE(Array, nullptr);
      ASSERT_NE(Function, nullptr);
      const auto Values = Constants.get<ink::ir::ArrayConstant>(Array, {First, Second});
      ASSERT_NE(Values, nullptr);
      EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(Array, {First, Longer}), nullptr);
      EXPECT_EQ(Array->size(), 10U);
      EXPECT_EQ(Array->alignment(), 1U);
      ASSERT_EQ(Values->elementCount(), 2U);
      EXPECT_EQ(Values->elements()[0], First);
      EXPECT_EQ(Values->elements()[1], Second);
      EXPECT_EQ(Function->returnType(), Bytes);
      ASSERT_EQ(Function->parameterCount(), 1U);
      EXPECT_EQ(Function->parameters()[0].ParameterType, Bytes);
    }
  }

  // String initializers use ordinary u8 array compatibility for globals and fields, including a zero-byte array.
  TEST(IRStringTest, InitializesMatchingByteArrays)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const core::Name Name = core::NamePool::instance().intern("IRStringTest.Text");
    for (const std::string_view Contents : {std::string_view(), std::string_view("hello"), std::string_view("\xE4\xB8\xAD")})
    {
      const auto Text = Constants.get<ink::ir::StringConstant>(std::string(Contents));
      const auto Bytes = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), Contents.size());
      ASSERT_NE(Text, nullptr);
      ASSERT_NE(Bytes, nullptr);
      EXPECT_TRUE(Text->canInitialize(Bytes));
      GlobalVariable Variable(*Bytes, Name);
      ASSERT_TRUE(Variable.setInitializer(Text));
      EXPECT_EQ(Variable.initializer(), Text);
      const Field Member(Bytes, Name, 0, Text);
      EXPECT_TRUE(Member.valid());
      auto Owner = Types.createStructType(Bytes->size(), Bytes->alignment());
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Member));
      EXPECT_EQ(Owner->field(0)->initializer(), Text);
    }
  }

  // Mismatched lengths, non-byte elements and foreign-module text are rejected without replacing an existing initializer.
  TEST(IRStringTest, RejectsIncompatibleAndForeignByteArrays)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target64);
    Module OtherTypes(OtherTypesContext);
    auto &Constants = Types.constantPool();
    const auto Text = Constants.get<ink::ir::StringConstant>("hello");
    const auto Short = Constants.get<ink::ir::StringConstant>("hell");
    const auto Foreign = OtherTypes.constantPool().get<ink::ir::StringConstant>("hello");
    const auto Bytes = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), 5);
    const auto BoolArray = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 5);
    const auto OtherBytes = OtherTypes.typePool().get<ink::ir::ArrayType>(OtherTypes.typePool().get<ink::ir::IntegerType>(8, false), 5);
    const core::Name Name = core::NamePool::instance().intern("IRStringTest.Text");
    ASSERT_NE(Text, nullptr);
    ASSERT_NE(Short, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_NE(Bytes, nullptr);
    EXPECT_FALSE(Text->canInitialize(BoolArray));
    EXPECT_FALSE(Text->canInitialize(OtherBytes));
    EXPECT_FALSE(Foreign->canInitialize(Bytes));
    EXPECT_FALSE(Short->canInitialize(Bytes));
    GlobalVariable Variable(*Bytes, Name);
    ASSERT_TRUE(Variable.setInitializer(Text));
    EXPECT_FALSE(Variable.setInitializer(Short));
    EXPECT_FALSE(Variable.setInitializer(Foreign));
    EXPECT_EQ(Variable.initializer(), Text);
    EXPECT_FALSE(Field(Bytes, Name, 0, Short).valid());
    EXPECT_FALSE(Field(Bytes, Name, 0, Foreign).valid());
    EXPECT_FALSE(Field(BoolArray, Name, 0, Text).valid());
    const auto Array = Types.typePool().get<ink::ir::ArrayType>(Bytes, 1);
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(Array, {Foreign}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(Array, {Short}), nullptr);
    EXPECT_FALSE(Constants.owns(Foreign));
    EXPECT_FALSE(OtherTypes.constantPool().owns(Text));
  }

  // Equal pooled contents reuse one initialized read-only global while different contents receive independent storage.
  TEST(IRStringTest, ReusesReadOnlyStringStorage)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    const auto First = Context.constantPool().get<ink::ir::StringConstant>("first");
    const auto Repeated = Context.constantPool().get<ink::ir::StringConstant>(std::string("first"));
    const auto Other = Context.constantPool().get<ink::ir::StringConstant>("other");
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Repeated, nullptr);
    ASSERT_NE(Other, nullptr);
    ASSERT_EQ(First, Repeated);

    const auto FirstStorage = Owner.getOrCreateStringStorage(*First);
    ASSERT_NE(FirstStorage, nullptr);
    EXPECT_EQ(Owner.getOrCreateStringStorage(*Repeated), FirstStorage);
    EXPECT_EQ(Owner.globalVariableCount(), 1U);
    EXPECT_TRUE(Owner.owns(FirstStorage));
    EXPECT_TRUE(FirstStorage->isStorage());
    EXPECT_TRUE(FirstStorage->isInitialized());
    EXPECT_FALSE(FirstStorage->isMutable());
    EXPECT_FALSE(FirstStorage->hasName());
    EXPECT_EQ(FirstStorage->type(), Context.typePool().get(First->type(), false));
    EXPECT_EQ(FirstStorage->initializer(), First);

    const auto OtherStorage = Owner.getOrCreateStringStorage(*Other);
    ASSERT_NE(OtherStorage, nullptr);
    EXPECT_NE(OtherStorage, FirstStorage);
    EXPECT_EQ(OtherStorage->initializer(), Other);
    EXPECT_EQ(OtherStorage->type(), FirstStorage->type());
    EXPECT_EQ(Owner.getOrCreateStringStorage(*First), FirstStorage);
    EXPECT_EQ(Owner.globalVariableCount(), 2U);
  }

  // Modules borrowing one context share string contents but own separate globals for the same pooled constant.
  TEST(IRStringTest, KeepsStringStorageLocalToEachModule)
  {
    IRContext Context(Target64);
    Module FirstOwner(Context);
    Module SecondOwner(Context);
    const auto Text = Context.constantPool().get<ink::ir::StringConstant>("shared");
    ASSERT_NE(Text, nullptr);
    const auto FirstStorage = FirstOwner.getOrCreateStringStorage(*Text);
    const auto SecondStorage = SecondOwner.getOrCreateStringStorage(*Text);
    ASSERT_NE(FirstStorage, nullptr);
    ASSERT_NE(SecondStorage, nullptr);
    EXPECT_NE(FirstStorage, SecondStorage);
    EXPECT_EQ(FirstStorage->initializer(), Text);
    EXPECT_EQ(SecondStorage->initializer(), Text);
    EXPECT_TRUE(FirstOwner.owns(FirstStorage));
    EXPECT_FALSE(FirstOwner.owns(SecondStorage));
    EXPECT_TRUE(SecondOwner.owns(SecondStorage));
    EXPECT_FALSE(SecondOwner.owns(FirstStorage));
    EXPECT_EQ(FirstOwner.getOrCreateStringStorage(*Text), FirstStorage);
    EXPECT_EQ(SecondOwner.getOrCreateStringStorage(*Text), SecondStorage);
    EXPECT_EQ(FirstOwner.globalVariableCount(), 1U);
    EXPECT_EQ(SecondOwner.globalVariableCount(), 1U);
  }

  // Foreign-context strings are rejected even with equal bytes, preserving existing globals and the local cache.
  TEST(IRStringTest, RejectsForeignStringStorageWithoutSideEffects)
  {
    IRContext Context(Target64);
    IRContext OtherContext(Target64);
    Module Owner(Context);
    const auto LocalText = Context.constantPool().get<ink::ir::StringConstant>("same");
    const auto ForeignText = OtherContext.constantPool().get<ink::ir::StringConstant>("same");
    ASSERT_NE(LocalText, nullptr);
    ASSERT_NE(ForeignText, nullptr);
    EXPECT_EQ(Owner.getOrCreateStringStorage(*ForeignText), nullptr);
    EXPECT_EQ(Owner.globalVariableCount(), 0U);

    const auto LocalStorage = Owner.getOrCreateStringStorage(*LocalText);
    ASSERT_NE(LocalStorage, nullptr);
    EXPECT_EQ(Owner.getOrCreateStringStorage(*ForeignText), nullptr);
    EXPECT_EQ(Owner.globalVariableCount(), 1U);
    EXPECT_EQ(Owner.globalVariable(0), LocalStorage);
    EXPECT_EQ(LocalStorage->initializer(), LocalText);
    EXPECT_EQ(Owner.getOrCreateStringStorage(*LocalText), LocalStorage);
  }

  // Materialized string storage retains empty arrays, embedded NUL and multibyte UTF-8 without adding a terminator.
  TEST(IRStringTest, StringStoragePreservesExactByteLength)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    const std::string_view Contents[] = {
        "",
        std::string_view("a\0b", 3),
        "\xE4\xB8\xAD",
    };
    for (const std::string_view Expected : Contents)
    {
      const auto Text = Context.constantPool().get<ink::ir::StringConstant>(std::string(Expected));
      ASSERT_NE(Text, nullptr);
      const auto Storage = Owner.getOrCreateStringStorage(*Text);
      ASSERT_NE(Storage, nullptr);
      EXPECT_EQ(Storage->initializer(), Text);
      EXPECT_FALSE(Storage->isMutable());
      ASSERT_TRUE(ArrayType::classof(Storage->type()));
      const auto Array = static_cast<const ArrayType *>(Storage->type());
      EXPECT_EQ(Array->elementCount(), Expected.size());
      EXPECT_EQ(Array->size(), Expected.size());
      EXPECT_EQ(Array->alignment(), 1U);
      EXPECT_EQ(Text->value(), Expected);
      EXPECT_EQ(Text->byteLength(), Expected.size());
    }
    EXPECT_EQ(Owner.globalVariableCount(), std::size(Contents));
  }
} // namespace ink::ir::test
