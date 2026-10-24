#include "ink/core/numeric/integer_bits.h"
#include "ink/ir/context.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);
    constexpr std::uint64_t AllBits = std::numeric_limits<std::uint64_t>::max();
  } // namespace

  // Nonstandard integer widths are canonical per signedness and qualifier, including the documented maximum width.
  TEST(IRWideIntegerTypeTest, InternsArbitraryWidthsAndQualifiers)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    for (const std::uint32_t Width : {1U, 7U, 9U, 63U, 65U, 127U, 128U, 129U, 256U, 257U, core::IntegerBits::MaxBitWidth})
    {
      for (const bool Signed : {false, true})
      {
        SCOPED_TRACE(Width);
        SCOPED_TRACE(Signed);
        // Request the const variant first to exercise creation of its canonical unqualified type.
        const auto *Const = Types.get<ink::ir::IntegerType>(Width, Signed, false);
        const auto *Mutable = Types.get<ink::ir::IntegerType>(Width, Signed);
        ASSERT_NE(Const, nullptr);
        ASSERT_NE(Mutable, nullptr);
        EXPECT_NE(Const, Mutable);
        EXPECT_EQ(Const, Types.get<ink::ir::IntegerType>(Width, Signed, false));
        EXPECT_EQ(Mutable, Types.get<ink::ir::IntegerType>(Width, Signed));
        EXPECT_EQ(Const->unqualifiedType(), Mutable);
        EXPECT_EQ(Types.get(Mutable, false), Const);
        EXPECT_EQ(Types.get(Const, true), Mutable);
        EXPECT_EQ(Const->width(), Width);
        EXPECT_EQ(Const->isSigned(), Signed);
        EXPECT_FALSE(Const->isMutable());
        EXPECT_TRUE(Mutable->isMutable());
        EXPECT_TRUE(Context.containsType(Const));
        EXPECT_TRUE(Types.owns(Mutable));
        EXPECT_EQ(Mutable->toString(), std::string(Signed ? "i" : "u") + std::to_string(Width));
        EXPECT_EQ(Const->toString(), std::string(Signed ? "const i" : "const u") + std::to_string(Width));
        EXPECT_NE(Mutable, Types.get<ink::ir::IntegerType>(Width, !Signed));
      }
    }
  }

  // Type identity, qualifier links and builtin objects survive growth of the dynamic integer cache.
  TEST(IRWideIntegerTypeTest, KeepsIdentitiesStableAcrossCacheGrowth)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Wide = Types.get<ink::ir::IntegerType>(257, true);
    const auto *ConstWide = Types.get<ink::ir::IntegerType>(257, true, false);
    const auto *Builtin = Types.get<ink::ir::IntegerType>(64, true);
    const auto *ConstBuiltin = Types.get<ink::ir::IntegerType>(64, true, false);
    ASSERT_NE(Wide, nullptr);
    ASSERT_NE(ConstWide, nullptr);
    ASSERT_NE(Builtin, nullptr);
    ASSERT_NE(ConstBuiltin, nullptr);
    for (std::uint32_t Width = 1; Width <= 1024; ++Width)
    {
      ASSERT_NE(Types.get<ink::ir::IntegerType>(Width, false), nullptr);
      ASSERT_NE(Types.get<ink::ir::IntegerType>(Width, true, false), nullptr);
    }
    EXPECT_EQ(Wide, Types.get<ink::ir::IntegerType>(257, true));
    EXPECT_EQ(ConstWide, Types.get<ink::ir::IntegerType>(257, true, false));
    EXPECT_EQ(ConstWide->unqualifiedType(), Wide);
    EXPECT_EQ(Builtin, Types.get<ink::ir::IntegerType>(64, true));
    EXPECT_EQ(ConstBuiltin, Types.get<ink::ir::IntegerType>(64, true, false));
    EXPECT_EQ(ConstBuiltin->unqualifiedType(), Builtin);
    EXPECT_TRUE(Context.containsType(Wide));
    EXPECT_TRUE(Context.containsType(ConstWide));
  }

  // Signed and unsigned extrema preserve every word and clear bits beyond non-word-aligned widths.
  TEST(IRWideIntegerTypeTest, ProducesExactBoundsAcrossWordBoundaries)
  {
    struct Case
    {
        std::uint32_t Width;
        bool Signed;
        std::vector<std::uint64_t> Minimum;
        std::vector<std::uint64_t> Maximum;
    };
    const Case Cases[] = {
        {1, true, {1}, {0}},
        {1, false, {0}, {1}},
        {7, true, {0x40}, {0x3f}},
        {7, false, {0}, {0x7f}},
        {64, true, {0x8000000000000000ULL}, {0x7fffffffffffffffULL}},
        {64, false, {0}, {AllBits}},
        {65, true, {0, 1}, {AllBits, 0}},
        {65, false, {0, 0}, {AllBits, 1}},
        {127, true, {0, 0x4000000000000000ULL}, {AllBits, 0x3fffffffffffffffULL}},
        {128, true, {0, 0x8000000000000000ULL}, {AllBits, 0x7fffffffffffffffULL}},
        {128, false, {0, 0}, {AllBits, AllBits}},
        {129, true, {0, 0, 1}, {AllBits, AllBits, 0}},
        {256, true, {0, 0, 0, 0x8000000000000000ULL}, {AllBits, AllBits, AllBits, 0x7fffffffffffffffULL}},
        {257, false, {0, 0, 0, 0, 0}, {AllBits, AllBits, AllBits, AllBits, 1}},
    };
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    for (const auto &Entry : Cases)
    {
      const auto *Integer = Types.get<ink::ir::IntegerType>(Entry.Width, Entry.Signed);
      ASSERT_NE(Integer, nullptr);
      SCOPED_TRACE(Integer->toString());
      const auto Minimum = Integer->minimumValue();
      const auto Maximum = Integer->maximumValue();
      EXPECT_EQ(Minimum.bitWidth(), Entry.Width);
      EXPECT_EQ(Maximum.bitWidth(), Entry.Width);
      EXPECT_EQ(std::vector<std::uint64_t>(Minimum.words().begin(), Minimum.words().end()), Entry.Minimum);
      EXPECT_EQ(std::vector<std::uint64_t>(Maximum.words().begin(), Maximum.words().end()), Entry.Maximum);
      EXPECT_EQ(Minimum.isNegative(), Entry.Signed);
      if (Entry.Signed)
      {
        EXPECT_FALSE(Maximum.isNegative());
      }
      else
      {
        EXPECT_TRUE(Minimum.isZero());
      }
    }
  }

  // Signed host inputs are checked against the destination's numeric range, including one-bit and wider-than-host types.
  TEST(IRWideIntegerTypeTest, ChecksSignedHostValuesWithoutNarrowing)
  {
    struct Case
    {
        std::uint32_t Width;
        bool Signed;
        std::int64_t Value;
        bool Fits;
    };
    constexpr Case Cases[] = {
        {1, true, -1, true},
        {1, true, -2, false},
        {1, true, 0, true},
        {1, true, 1, false},
        {1, false, -1, false},
        {1, false, 1, true},
        {1, false, 2, false},
        {7, true, -64, true},
        {7, true, -65, false},
        {7, true, 63, true},
        {7, true, 64, false},
        {64, true, std::numeric_limits<std::int64_t>::min(), true},
        {63, true, std::numeric_limits<std::int64_t>::min(), false},
        {64, false, -1, false},
        {65, true, std::numeric_limits<std::int64_t>::min(), true},
        {128, true, std::numeric_limits<std::int64_t>::max(), true},
        {257, false, std::numeric_limits<std::int64_t>::max(), true},
        {257, false, std::numeric_limits<std::int64_t>::min(), false},
    };
    IRContext Context(Target64);
    for (const auto &Entry : Cases)
    {
      const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(Entry.Width, Entry.Signed);
      ASSERT_NE(Integer, nullptr);
      SCOPED_TRACE(Integer->toString());
      SCOPED_TRACE(Entry.Value);
      EXPECT_EQ(Integer->canRepresentSignedValue(Entry.Value), Entry.Fits);
    }
  }

  // Unsigned host inputs retain their high bit when checked against signed or unsigned destinations.
  TEST(IRWideIntegerTypeTest, ChecksUnsignedHostValuesWithoutNarrowing)
  {
    struct Case
    {
        std::uint32_t Width;
        bool Signed;
        std::uint64_t Value;
        bool Fits;
    };
    constexpr Case Cases[] = {
        {1, true, 0, true},
        {1, true, 1, false},
        {1, false, 1, true},
        {1, false, 2, false},
        {7, true, 63, true},
        {7, true, 64, false},
        {7, false, 127, true},
        {7, false, 128, false},
        {63, false, 0x7fffffffffffffffULL, true},
        {63, false, 0x8000000000000000ULL, false},
        {64, true, 0x7fffffffffffffffULL, true},
        {64, true, 0x8000000000000000ULL, false},
        {64, false, AllBits, true},
        {65, true, AllBits, true},
        {128, true, AllBits, true},
        {257, false, AllBits, true},
    };
    IRContext Context(Target64);
    for (const auto &Entry : Cases)
    {
      const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(Entry.Width, Entry.Signed);
      ASSERT_NE(Integer, nullptr);
      SCOPED_TRACE(Integer->toString());
      SCOPED_TRACE(Entry.Value);
      EXPECT_EQ(Integer->canRepresentUnsignedValue(Entry.Value), Entry.Fits);
    }
  }

  // Non-byte-aligned integers round storage up to bytes while arrays pad each element to the target alignment.
  TEST(IRWideIntegerTypeTest, ComposesStorageBytesAndArrayStride)
  {
    struct Case
    {
        std::uint32_t Width;
        std::uint64_t Bytes;
        std::uint64_t Alignment32;
        std::uint64_t Alignment64;
        std::uint64_t Stride32;
        std::uint64_t Stride64;
    };
    constexpr Case Cases[] = {
        {1, 1, 1, 1, 1, 1},
        {9, 2, 2, 2, 2, 2},
        {17, 3, 4, 4, 4, 4},
        {65, 9, 4, 16, 12, 16},
        {129, 17, 4, 16, 20, 32},
        {257, 33, 4, 16, 36, 48},
    };
    for (const auto &Target : {Target32, Target64})
    {
      IRContext Context(Target);
      auto &Types = Context.typePool();
      for (const auto &Entry : Cases)
      {
        const auto *Integer = Types.get<ink::ir::IntegerType>(Entry.Width, false);
        ASSERT_NE(Integer, nullptr);
        SCOPED_TRACE(Integer->toString());
        const bool Is64 = Target.pointerByteWidth() == 8;
        const auto Alignment = Is64 ? Entry.Alignment64 : Entry.Alignment32;
        const auto Stride = Is64 ? Entry.Stride64 : Entry.Stride32;
        EXPECT_EQ(Integer->size(), Entry.Bytes);
        EXPECT_EQ(Integer->alignment(), Alignment);
        const auto *Array = Types.get<ink::ir::ArrayType>(Integer, 3);
        ASSERT_NE(Array, nullptr);
        EXPECT_EQ(Array->size(), Stride * 3);
        EXPECT_EQ(Array->alignment(), Alignment);
        EXPECT_EQ(Array->elementType(), Integer);
      }
    }
  }

  // Equal wide type descriptions in separate contexts do not share ownership or qualify as local operands.
  TEST(IRWideIntegerTypeTest, KeepsWideTypesWithinTheirOwningContext)
  {
    IRContext Context(Target64);
    IRContext Other(Target64);
    auto &Types = Context.typePool();
    const auto *Local = Types.get<ink::ir::IntegerType>(128, true);
    const auto *Foreign = Other.typePool().get<ink::ir::IntegerType>(128, true);
    const auto *ForeignConst = Other.typePool().get<ink::ir::IntegerType>(128, true, false);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_NE(ForeignConst, nullptr);
    EXPECT_NE(Local, Foreign);
    EXPECT_FALSE(Context.containsType(Foreign));
    EXPECT_FALSE(Types.owns(ForeignConst));
    EXPECT_EQ(Types.get(Foreign, false), nullptr);
    EXPECT_EQ(Types.get<ink::ir::PointerType>(Foreign), nullptr);
    EXPECT_EQ(Types.get<ink::ir::ArrayType>(ForeignConst, 1), nullptr);
    EXPECT_EQ(Types.get<ink::ir::FunctionType>(Foreign), nullptr);
  }
} // namespace ink::ir::test
