#include "ink/ir/instruction/address_operand.h"
#include "ink/ir/instruction/immediate_operand.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_trivially_copyable_v<AddressOperand>);
  static_assert(std::is_trivially_copyable_v<ImmediateOperand>);
  static_assert(ImmediateOperand::fromSigned(-1).bits() == std::numeric_limits<std::uint64_t>::max());

  // BP-relative, symbolic, and arbitrary addresses retain their base slots and signed byte displacements as descriptions.
  TEST(IRAddressOperandTest, DescribesStackDataAndIndependentBaseSlots)
  {
    constexpr std::uint32_t SymbolSlot = 1;
    constexpr AddressOperand Stack = AddressOperand::bp(5);
    static_assert(Stack.baseIndex() == AddressOperand::BasePointerSlot);
    static_assert(Stack.byteOffset() == 5);
    const AddressOperand Symbol = AddressOperand::data(SymbolSlot, 42);
    const AddressOperand Independent(2, -7);
    EXPECT_EQ(Stack.baseIndex(), AddressOperand::BasePointerSlot);
    EXPECT_EQ(Stack.byteOffset(), 5);
    EXPECT_EQ(AddressOperand::bp().byteOffset(), 0);
    EXPECT_EQ(Symbol.baseIndex(), SymbolSlot);
    EXPECT_EQ(Symbol.byteOffset(), 42);
    EXPECT_EQ(Independent.baseIndex(), 2U);
    EXPECT_EQ(Independent.byteOffset(), -7);
  }

  // Symbols with equal displacements retain distinct explicit slots, and omitting the displacement records zero.
  TEST(IRAddressOperandTest, KeepsExplicitSymbolBaseSlotsIndependent)
  {
    constexpr std::uint32_t FirstSymbolSlot = 2;
    constexpr std::uint32_t SecondSymbolSlot = 4;
    const AddressOperand FirstSymbol = AddressOperand::data(FirstSymbolSlot, 8);
    const AddressOperand SecondSymbol = AddressOperand::data(SecondSymbolSlot, 8);
    const AddressOperand FirstSymbolBase = AddressOperand::data(FirstSymbolSlot);
    EXPECT_EQ(FirstSymbol.baseIndex(), FirstSymbolSlot);
    EXPECT_EQ(SecondSymbol.baseIndex(), SecondSymbolSlot);
    EXPECT_EQ(FirstSymbol.byteOffset(), 8);
    EXPECT_EQ(SecondSymbol.byteOffset(), 8);
    EXPECT_FALSE(FirstSymbol == SecondSymbol);
    EXPECT_EQ(FirstSymbolBase.baseIndex(), FirstSymbolSlot);
    EXPECT_EQ(FirstSymbolBase.byteOffset(), 0);
  }

  // Address adjustment preserves the base slot and rejects signed overflow, including subtraction of INT64_MIN.
  TEST(IRAddressOperandTest, ChecksSignedOffsetArithmeticAtBothLimits)
  {
    constexpr auto Minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto Maximum = std::numeric_limits<std::int64_t>::max();
    struct Case
    {
        std::int64_t Offset;
        std::int64_t Amount;
        std::optional<std::int64_t> Added;
        std::optional<std::int64_t> Subtracted;
    };
    const Case Cases[] = {
        {5, 17, 22, -12},
        {5, -17, -12, 22},
        {0, Minimum, Minimum, std::nullopt},
        {Minimum, Minimum, std::nullopt, 0},
        {-1, Minimum, std::nullopt, Maximum},
        {Maximum, 1, std::nullopt, Maximum - 1},
        {Minimum, -1, std::nullopt, Minimum + 1},
        {Maximum, -1, Maximum - 1, std::nullopt},
        {Minimum, 1, Minimum + 1, std::nullopt},
        {Maximum, Minimum, -1, std::nullopt},
        {0, Maximum, Maximum, -Maximum},
        {Minimum, 0, Minimum, Minimum},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Offset);
      SCOPED_TRACE(Entry.Amount);
      const AddressOperand Original(17, Entry.Offset);
      const auto Added = Original + Entry.Amount;
      const auto Subtracted = Original - Entry.Amount;
      ASSERT_EQ(Added.has_value(), Entry.Added.has_value());
      ASSERT_EQ(Subtracted.has_value(), Entry.Subtracted.has_value());
      if (Added)
      {
        EXPECT_EQ(Added->baseIndex(), 17U);
        EXPECT_EQ(Added->byteOffset(), *Entry.Added);
      }
      if (Subtracted)
      {
        EXPECT_EQ(Subtracted->baseIndex(), 17U);
        EXPECT_EQ(Subtracted->byteOffset(), *Entry.Subtracted);
      }
      EXPECT_EQ(Original, AddressOperand(17, Entry.Offset));
    }
  }

  // Replacing a copied descriptor cannot change the original base slot or displacement.
  TEST(IRAddressOperandTest, CopiesAddressDescriptorsByValue)
  {
    constexpr std::uint32_t CopySymbolSlot = 7;
    const AddressOperand Original = AddressOperand::bp(17);
    AddressOperand Copy = Original;
    Copy = AddressOperand::data(CopySymbolSlot, -5);
    EXPECT_EQ(Original, AddressOperand(AddressOperand::BasePointerSlot, 17));
    EXPECT_EQ(Copy, AddressOperand(CopySymbolSlot, -5));
    EXPECT_FALSE(Original == Copy);
  }

  // Integer immediates preserve all unsigned bits and the signed endpoint representation without host signed arithmetic.
  TEST(IRImmediateOperandTest, PreservesUnsignedAndSignedBitPatterns)
  {
    constexpr auto MaximumBits = std::numeric_limits<std::uint64_t>::max();
    constexpr auto MinimumSigned = std::numeric_limits<std::int64_t>::min();
    constexpr auto MaximumSigned = std::numeric_limits<std::int64_t>::max();
    EXPECT_EQ(ImmediateOperand(MaximumBits).bits(), MaximumBits);
    EXPECT_EQ(ImmediateOperand::fromSigned(-1).bits(), MaximumBits);
    EXPECT_EQ(ImmediateOperand::fromSigned(MinimumSigned).bits(), std::uint64_t{1} << 63);
    EXPECT_EQ(ImmediateOperand::fromSigned(MaximumSigned).bits(), static_cast<std::uint64_t>(MaximumSigned));
    EXPECT_EQ(ImmediateOperand::fromSigned(0).bits(), 0U);
  }

  // Floating point immediates retain negative zero and NaN payloads while float values occupy only the low 32 bits.
  TEST(IRImmediateOperandTest, PreservesFloatingPointBitPatterns)
  {
    constexpr std::uint32_t FloatNanBits = 0x7FC12345U;
    constexpr std::uint64_t DoubleNanBits = 0x7FF8123456789ABCULL;
    EXPECT_EQ(ImmediateOperand::fromFloat(-0.0F).bits(), 0x80000000ULL);
    EXPECT_EQ(ImmediateOperand::fromDouble(-0.0).bits(), 0x8000000000000000ULL);
    EXPECT_EQ(ImmediateOperand::fromFloat(1.5F).bits(), 0x3FC00000ULL);
    EXPECT_EQ(ImmediateOperand::fromDouble(1.5).bits(), 0x3FF8000000000000ULL);
    EXPECT_EQ(ImmediateOperand::fromFloat(std::bit_cast<float>(FloatNanBits)).bits(), static_cast<std::uint64_t>(FloatNanBits));
    EXPECT_EQ(ImmediateOperand::fromDouble(std::bit_cast<double>(DoubleNanBits)).bits(), DoubleNanBits);
  }
} // namespace ink::ir::test
