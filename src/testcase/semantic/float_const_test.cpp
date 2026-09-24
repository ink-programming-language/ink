#include "ink/semantic/model/context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace ink::semantic::test
{
  namespace
  {
    struct FloatFormat
    {
        unsigned Width;
        unsigned FractionBits;
        std::uint64_t One;
        std::uint64_t Infinity;
    };

    constexpr FloatFormat Formats[] = {
        {16, 10, 0x3c00, 0x7c00},
        {32, 23, 0x3f800000, 0x7f800000},
        {64, 52, 0x3ff0000000000000, 0x7ff0000000000000},
    };
  } // namespace

  // Every IEEE format preserves finite limits, subnormals, signed zeros, infinities and all tested NaN bits.
  TEST(SemanticFloatConstTest, ExactBitsDetermineIdentityInEveryFormat)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    std::size_t ExpectedSize = 2;
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Type = Context.getFloatType(Format.Width);
      ASSERT_NE(Type, nullptr);
      const std::uint64_t Sign = std::uint64_t{1} << (Format.Width - 1);
      const std::uint64_t MinNormal = std::uint64_t{1} << Format.FractionBits;
      const std::uint64_t QuietNaN = Format.Infinity | (MinNormal >> 1);
      const std::uint64_t Patterns[] = {
          0,
          Sign,
          1,
          Sign | 1,
          MinNormal - 1,
          MinNormal,
          Format.One,
          Sign | Format.One,
          Format.Infinity - 1,
          Sign | (Format.Infinity - 1),
          Format.Infinity,
          Sign | Format.Infinity,
          QuietNaN,
          QuietNaN | 1,
          QuietNaN | 2,
          Sign | QuietNaN | 1,
          Format.Infinity | 1,
          Format.Infinity | 2,
          Sign | Format.Infinity | 1,
      };
      std::vector<const FloatConst *> Constants;
      for (std::uint64_t Pattern : Patterns)
      {
        SCOPED_TRACE(Pattern);
        const FloatBits Payload(Format.Width, Pattern);
        const FloatConst *Value = Pool.getFloatConst(*Type, Payload);
        ASSERT_NE(Value, nullptr);
        EXPECT_EQ(&Value->type(), Type);
        EXPECT_EQ(Value->value().bitWidth(), Format.Width);
        EXPECT_EQ(Value->value().bits(), Pattern);
        EXPECT_EQ(Value, Context.getFloatConst(*Type, FloatBits(Format.Width, Pattern)));
        EXPECT_TRUE(Pool.owns(*Value));
        for (const FloatConst *Previous : Constants)
        {
          EXPECT_NE(Value, Previous);
        }
        Constants.push_back(Value);
        EXPECT_EQ(Pool.size(), ++ExpectedSize);
      }
    }
  }

  // Equal numeric values in different formats, or equal raw bits in an integer, remain separate constants.
  TEST(SemanticFloatConstTest, FormatAndConstantKindParticipateInIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    std::vector<const FloatConst *> Constants;
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Float = Context.getFloatType(Format.Width);
      const IntegerType *Integer = Context.getIntegerType(Format.Width, false);
      ASSERT_NE(Float, nullptr);
      ASSERT_NE(Integer, nullptr);
      const FloatConst *One = Context.getFloatConst(*Float, FloatBits(Format.Width, Format.One));
      const IntegerConstant *RawBits = Context.getIntegerConstant(*Integer, IntegerBits(Format.Width, Format.One));
      ASSERT_NE(One, nullptr);
      ASSERT_NE(RawBits, nullptr);
      EXPECT_NE(static_cast<const Constant *>(One), RawBits);
      for (const FloatConst *Previous : Constants)
      {
        EXPECT_NE(One, Previous);
      }
      Constants.push_back(One);
    }
    EXPECT_EQ(Context.constantPool().size(), 8U);
  }

  // Foreign types and other IEEE widths cannot be inserted or implicitly converted.
  TEST(SemanticFloatConstTest, InvalidFormatsAndForeignTypesLeaveStorageUnchanged)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    ConstantPool &Pool = Context.constantPool();
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Local = Context.getFloatType(Format.Width);
      const FloatType *Foreign = Other.getFloatType(Format.Width);
      ASSERT_NE(Local, nullptr);
      ASSERT_NE(Foreign, nullptr);
      const FloatBits Payload(Format.Width, Format.One);
      const FloatConst *One = Pool.getFloatConst(*Local, Payload);
      ASSERT_NE(One, nullptr);
      const std::size_t OriginalSize = Pool.size();
      for (const FloatFormat &OtherFormat : Formats)
      {
        if (OtherFormat.Width != Format.Width)
        {
          EXPECT_EQ(Pool.getFloatConst(*Local, FloatBits(OtherFormat.Width, OtherFormat.One)), nullptr);
        }
      }
      EXPECT_EQ(Context.getFloatConst(*Foreign, Payload), nullptr);
      EXPECT_EQ(Other.getFloatConst(*Local, Payload), nullptr);
      EXPECT_EQ(Pool.size(), OriginalSize);
      EXPECT_EQ(Other.constantPool().size(), 2U);
      EXPECT_EQ(Pool.getFloatConst(*Local, Payload), One);
    }
  }

  // Unsupported widths and bits outside an IEEE encoding are rejected without masking or modifying the pool.
  TEST(SemanticFloatConstTest, MalformedBitsLeaveStorageUnchanged)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    constexpr std::uint32_t InvalidWidths[] = {
        0,
        1,
        8,
        15,
        17,
        31,
        33,
        63,
        65,
        128,
        UINT32_MAX,
    };
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Type = Context.getFloatType(Format.Width);
      ASSERT_NE(Type, nullptr);
      const FloatConst *One = Pool.getFloatConst(*Type, FloatBits(Format.Width, Format.One));
      ASSERT_NE(One, nullptr);
      const std::size_t OriginalSize = Pool.size();
      for (std::uint32_t Width : InvalidWidths)
      {
        const FloatBits Payload(Width, 0);
        EXPECT_FALSE(Payload.valid());
        EXPECT_EQ(Pool.getFloatConst(*Type, Payload), nullptr);
      }
      if (Format.Width < 64)
      {
        const std::uint64_t Overflow = std::uint64_t{1} << Format.Width;
        const FloatBits Payload(Format.Width, Overflow | Format.One);
        EXPECT_FALSE(Payload.valid());
        EXPECT_EQ(Payload.bits(), Overflow | Format.One);
        EXPECT_EQ(Pool.getFloatConst(*Type, Payload), nullptr);
      }
      EXPECT_EQ(One, Context.getFloatConst(*Type, FloatBits(Format.Width, Format.One)));
      EXPECT_TRUE(Pool.owns(*One));
      EXPECT_EQ(Pool.size(), OriginalSize);
    }
  }

  // Caller mutation/destruction and map growth preserve the saved payload and declaration initializer.
  TEST(SemanticFloatConstTest, OwnedPayloadAndReferencesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const FloatType *Float = Context.getFloatType(64);
    ASSERT_NE(Float, nullptr);
    const FloatConst *One = nullptr;
    {
      FloatBits Payload(64, 0x3ff0000000000000);
      One = Pool.getFloatConst(*Float, Payload);
      ASSERT_NE(One, nullptr);
      Payload = FloatBits(64, 0xbff0000000000000);
      EXPECT_EQ(One->value().bits(), 0x3ff0000000000000U);
    }
    const FloatBits &Payload = One->value();
    Variable *Declaration = Context.createVariable(Context.namePool().intern("Number"), BindingMutability::Immutable, One);
    ASSERT_NE(Declaration, nullptr);
    ASSERT_TRUE(Declaration->setType(*Float));
    for (std::uint64_t Index = 0; Index < 2048; ++Index)
    {
      const FloatConst *Item = Pool.getFloatConst(*Float, FloatBits(64, Index));
      ASSERT_NE(Item, nullptr);
      EXPECT_TRUE(Pool.owns(*Item));
    }
    EXPECT_EQ(Pool.size(), 2051U);
    EXPECT_EQ(One, Context.getFloatConst(*Float, Payload));
    EXPECT_EQ(&One->value(), &Payload);
    EXPECT_EQ(Payload.bits(), 0x3ff0000000000000U);
    EXPECT_EQ(Declaration->initializer(), One);
    EXPECT_EQ(Declaration->type(), Float);
    EXPECT_TRUE(Pool.owns(*One));
  }

  // Equal floating constants have separate context-local identities and reject foreign declaration use.
  TEST(SemanticFloatConstTest, OwnershipAndDeclarationTypesAreChecked)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const FloatType *LocalType = Context.getFloatType(32);
    const FloatType *OtherType = Other.getFloatType(32);
    ASSERT_NE(LocalType, nullptr);
    ASSERT_NE(OtherType, nullptr);
    const FloatBits Payload(32, 0x3f800000);
    const FloatConst *LocalOne = Context.getFloatConst(*LocalType, Payload);
    const FloatConst *OtherOne = Other.getFloatConst(*OtherType, Payload);
    ASSERT_NE(LocalOne, nullptr);
    ASSERT_NE(OtherOne, nullptr);
    EXPECT_NE(LocalOne, OtherOne);
    EXPECT_TRUE(Context.constantPool().owns(*LocalOne));
    EXPECT_FALSE(Context.constantPool().owns(*OtherOne));
    EXPECT_TRUE(Other.constantPool().owns(*OtherOne));
    EXPECT_FALSE(Other.constantPool().owns(*LocalOne));
    const Name NameValue = Context.namePool().intern("Number");
    EXPECT_EQ(Context.createVariable(NameValue, BindingMutability::Immutable, OtherOne), nullptr);
    Variable *Declaration = Context.createVariable(NameValue, BindingMutability::Immutable, LocalOne);
    ASSERT_NE(Declaration, nullptr);
    EXPECT_FALSE(Declaration->setType(Context.getBoolType()));
    EXPECT_EQ(Declaration->type(), nullptr);
    EXPECT_TRUE(Declaration->setType(*LocalType));
    EXPECT_EQ(Declaration->initializer(), LocalOne);
  }
} // namespace ink::semantic::test
