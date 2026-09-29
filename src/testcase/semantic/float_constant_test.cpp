#include "ink/semantic/ir_builder.h"
#include "ink/semantic/model/constant/float_constant.h"
#include "ink/semantic/context.h"

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
  TEST(SemanticFloatConstantTest, ExactBitsDetermineIdentityInEveryFormat)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    std::size_t ExpectedSize = 2;
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Type = Context.typePool().getType<TypeKind::Float>(Format.Width);
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
      std::vector<const FloatConstant *> Constants;
      for (std::uint64_t Pattern : Patterns)
      {
        SCOPED_TRACE(Pattern);
        const FloatBits Payload(Format.Width, Pattern);
        const FloatConstant *Value = Pool.getFloatConstant(*Type, Payload);
        ASSERT_NE(Value, nullptr);
        EXPECT_EQ(&Value->type(), Type);
        EXPECT_EQ(Value->value().bitWidth(), Format.Width);
        EXPECT_EQ(Value->value().bits(), Pattern);
        EXPECT_EQ(Value, Context.constantPool().getFloatConstant(*Type, FloatBits(Format.Width, Pattern)));
        EXPECT_TRUE(Pool.owns(*Value));
        for (const FloatConstant *Previous : Constants)
        {
          EXPECT_NE(Value, Previous);
        }
        Constants.push_back(Value);
        EXPECT_EQ(Pool.size(), ++ExpectedSize);
      }
    }
  }

  // Equal numeric values in different formats, or equal raw bits in an integer, remain separate constants.
  TEST(SemanticFloatConstantTest, FormatAndConstantKindParticipateInIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    std::vector<const FloatConstant *> Constants;
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Float = Context.typePool().getType<TypeKind::Float>(Format.Width);
      const IntegerType *Integer = Context.typePool().getType<TypeKind::Integer>(Format.Width, false);
      ASSERT_NE(Float, nullptr);
      ASSERT_NE(Integer, nullptr);
      const FloatConstant *One = Context.constantPool().getFloatConstant(*Float, FloatBits(Format.Width, Format.One));
      const IntegerConstant *RawBits = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(Format.Width, Format.One));
      ASSERT_NE(One, nullptr);
      ASSERT_NE(RawBits, nullptr);
      EXPECT_NE(static_cast<const Constant *>(One), RawBits);
      for (const FloatConstant *Previous : Constants)
      {
        EXPECT_NE(One, Previous);
      }
      Constants.push_back(One);
    }
    EXPECT_EQ(Context.constantPool().size(), 8U);
  }

  // Foreign types and other IEEE widths cannot be inserted or implicitly converted.
  TEST(SemanticFloatConstantTest, InvalidFormatsAndForeignTypesLeaveStorageUnchanged)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    ConstantPool &Pool = Context.constantPool();
    for (const FloatFormat &Format : Formats)
    {
      SCOPED_TRACE(Format.Width);
      const FloatType *Local = Context.typePool().getType<TypeKind::Float>(Format.Width);
      const FloatType *Foreign = Other.typePool().getType<TypeKind::Float>(Format.Width);
      ASSERT_NE(Local, nullptr);
      ASSERT_NE(Foreign, nullptr);
      const FloatBits Payload(Format.Width, Format.One);
      const FloatConstant *One = Pool.getFloatConstant(*Local, Payload);
      ASSERT_NE(One, nullptr);
      const std::size_t OriginalSize = Pool.size();
      for (const FloatFormat &OtherFormat : Formats)
      {
        if (OtherFormat.Width != Format.Width)
        {
          EXPECT_EQ(Pool.getFloatConstant(*Local, FloatBits(OtherFormat.Width, OtherFormat.One)), nullptr);
        }
      }
      EXPECT_EQ(Context.constantPool().getFloatConstant(*Foreign, Payload), nullptr);
      EXPECT_EQ(Other.constantPool().getFloatConstant(*Local, Payload), nullptr);
      EXPECT_EQ(Pool.size(), OriginalSize);
      EXPECT_EQ(Other.constantPool().size(), 2U);
      EXPECT_EQ(Pool.getFloatConstant(*Local, Payload), One);
    }
  }

  // Unsupported widths and bits outside an IEEE encoding are rejected without masking or modifying the pool.
  TEST(SemanticFloatConstantTest, MalformedBitsLeaveStorageUnchanged)
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
      const FloatType *Type = Context.typePool().getType<TypeKind::Float>(Format.Width);
      ASSERT_NE(Type, nullptr);
      const FloatConstant *One = Pool.getFloatConstant(*Type, FloatBits(Format.Width, Format.One));
      ASSERT_NE(One, nullptr);
      const std::size_t OriginalSize = Pool.size();
      for (std::uint32_t Width : InvalidWidths)
      {
        const FloatBits Payload(Width, 0);
        EXPECT_FALSE(Payload.valid());
        EXPECT_EQ(Pool.getFloatConstant(*Type, Payload), nullptr);
      }
      if (Format.Width < 64)
      {
        const std::uint64_t Overflow = std::uint64_t{1} << Format.Width;
        const FloatBits Payload(Format.Width, Overflow | Format.One);
        EXPECT_FALSE(Payload.valid());
        EXPECT_EQ(Payload.bits(), Overflow | Format.One);
        EXPECT_EQ(Pool.getFloatConstant(*Type, Payload), nullptr);
      }
      EXPECT_EQ(One, Context.constantPool().getFloatConstant(*Type, FloatBits(Format.Width, Format.One)));
      EXPECT_TRUE(Pool.owns(*One));
      EXPECT_EQ(Pool.size(), OriginalSize);
    }
  }

  // Caller mutation/destruction and map growth preserve the saved payload and declaration initializer.
  TEST(SemanticFloatConstantTest, OwnedPayloadAndReferencesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    ConstantPool &Pool = Context.constantPool();
    const FloatType *Float = Context.typePool().getType<TypeKind::Float>(64);
    ASSERT_NE(Float, nullptr);
    const FloatConstant *One = nullptr;
    {
      FloatBits Payload(64, 0x3ff0000000000000);
      One = Pool.getFloatConstant(*Float, Payload);
      ASSERT_NE(One, nullptr);
      Payload = FloatBits(64, 0xbff0000000000000);
      EXPECT_EQ(One->value().bits(), 0x3ff0000000000000U);
    }
    const FloatBits &Payload = One->value();
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*Float);
    AllocaInstruction *Slot = SlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    auto StoreOwner = Factory.createDetachedStoreInstruction(*Slot, *One);
    const StoreInstruction *Store = StoreOwner.get();
    ASSERT_NE(Store, nullptr);
    for (std::uint64_t Index = 0; Index < 2048; ++Index)
    {
      const FloatConstant *Item = Pool.getFloatConstant(*Float, FloatBits(64, Index));
      ASSERT_NE(Item, nullptr);
      EXPECT_TRUE(Pool.owns(*Item));
    }
    EXPECT_EQ(Pool.size(), 2051U);
    EXPECT_EQ(One, Context.constantPool().getFloatConstant(*Float, Payload));
    EXPECT_EQ(&One->value(), &Payload);
    EXPECT_EQ(Payload.bits(), 0x3ff0000000000000U);
    EXPECT_EQ(&Store->storedValue(), One);
    EXPECT_EQ(&Slot->allocatedType(), Float);
    EXPECT_TRUE(Pool.owns(*One));
  }

  // Equal floating constants have separate context-local identities and stores require exact local types.
  TEST(SemanticFloatConstantTest, OwnershipAndStoreTypesAreChecked)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    SemanticContext Other(Compilation);
    const FloatType *LocalType = Context.typePool().getType<TypeKind::Float>(32);
    const FloatType *OtherType = Other.typePool().getType<TypeKind::Float>(32);
    ASSERT_NE(LocalType, nullptr);
    ASSERT_NE(OtherType, nullptr);
    const FloatBits Payload(32, 0x3f800000);
    const FloatConstant *LocalOne = Context.constantPool().getFloatConstant(*LocalType, Payload);
    const FloatConstant *OtherOne = Other.constantPool().getFloatConstant(*OtherType, Payload);
    ASSERT_NE(LocalOne, nullptr);
    ASSERT_NE(OtherOne, nullptr);
    EXPECT_NE(LocalOne, OtherOne);
    EXPECT_TRUE(Context.constantPool().owns(*LocalOne));
    EXPECT_FALSE(Context.constantPool().owns(*OtherOne));
    EXPECT_TRUE(Other.constantPool().owns(*OtherOne));
    EXPECT_FALSE(Other.constantPool().owns(*LocalOne));
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*LocalType);
    AllocaInstruction *Slot = SlotOwner.get();
    auto BoolSlotOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *BoolSlot = BoolSlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(BoolSlot, nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(*Slot, *OtherOne), nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(*BoolSlot, *LocalOne), nullptr);
    auto StoreOwner = Factory.createDetachedStoreInstruction(*Slot, *LocalOne);
    const StoreInstruction *Store = StoreOwner.get();
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(&Store->storedValue(), LocalOne);
  }
} // namespace ink::semantic::test
