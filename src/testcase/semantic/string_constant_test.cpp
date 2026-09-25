#include "ink/semantic/model/constant/string_constant.h"
#include "ink/semantic/context.h"
#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    const SliceType *getStringType(SemanticContext &Context)
    {
      const IntegerType *Byte = Context.getIntegerType(8, false);
      return Byte ? Context.getSliceType(*Byte, AccessKind::ReadOnly) : nullptr;
    }
  } // namespace

  // Repeated literals and alternative spellings share one constant when their decoded token values are interned.
  TEST(SemanticStringConstantTest, RepeatedAndEquivalentDecodedLiteralsShareOneConstant)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const SliceType *String = getStringType(Context);
    ASSERT_NE(String, nullptr);
    std::vector<const StringConstant *> Constants;
    {
      core::FrontendContext Frontend(Compilation);
      const auto Tokens = tokenizer::tokenize(Frontend, R"ink(int main()
{
  io.print("hello,world");
  io.print("hello,world");
  io.print("\x68ello,world");
  io.print(r"hello,world");
})ink");
      ASSERT_TRUE(Tokens.succeeded());
      for (const tokenizer::Token &Token : Tokens.tokens())
      {
        if (Token.Kind == tokenizer::TokenKind::StringLiteral)
        {
          const auto *Info = std::get_if<tokenizer::StringInfo>(&Token.Payload);
          ASSERT_NE(Info, nullptr);
          const StringConstant *Value = Context.getStringConstant(*String, Info->Decoded);
          ASSERT_NE(Value, nullptr);
          Constants.push_back(Value);
        }
      }
    }
    ASSERT_EQ(Constants.size(), 4U);
    for (const StringConstant *Value : Constants)
    {
      EXPECT_EQ(Value, Constants.front());
      EXPECT_EQ(Value->value(), "hello,world");
      EXPECT_EQ(&Value->type(), String);
      EXPECT_TRUE(Context.constantPool().owns(*Value));
    }
    EXPECT_EQ(Constants.front(), Context.constantPool().getStringConstant(*String, "hello,world"));
    EXPECT_EQ(Context.constantPool().size(), 3U);
  }

  // Empty values, embedded NULs, UTF-8 and canonically equivalent Unicode retain their exact byte identity.
  TEST(SemanticStringConstantTest, FullByteSequenceDeterminesIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const SliceType *String = getStringType(Context);
    ASSERT_NE(String, nullptr);
    const std::string_view Payloads[] = {
        std::string_view{},
        std::string_view("\0", 1),
        std::string_view("\0\0", 2),
        "a",
        std::string_view("a\0", 2),
        std::string_view("a\0b", 3),
        std::string_view("a\0c", 3),
        "\u00e9",
        "e\u0301",
        "\u4e2d\U0001f600",
        "\\n",
        "\n",
    };
    std::vector<const StringConstant *> Constants;
    for (std::string_view Payload : Payloads)
    {
      SCOPED_TRACE(Constants.size());
      const StringConstant *Value = Pool.getStringConstant(*String, Payload);
      ASSERT_NE(Value, nullptr);
      EXPECT_EQ(Value->value(), Payload);
      EXPECT_EQ(Value, Context.getStringConstant(*String, std::string(Payload)));
      for (const StringConstant *Previous : Constants)
      {
        EXPECT_NE(Value, Previous);
      }
      Constants.push_back(Value);
      EXPECT_TRUE(Pool.owns(*Value));
      EXPECT_EQ(Pool.size(), Constants.size() + 2);
    }
    EXPECT_EQ(Constants.front(), Pool.getStringConstant(*String, ""));
    const std::string Longer = "a-suffix";
    EXPECT_EQ(Constants[3], Pool.getStringConstant(*String, std::string_view(Longer.data(), 1)));
    EXPECT_EQ(Pool.size(), Constants.size() + 2);
  }

  // Writable, signed-byte, wider-element, noninteger and foreign slices cannot type a string constant.
  TEST(SemanticStringConstantTest, InvalidAndForeignSliceTypesLeaveStorageUnchanged)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const SliceType *String = getStringType(Context);
    const SliceType *Foreign = getStringType(Other);
    const IntegerType *Byte = Context.getIntegerType(8, false);
    const IntegerType *SignedByte = Context.getIntegerType(8, true);
    const IntegerType *Wide = Context.getIntegerType(16, false);
    ASSERT_NE(String, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_NE(Byte, nullptr);
    ASSERT_NE(SignedByte, nullptr);
    ASSERT_NE(Wide, nullptr);
    const StringConstant *Value = Pool.getStringConstant(*String, "hello,world");
    ASSERT_NE(Value, nullptr);
    const SliceType *InvalidTypes[] = {
        Context.getSliceType(*Byte, AccessKind::ReadWrite),
        Context.getSliceType(*SignedByte, AccessKind::ReadOnly),
        Context.getSliceType(*Wide, AccessKind::ReadOnly),
        Context.getSliceType(Context.getBoolType(), AccessKind::ReadOnly),
        Context.getSliceType(*String, AccessKind::ReadOnly),
        Foreign,
    };
    for (const SliceType *Invalid : InvalidTypes)
    {
      ASSERT_NE(Invalid, nullptr);
      EXPECT_EQ(Pool.getStringConstant(*Invalid, "hello,world"), nullptr);
      EXPECT_EQ(Context.getStringConstant(*Invalid, ""), nullptr);
      EXPECT_EQ(Pool.size(), 3U);
    }
    EXPECT_EQ(Other.getStringConstant(*String, "hello,world"), nullptr);
    EXPECT_EQ(Other.constantPool().size(), 2U);
    EXPECT_EQ(Pool.getStringConstant(*String, "hello,world"), Value);
    EXPECT_TRUE(Pool.owns(*Value));
  }

  // The pool copies short and heap-backed inputs, and growth preserves both object addresses and borrowed byte views.
  TEST(SemanticStringConstantTest, OwnedBytesAndViewsSurviveMutationDestructionAndGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const SliceType *String = getStringType(Context);
    ASSERT_NE(String, nullptr);
    const StringConstant *Short = nullptr;
    const StringConstant *Long = nullptr;
    {
      std::string Source = "hello,world";
      Short = Pool.getStringConstant(*String, Source);
      ASSERT_NE(Short, nullptr);
      Source.assign(65536, 'x');
      Long = Pool.getStringConstant(*String, Source);
      ASSERT_NE(Long, nullptr);
      Source.front() = 'y';
      EXPECT_EQ(Short->value(), "hello,world");
      EXPECT_EQ(Long->value().front(), 'x');
    }
    const std::string_view ShortView = Short->value();
    const std::string_view LongView = Long->value();
    for (unsigned Index = 0; Index < 2048; ++Index)
    {
      const StringConstant *Item = Pool.getStringConstant(*String, "string-" + std::to_string(Index));
      ASSERT_NE(Item, nullptr);
      EXPECT_TRUE(Pool.owns(*Item));
    }
    EXPECT_EQ(Pool.size(), 2052U);
    EXPECT_EQ(Short->value().data(), ShortView.data());
    EXPECT_EQ(Long->value().data(), LongView.data());
    EXPECT_EQ(ShortView, "hello,world");
    EXPECT_EQ(LongView, std::string(65536, 'x'));
    EXPECT_EQ(Short, Context.getStringConstant(*String, ShortView));
    EXPECT_EQ(Long, Context.getStringConstant(*String, LongView));
    EXPECT_TRUE(Pool.owns(*Short));
    EXPECT_TRUE(Pool.owns(*Long));
    EXPECT_EQ(Pool.size(), 2052U);
  }

  // Stores preserve the exact string slice type, including element access, and reject foreign constants.
  TEST(SemanticStringConstantTest, OwnershipAndStoreTypesAreChecked)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const SliceType *LocalType = getStringType(Context);
    const SliceType *OtherType = getStringType(Other);
    ASSERT_NE(LocalType, nullptr);
    ASSERT_NE(OtherType, nullptr);
    EXPECT_EQ(LocalType->access(), AccessKind::ReadOnly);
    ASSERT_TRUE(IntegerType::classof(&LocalType->elementType()));
    const auto &Byte = static_cast<const IntegerType &>(LocalType->elementType());
    EXPECT_EQ(Byte.bitWidth(), 8U);
    EXPECT_FALSE(Byte.isSigned());
    const StringConstant *Local = Context.getStringConstant(*LocalType, "hello,world");
    const StringConstant *Foreign = Other.getStringConstant(*OtherType, "hello,world");
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_NE(Local, Foreign);
    EXPECT_TRUE(Context.constantPool().owns(*Local));
    EXPECT_FALSE(Context.constantPool().owns(*Foreign));
    EXPECT_TRUE(Other.constantPool().owns(*Foreign));
    EXPECT_FALSE(Other.constantPool().owns(*Local));
    AllocaInstruction *Slot = Context.createAllocaInstruction(*LocalType);
    ASSERT_NE(Slot, nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*Slot, *Foreign), nullptr);
    const SliceType *Writable = Context.getSliceType(Byte, AccessKind::ReadWrite);
    ASSERT_NE(Writable, nullptr);
    AllocaInstruction *WritableSlot = Context.createAllocaInstruction(*Writable);
    ASSERT_NE(WritableSlot, nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*WritableSlot, *Local), nullptr);
    const StoreInstruction *Store = Context.createStoreInstruction(*Slot, *Local);
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(&Store->storedValue(), Local);
  }
} // namespace ink::semantic::test
