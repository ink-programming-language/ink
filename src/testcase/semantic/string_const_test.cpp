#include "ink/semantic/model/context.h"
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
  TEST(SemanticStringConstTest, RepeatedAndEquivalentDecodedLiteralsShareOneConstant)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const SliceType *String = getStringType(Context);
    ASSERT_NE(String, nullptr);
    std::vector<const StringConst *> Constants;
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
          const StringConst *Value = Context.getStringConst(*String, Info->Decoded);
          ASSERT_NE(Value, nullptr);
          Constants.push_back(Value);
        }
      }
    }
    ASSERT_EQ(Constants.size(), 4U);
    for (const StringConst *Value : Constants)
    {
      EXPECT_EQ(Value, Constants.front());
      EXPECT_EQ(Value->value(), "hello,world");
      EXPECT_EQ(&Value->type(), String);
      EXPECT_TRUE(Context.constantPool().owns(*Value));
    }
    EXPECT_EQ(Constants.front(), Context.constantPool().getStringConst(*String, "hello,world"));
    EXPECT_EQ(Context.constantPool().size(), 3U);
  }

  // Empty values, embedded NULs, UTF-8 and canonically equivalent Unicode retain their exact byte identity.
  TEST(SemanticStringConstTest, FullByteSequenceDeterminesIdentity)
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
    std::vector<const StringConst *> Constants;
    for (std::string_view Payload : Payloads)
    {
      SCOPED_TRACE(Constants.size());
      const StringConst *Value = Pool.getStringConst(*String, Payload);
      ASSERT_NE(Value, nullptr);
      EXPECT_EQ(Value->value(), Payload);
      EXPECT_EQ(Value, Context.getStringConst(*String, std::string(Payload)));
      for (const StringConst *Previous : Constants)
      {
        EXPECT_NE(Value, Previous);
      }
      Constants.push_back(Value);
      EXPECT_TRUE(Pool.owns(*Value));
      EXPECT_EQ(Pool.size(), Constants.size() + 2);
    }
    EXPECT_EQ(Constants.front(), Pool.getStringConst(*String, ""));
    const std::string Longer = "a-suffix";
    EXPECT_EQ(Constants[3], Pool.getStringConst(*String, std::string_view(Longer.data(), 1)));
    EXPECT_EQ(Pool.size(), Constants.size() + 2);
  }

  // Writable, signed-byte, wider-element, noninteger and foreign slices cannot type a string constant.
  TEST(SemanticStringConstTest, InvalidAndForeignSliceTypesLeaveStorageUnchanged)
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
    const StringConst *Value = Pool.getStringConst(*String, "hello,world");
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
      EXPECT_EQ(Pool.getStringConst(*Invalid, "hello,world"), nullptr);
      EXPECT_EQ(Context.getStringConst(*Invalid, ""), nullptr);
      EXPECT_EQ(Pool.size(), 3U);
    }
    EXPECT_EQ(Other.getStringConst(*String, "hello,world"), nullptr);
    EXPECT_EQ(Other.constantPool().size(), 2U);
    EXPECT_EQ(Pool.getStringConst(*String, "hello,world"), Value);
    EXPECT_TRUE(Pool.owns(*Value));
  }

  // The pool copies short and heap-backed inputs, and growth preserves both object addresses and borrowed byte views.
  TEST(SemanticStringConstTest, OwnedBytesAndViewsSurviveMutationDestructionAndGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const SliceType *String = getStringType(Context);
    ASSERT_NE(String, nullptr);
    const StringConst *Short = nullptr;
    const StringConst *Long = nullptr;
    {
      std::string Source = "hello,world";
      Short = Pool.getStringConst(*String, Source);
      ASSERT_NE(Short, nullptr);
      Source.assign(65536, 'x');
      Long = Pool.getStringConst(*String, Source);
      ASSERT_NE(Long, nullptr);
      Source.front() = 'y';
      EXPECT_EQ(Short->value(), "hello,world");
      EXPECT_EQ(Long->value().front(), 'x');
    }
    const std::string_view ShortView = Short->value();
    const std::string_view LongView = Long->value();
    for (unsigned Index = 0; Index < 2048; ++Index)
    {
      const StringConst *Item = Pool.getStringConst(*String, "string-" + std::to_string(Index));
      ASSERT_NE(Item, nullptr);
      EXPECT_TRUE(Pool.owns(*Item));
    }
    EXPECT_EQ(Pool.size(), 2052U);
    EXPECT_EQ(Short->value().data(), ShortView.data());
    EXPECT_EQ(Long->value().data(), LongView.data());
    EXPECT_EQ(ShortView, "hello,world");
    EXPECT_EQ(LongView, std::string(65536, 'x'));
    EXPECT_EQ(Short, Context.getStringConst(*String, ShortView));
    EXPECT_EQ(Long, Context.getStringConst(*String, LongView));
    EXPECT_TRUE(Pool.owns(*Short));
    EXPECT_TRUE(Pool.owns(*Long));
    EXPECT_EQ(Pool.size(), 2052U);
  }

  // Strings are local constants with a read-only u8 slice type, and declaration initializers enforce that identity.
  TEST(SemanticStringConstTest, OwnershipAndDeclarationTypesAreChecked)
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
    const StringConst *Local = Context.getStringConst(*LocalType, "hello,world");
    const StringConst *Foreign = Other.getStringConst(*OtherType, "hello,world");
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_NE(Local, Foreign);
    EXPECT_TRUE(Context.constantPool().owns(*Local));
    EXPECT_FALSE(Context.constantPool().owns(*Foreign));
    EXPECT_TRUE(Other.constantPool().owns(*Foreign));
    EXPECT_FALSE(Other.constantPool().owns(*Local));
    const Name NameValue = Context.namePool().intern("Message");
    EXPECT_EQ(Context.createVariable(NameValue, BindingMutability::Immutable, Foreign), nullptr);
    Variable *Declaration = Context.createVariable(NameValue, BindingMutability::Immutable, Local);
    ASSERT_NE(Declaration, nullptr);
    const SliceType *Writable = Context.getSliceType(Byte, AccessKind::ReadWrite);
    ASSERT_NE(Writable, nullptr);
    EXPECT_FALSE(Declaration->setType(*Writable));
    EXPECT_EQ(Declaration->type(), nullptr);
    EXPECT_TRUE(Declaration->setType(*LocalType));
    EXPECT_EQ(Declaration->initializer(), Local);
  }
} // namespace ink::semantic::test
