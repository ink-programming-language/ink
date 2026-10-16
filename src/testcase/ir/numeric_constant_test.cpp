#include "ink/target/target_context.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Signed factories preserve two's-complement boundary patterns and reject values outside every supported width.
  TEST(IRNumericConstantTest, SignedIntegerBoundariesAndNegativeBits)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const auto Signed8 = Types.typePool().getInt8Type();
    const auto Signed16 = Types.typePool().getInt16Type();
    const auto Signed32 = Types.typePool().getIntegerType(32, true);
    const auto Signed64 = Types.typePool().getIntegerType(64, true);
    ASSERT_NE(Signed32, nullptr);
    ASSERT_NE(Signed64, nullptr);

    struct SignedCase
    {
        const IntegerType *Type;
        std::int64_t Value;
        std::uint64_t ExpectedBits;
    };
    const SignedCase Cases[] = {
        {Signed8, 0, 0},
        {Signed8, -1, 0xffULL},
        {Signed8, -128, 0x80ULL},
        {Signed8, 127, 0x7fULL},
        {Signed16, 0, 0},
        {Signed16, -1, 0xffffULL},
        {Signed16, -32768, 0x8000ULL},
        {Signed16, 32767, 0x7fffULL},
        {Signed32, 0, 0},
        {Signed32, -1, 0xffffffffULL},
        {Signed32, std::numeric_limits<std::int32_t>::min(), 0x80000000ULL},
        {Signed32, std::numeric_limits<std::int32_t>::max(), 0x7fffffffULL},
        {Signed64, 0, 0},
        {Signed64, -1, 0xffffffffffffffffULL},
        {Signed64, std::numeric_limits<std::int64_t>::min(), 0x8000000000000000ULL},
        {Signed64, std::numeric_limits<std::int64_t>::max(), 0x7fffffffffffffffULL},
    };
    for (const SignedCase &Case : Cases)
    {
      const auto Constant = Constants.getSignedIntegerConstant(Case.Type, Case.Value);
      ASSERT_NE(Constant, nullptr);
      EXPECT_EQ(Constant->type(), Case.Type);
      EXPECT_EQ(Constant->bits(), Case.ExpectedBits);
      EXPECT_EQ(Constant, Constants.getIntegerConstantFromBits(Case.Type, Case.ExpectedBits));
    }
    EXPECT_EQ(Constants.getSignedIntegerConstant(Signed8, -129), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Signed8, 128), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Signed16, -32769), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Signed16, 32768), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Signed32, static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::min()) - 1), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Signed32, static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max()) + 1), nullptr);
  }

  // Unsigned factories accept the entire width, including the high bit, without truncating overflow.
  TEST(IRNumericConstantTest, UnsignedIntegerBoundaries)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      const auto UnsignedType = Types.typePool().getIntegerType(Width, false);
      ASSERT_NE(UnsignedType, nullptr);
      const std::uint64_t Maximum = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
      for (const std::uint64_t Value : {std::uint64_t{0}, Maximum})
      {
        const auto Constant = Constants.getUnsignedIntegerConstant(UnsignedType, Value);
        ASSERT_NE(Constant, nullptr);
        EXPECT_EQ(Constant->bits(), Value);
        EXPECT_EQ(Constant->type(), UnsignedType);
      }
      if (Width < 64)
      {
        EXPECT_EQ(Constants.getUnsignedIntegerConstant(UnsignedType, Maximum + 1), nullptr);
        EXPECT_EQ(Constants.getUnsignedIntegerConstant(UnsignedType, std::numeric_limits<std::uint64_t>::max()), nullptr);
      }
    }
  }

  // Unsigned byte factories accept zero through 255 without truncation, reject signed calls and refuse foreign byte types.
  TEST(IRNumericConstantTest, UInt8FactoriesEnforceByteRangeAndOwnership)
  {
    for (const target::PointerWidth Width : {target::PointerWidth::Bits32, target::PointerWidth::Bits64})
    {
      const target::TargetContext ByteTarget(Width, target::ByteOrder::LittleEndian);
      IRContext TypesContext(ByteTarget);
      Module Types(TypesContext);
      IRContext OtherContext(ByteTarget);
      Module Other(OtherContext);
      auto &Constants = Types.constantPool();
      const auto Byte = Types.typePool().getUInt8Type();
      ASSERT_NE(Byte, nullptr);
      for (const std::uint64_t Value : {std::uint64_t{0}, std::uint64_t{255}})
      {
        const auto Unsigned = Constants.getUnsignedIntegerConstant(Byte, Value);
        const auto Bits = Constants.getIntegerConstantFromBits(Byte, Value);
        ASSERT_NE(Unsigned, nullptr);
        ASSERT_NE(Bits, nullptr);
        EXPECT_EQ(Unsigned, Bits);
        EXPECT_EQ(Unsigned->type(), Byte);
        EXPECT_EQ(Unsigned->bits(), Value);
      }
      for (const std::uint64_t Overflow : {std::uint64_t{256}, std::numeric_limits<std::uint64_t>::max()})
      {
        EXPECT_EQ(Constants.getUnsignedIntegerConstant(Byte, Overflow), nullptr);
        EXPECT_EQ(Constants.getIntegerConstantFromBits(Byte, Overflow), nullptr);
      }
      EXPECT_EQ(Constants.getSignedIntegerConstant(Byte, 0), nullptr);
      EXPECT_EQ(Constants.getSignedIntegerConstant(Byte, -1), nullptr);
      EXPECT_EQ(Constants.getSignedIntegerConstant(Byte, 255), nullptr);
      EXPECT_EQ(Constants.getUnsignedIntegerConstant(Other.typePool().getUInt8Type(), 0), nullptr);
      EXPECT_EQ(Constants.getIntegerConstantFromBits(Other.typePool().getUInt8Type(), 0), nullptr);
    }
  }

  // Numeric factories reject missing types and signedness mismatches even for an otherwise valid zero.
  TEST(IRNumericConstantTest, IntegerRejectsMissingTypeAndWrongSignedness)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    EXPECT_EQ(Constants.getIntegerConstantFromBits(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getUnsignedIntegerConstant(nullptr, 0), nullptr);
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      const auto SignedType = Types.typePool().getIntegerType(Width, true);
      const auto UnsignedType = Types.typePool().getIntegerType(Width, false);
      ASSERT_NE(SignedType, nullptr);
      ASSERT_NE(UnsignedType, nullptr);
      EXPECT_EQ(Constants.getUnsignedIntegerConstant(SignedType, 0), nullptr);
      EXPECT_EQ(Constants.getSignedIntegerConstant(UnsignedType, 0), nullptr);
      EXPECT_EQ(Constants.getSignedIntegerConstant(UnsignedType, -1), nullptr);
    }
  }

  // Raw integer bits accept every in-width pattern for either signedness and reject nonzero upper bits.
  TEST(IRNumericConstantTest, IntegerRawBitsHaveNoImplicitTruncation)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    for (const bool Signed : {false, true})
    {
      for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
      {
        const auto Integer = Types.typePool().getIntegerType(Width, Signed);
        ASSERT_NE(Integer, nullptr);
        const std::uint64_t Maximum = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
        const auto Constant = Constants.getIntegerConstantFromBits(Integer, Maximum);
        ASSERT_NE(Constant, nullptr);
        EXPECT_EQ(Constant->bits(), Maximum);
        if (Width < 64)
        {
          EXPECT_EQ(Constants.getIntegerConstantFromBits(Integer, Maximum + 1), nullptr);
          EXPECT_EQ(Constants.getIntegerConstantFromBits(Integer, std::numeric_limits<std::uint64_t>::max()), nullptr);
        }
      }
    }
  }

  // Floating-point raw bits retain signed zero, infinities and NaN payloads regardless of target byte order.
  TEST(IRNumericConstantTest, FloatPreservesSpecialValuePatterns)
  {
    struct FloatCase
    {
        std::uint32_t Width;
        std::uint64_t Bits;
    };
    constexpr FloatCase Cases[] = {
        {32, 0x00000000ULL},
        {32, 0x80000000ULL},
        {32, 0x7f800000ULL},
        {32, 0xff800000ULL},
        {32, 0x7fc01234ULL},
        {32, 0xff801234ULL},
        {64, 0x0000000000000000ULL},
        {64, 0x8000000000000000ULL},
        {64, 0x7ff0000000000000ULL},
        {64, 0xfff0000000000000ULL},
        {64, 0x7ff8000000001234ULL},
        {64, 0xfff0000000001234ULL},
        {64, 0xffffffffffffffffULL},
    };
    for (const target::ByteOrder Order : {target::ByteOrder::LittleEndian, target::ByteOrder::BigEndian})
    {
      const target::TargetContext FloatTarget(target::PointerWidth::Bits64, Order);
      IRContext FloatTypesContext(FloatTarget);
      Module FloatTypes(FloatTypesContext);
      auto &Constants = FloatTypes.constantPool();
      for (const FloatCase &Case : Cases)
      {
        const auto Float = FloatTypes.typePool().getFloatType(Case.Width);
        ASSERT_NE(Float, nullptr);
        const auto Constant = Constants.getFloatConstantFromBits(Float, Case.Bits);
        ASSERT_NE(Constant, nullptr);
        EXPECT_EQ(Constant->type(), Float);
        EXPECT_EQ(Constant->bits(), Case.Bits);
      }
    }
  }

  // Numeric floating-point factories select the local 32-bit or 64-bit type and reuse raw-bit constants for finite boundaries regardless of byte order.
  TEST(IRNumericConstantTest, FloatNumericFactoriesPreserveFiniteValues)
  {
    struct Float32Case
    {
        float Value;
        std::uint64_t ExpectedBits;
    };
    constexpr Float32Case Float32Cases[] = {
        {4.13f, 0x408428f6ULL},
        {std::numeric_limits<float>::min(), 0x00800000ULL},
        {std::numeric_limits<float>::denorm_min(), 0x00000001ULL},
        {std::numeric_limits<float>::max(), 0x7f7fffffULL},
        {std::numeric_limits<float>::lowest(), 0xff7fffffULL},
    };
    struct Float64Case
    {
        double Value;
        std::uint64_t ExpectedBits;
    };
    constexpr Float64Case Float64Cases[] = {
        {4.13, 0x4010851eb851eb85ULL},
        {std::numeric_limits<double>::min(), 0x0010000000000000ULL},
        {std::numeric_limits<double>::denorm_min(), 0x0000000000000001ULL},
        {std::numeric_limits<double>::max(), 0x7fefffffffffffffULL},
        {std::numeric_limits<double>::lowest(), 0xffefffffffffffffULL},
    };
    for (const target::ByteOrder Order : {target::ByteOrder::LittleEndian, target::ByteOrder::BigEndian})
    {
      const target::TargetContext FloatTarget(target::PointerWidth::Bits64, Order);
      IRContext FloatTypesContext(FloatTarget);
      Module FloatTypes(FloatTypesContext);
      auto &Constants = FloatTypes.constantPool();
      const auto Float32 = FloatTypes.typePool().getFloatType(32);
      const auto Float64 = FloatTypes.typePool().getFloatType(64);
      ASSERT_NE(Float32, nullptr);
      ASSERT_NE(Float64, nullptr);
      for (const Float32Case &Case : Float32Cases)
      {
        const auto Constant = Constants.getFloatConstant(Case.Value);
        ASSERT_NE(Constant, nullptr);
        EXPECT_EQ(Constant->type(), Float32);
        EXPECT_EQ(Constant->bits(), Case.ExpectedBits);
        EXPECT_EQ(Constant, Constants.getFloatConstantFromBits(Float32, Case.ExpectedBits));
      }
      for (const Float64Case &Case : Float64Cases)
      {
        const auto Constant = Constants.getFloatConstant(Case.Value);
        ASSERT_NE(Constant, nullptr);
        EXPECT_EQ(Constant->type(), Float64);
        EXPECT_EQ(Constant->bits(), Case.ExpectedBits);
        EXPECT_EQ(Constant, Constants.getFloatConstantFromBits(Float64, Case.ExpectedBits));
      }
    }
  }

  // Numeric floating-point factories preserve signed zero and infinities, and retain NaN classification.
  TEST(IRNumericConstantTest, FloatNumericFactoriesPreserveSpecialValues)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const auto Float32 = Types.typePool().getFloatType(32);
    const auto Float64 = Types.typePool().getFloatType(64);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(Float64, nullptr);
    struct SpecialCase
    {
        float Value32;
        double Value64;
        std::uint64_t ExpectedBits32;
        std::uint64_t ExpectedBits64;
    };
    constexpr SpecialCase Cases[] = {
        {0.0f, 0.0, 0x00000000ULL, 0x0000000000000000ULL},
        {-0.0f, -0.0, 0x80000000ULL, 0x8000000000000000ULL},
        {std::numeric_limits<float>::infinity(), std::numeric_limits<double>::infinity(), 0x7f800000ULL, 0x7ff0000000000000ULL},
        {-std::numeric_limits<float>::infinity(), -std::numeric_limits<double>::infinity(), 0xff800000ULL, 0xfff0000000000000ULL},
    };
    for (const SpecialCase &Case : Cases)
    {
      const auto Constant32 = Constants.getFloatConstant(Case.Value32);
      const auto Constant64 = Constants.getFloatConstant(Case.Value64);
      ASSERT_NE(Constant32, nullptr);
      ASSERT_NE(Constant64, nullptr);
      EXPECT_EQ(Constant32->type(), Float32);
      EXPECT_EQ(Constant64->type(), Float64);
      EXPECT_EQ(Constant32->bits(), Case.ExpectedBits32);
      EXPECT_EQ(Constant64->bits(), Case.ExpectedBits64);
    }
    const auto NaN32 = Constants.getFloatConstant(std::numeric_limits<float>::quiet_NaN());
    const auto NaN64 = Constants.getFloatConstant(std::numeric_limits<double>::quiet_NaN());
    ASSERT_NE(NaN32, nullptr);
    ASSERT_NE(NaN64, nullptr);
    EXPECT_EQ(NaN32->bits() & 0x7f800000ULL, 0x7f800000ULL);
    EXPECT_NE(NaN32->bits() & 0x007fffffULL, 0ULL);
    EXPECT_EQ(NaN64->bits() & 0x7ff0000000000000ULL, 0x7ff0000000000000ULL);
    EXPECT_NE(NaN64->bits() & 0x000fffffffffffffULL, 0ULL);
  }

  // Raw floating-point factories reject missing types and nonzero high bits instead of narrowing them.
  TEST(IRNumericConstantTest, FloatRejectsMissingTypeAndHighBits)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    EXPECT_EQ(Constants.getFloatConstantFromBits(nullptr, 0), nullptr);
    const auto Float32 = Types.typePool().getFloatType(32);
    ASSERT_NE(Float32, nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(Float32, 0x100000000ULL), nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(Float32, 0xffffffff80000000ULL), nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(Float32, std::numeric_limits<std::uint64_t>::max()), nullptr);
    const auto AllBits = Constants.getFloatConstantFromBits(Float32, 0xffffffffULL);
    ASSERT_NE(AllBits, nullptr);
    EXPECT_EQ(AllBits->bits(), 0xffffffffULL);
  }
} // namespace ink::ir::test
