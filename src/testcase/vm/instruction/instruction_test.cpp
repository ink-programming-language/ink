#include "ink/vm/instruction/instructions.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  static_assert(sizeof(Opcode) == 2);
  static_assert(std::is_same_v<std::underlying_type_t<Opcode>, std::uint16_t>);
  static_assert(OpcodeEncodingBytes == 2);
  static_assert(AddressOperandEncodingBytes == 12);
  static_assert(ImmediateOperandEncodingBytes == 8);

  template <typename InstructionType, typename = void>
  struct HasTypeAccessor : std::false_type
  {
  };

  template <typename InstructionType>
  struct HasTypeAccessor<InstructionType, std::void_t<decltype(std::declval<const InstructionType &>().getType())>> : std::true_type
  {
  };

  static_assert(!HasTypeAccessor<Instruction>::value);
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) static_assert(Kind##Instruction::Code == Opcode::Kind && Kind##Instruction::BitWidth == Width && !HasTypeAccessor<Kind##Instruction>::value);
#include "ink/vm/instruction/opcode.def"
#undef OPCODE

  template <typename InstructionType, typename... Operands>
  void checkOperandConstruction()
  {
    static_assert(std::is_constructible_v<InstructionType, Operands...>);
  }

  template <typename InstructionType>
  void checkAlloca()
  {
    checkOperandConstruction<InstructionType, AddressOperand, ImmediateOperand>();
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand>);
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, AddressOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getByteCount()), const ImmediateOperand &>);
    const InstructionType Allocation(AddressOperand(3, 16), ImmediateOperand(13));
    EXPECT_EQ(Allocation.getByteCount().bits(), 13U);
    ASSERT_TRUE(Allocation.getResult().has_value());
    EXPECT_EQ(Allocation.getResult()->baseIndex(), 3U);
    EXPECT_EQ(Allocation.getResult()->byteOffset(), 16);
  }

  template <typename InstructionType>
  void checkLoad()
  {
    checkOperandConstruction<InstructionType, AddressOperand, AddressOperand>();
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, AddressOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, ImmediateOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getSource()), const AddressOperand &>);
    const InstructionType Load(AddressOperand(4, -8), AddressOperand::bp(2));
    EXPECT_EQ(Load.getSource().baseIndex(), 0U);
    EXPECT_EQ(Load.getSource().byteOffset(), 2);
    ASSERT_TRUE(Load.getResult().has_value());
    EXPECT_EQ(Load.getResult()->baseIndex(), 4U);
    EXPECT_EQ(Load.getResult()->byteOffset(), -8);
  }

  template <typename InstructionType>
  void checkStorem()
  {
    checkOperandConstruction<InstructionType, AddressOperand, AddressOperand>();
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, AddressOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, ImmediateOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getDestination()), const AddressOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getSource()), const AddressOperand &>);
    const InstructionType Store(AddressOperand::bp(-2), AddressOperand::data(7, 7));
    EXPECT_FALSE(Store.getResult().has_value());
    EXPECT_EQ(Store.getDestination().baseIndex(), 0U);
    EXPECT_EQ(Store.getDestination().byteOffset(), -2);
    EXPECT_EQ(Store.getSource().baseIndex(), 7U);
    EXPECT_EQ(Store.getSource().byteOffset(), 7);
  }

  template <typename InstructionType>
  void checkStorei()
  {
    checkOperandConstruction<InstructionType, AddressOperand, ImmediateOperand>();
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, AddressOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getDestination()), const AddressOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getSource()), const ImmediateOperand &>);
    const InstructionType Store(AddressOperand(9, -13), ImmediateOperand(42));
    EXPECT_FALSE(Store.getResult().has_value());
    EXPECT_EQ(Store.getDestination().baseIndex(), 9U);
    EXPECT_EQ(Store.getDestination().byteOffset(), -13);
    EXPECT_EQ(Store.getSource().bits(), 42U);
  }

  template <typename InstructionType>
  void checkAddmm()
  {
    checkOperandConstruction<InstructionType, AddressOperand, AddressOperand, AddressOperand>();
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, AddressOperand, AddressOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, ImmediateOperand, AddressOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, AddressOperand, ImmediateOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getLHS()), const AddressOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getRHS()), const AddressOperand &>);
    const InstructionType Add(AddressOperand(5, 8), AddressOperand::bp(-4), AddressOperand::data(3, 12));
    EXPECT_EQ(Add.getLHS().baseIndex(), 0U);
    EXPECT_EQ(Add.getLHS().byteOffset(), -4);
    EXPECT_EQ(Add.getRHS().baseIndex(), 3U);
    EXPECT_EQ(Add.getRHS().byteOffset(), 12);
    ASSERT_TRUE(Add.getResult().has_value());
    EXPECT_EQ(Add.getResult()->baseIndex(), 5U);
    EXPECT_EQ(Add.getResult()->byteOffset(), 8);
  }

  template <typename InstructionType>
  void checkAddmi()
  {
    checkOperandConstruction<InstructionType, AddressOperand, AddressOperand, ImmediateOperand>();
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, AddressOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, ImmediateOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, AddressOperand, AddressOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getLHS()), const AddressOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getRHS()), const ImmediateOperand &>);
    const InstructionType Add(AddressOperand(6, 24), AddressOperand::data(3, -12), ImmediateOperand(17));
    EXPECT_EQ(Add.getLHS().baseIndex(), 3U);
    EXPECT_EQ(Add.getLHS().byteOffset(), -12);
    EXPECT_EQ(Add.getRHS().bits(), 17U);
    ASSERT_TRUE(Add.getResult().has_value());
    EXPECT_EQ(Add.getResult()->baseIndex(), 6U);
    EXPECT_EQ(Add.getResult()->byteOffset(), 24);
  }

  template <typename InstructionType>
  void checkAddii()
  {
    checkOperandConstruction<InstructionType, AddressOperand, ImmediateOperand, ImmediateOperand>();
    static_assert(!std::is_constructible_v<InstructionType, ImmediateOperand, ImmediateOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, AddressOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<InstructionType, AddressOperand, ImmediateOperand, AddressOperand>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getLHS()), const ImmediateOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const InstructionType &>().getRHS()), const ImmediateOperand &>);
    const InstructionType Add(AddressOperand(7, -24), ImmediateOperand(29), ImmediateOperand(31));
    EXPECT_EQ(Add.getLHS().bits(), 29U);
    EXPECT_EQ(Add.getRHS().bits(), 31U);
    ASSERT_TRUE(Add.getResult().has_value());
    EXPECT_EQ(Add.getResult()->baseIndex(), 7U);
    EXPECT_EQ(Add.getResult()->byteOffset(), -24);
  }

  // Every opcode definition maps to a distinct concrete class with matching spelling, width, and exclusive classof checks.
  TEST(VMInstructionTest, IdentifiesEveryOpcodeThroughBasePointers)
  {
#define DECLARE_Alloca(Kind) const Kind##Instruction Kind##Inst(AddressOperand(3), ImmediateOperand(5));
#define DECLARE_Load(Kind) const Kind##Instruction Kind##Inst(AddressOperand(3), AddressOperand::bp(2));
#define DECLARE_Storem(Kind) const Kind##Instruction Kind##Inst(AddressOperand::bp(-2), AddressOperand::data(3, 7));
#define DECLARE_Storei(Kind) const Kind##Instruction Kind##Inst(AddressOperand::bp(-2), ImmediateOperand(42));
#define DECLARE_Addmm(Kind) const Kind##Instruction Kind##Inst(AddressOperand(3), AddressOperand::bp(2), AddressOperand::data(3, 7));
#define DECLARE_Addmi(Kind) const Kind##Instruction Kind##Inst(AddressOperand(3), AddressOperand::bp(2), ImmediateOperand(42));
#define DECLARE_Addii(Kind) const Kind##Instruction Kind##Inst(AddressOperand(3), ImmediateOperand(7), ImmediateOperand(42));
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) DECLARE_##Form(Kind)
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
#undef DECLARE_Addii
#undef DECLARE_Addmi
#undef DECLARE_Addmm
#undef DECLARE_Storei
#undef DECLARE_Storem
#undef DECLARE_Load
#undef DECLARE_Alloca
    struct Case
    {
        const Instruction *Value;
        Opcode Code;
        std::uint16_t Encoding;
        std::string_view Name;
        std::uint32_t Width;
        std::uint32_t AddressCount;
        std::uint32_t ImmediateCount;
        bool (*Classof)(const Instruction *) noexcept;
    };
    const Case Cases[] = {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) {&Kind##Inst, Opcode::Kind, Encoding, Spelling, Width, AddressCount, ImmediateCount, &Kind##Instruction::classof},
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      EXPECT_EQ(Entry.Value->getOpcode(), Entry.Code);
      EXPECT_EQ(static_cast<std::uint16_t>(Entry.Code), Entry.Encoding);
      EXPECT_EQ(Entry.Value->getName(), Entry.Name);
      EXPECT_EQ(opcodeName(Entry.Code), Entry.Name);
      EXPECT_EQ(Entry.Value->bitWidth(), Entry.Width);
      EXPECT_EQ(opcodeBitWidth(Entry.Code), Entry.Width);
      const auto OperandCount = opcodeOperandCount(Entry.Code);
      ASSERT_TRUE(OperandCount.has_value());
      EXPECT_EQ(*OperandCount, Entry.AddressCount + Entry.ImmediateCount);
      const auto EncodedSize = opcodeEncodedSize(Entry.Code);
      ASSERT_TRUE(EncodedSize.has_value());
      EXPECT_EQ(*EncodedSize, 2U + 12U * Entry.AddressCount + 8U * Entry.ImmediateCount);
      EXPECT_FALSE(Entry.Classof(nullptr));
      for (const auto &Other : Cases)
      {
        SCOPED_TRACE(Other.Name);
        EXPECT_EQ(Entry.Classof(Other.Value), Entry.Code == Other.Code);
      }
    }
  }

  // All widths and memory/immediate forms preserve typed operands and results while rejecting mismatched constructor forms.
  TEST(VMInstructionTest, PreservesTypedOperandsForEveryOpcodeForm)
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) \
  {                                                                                 \
    SCOPED_TRACE(Spelling);                                                         \
    check##Form<Kind##Instruction>();                                               \
  }
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
  }

  // Alloca retains explicit byte counts, including a non-power-of-two size and the full unsigned 64-bit maximum.
  TEST(VMInstructionTest, PreservesExplicitAllocationByteCountsWithoutScaling)
  {
    const AllocaInstruction OddBytes(AddressOperand::bp(16), ImmediateOperand(13));
    const AllocaInstruction MaximumBytes(AddressOperand::data(7, 8), ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
    EXPECT_EQ(OddBytes.getByteCount().bits(), 13U);
    EXPECT_EQ(MaximumBytes.getByteCount().bits(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(OddBytes.bitWidth(), 0U);
    EXPECT_EQ(MaximumBytes.bitWidth(), 0U);
  }

  // Instructions keep independent operand values after callers replace every original address and immediate.
  TEST(VMInstructionTest, OwnsCopiesOfResultSourceAndDestinationOperands)
  {
    AddressOperand Result(3, 4);
    AddressOperand Source(7, -8);
    AddressOperand Destination(9, 12);
    ImmediateOperand Immediate(42);
    const Load32Instruction Load(Result, Source);
    const Store32mInstruction Storem(Destination, Source);
    const Store32iInstruction Storei(Destination, Immediate);
    const Add32miInstruction Add(Result, Source, Immediate);
    const AllocaInstruction Alloca(Result, Immediate);
    Result = AddressOperand(30, 40);
    Source = AddressOperand(70, 80);
    Destination = AddressOperand(90, 120);
    Immediate = ImmediateOperand(99);
    ASSERT_TRUE(Load.getResult().has_value());
    EXPECT_EQ(Load.getResult()->baseIndex(), 3U);
    EXPECT_EQ(Load.getResult()->byteOffset(), 4);
    EXPECT_EQ(Load.getSource().baseIndex(), 7U);
    EXPECT_EQ(Load.getSource().byteOffset(), -8);
    EXPECT_EQ(Storem.getDestination().baseIndex(), 9U);
    EXPECT_EQ(Storem.getDestination().byteOffset(), 12);
    EXPECT_EQ(Storem.getSource().baseIndex(), 7U);
    EXPECT_EQ(Storem.getSource().byteOffset(), -8);
    EXPECT_EQ(Storei.getDestination().baseIndex(), 9U);
    EXPECT_EQ(Storei.getDestination().byteOffset(), 12);
    EXPECT_EQ(Storei.getSource().bits(), 42U);
    ASSERT_TRUE(Add.getResult().has_value());
    EXPECT_EQ(Add.getResult()->baseIndex(), 3U);
    EXPECT_EQ(Add.getResult()->byteOffset(), 4);
    EXPECT_EQ(Add.getLHS().baseIndex(), 7U);
    EXPECT_EQ(Add.getLHS().byteOffset(), -8);
    EXPECT_EQ(Add.getRHS().bits(), 42U);
    ASSERT_TRUE(Alloca.getResult().has_value());
    EXPECT_EQ(Alloca.getResult()->baseIndex(), 3U);
    EXPECT_EQ(Alloca.getResult()->byteOffset(), 4);
    EXPECT_EQ(Alloca.getByteCount().bits(), 42U);
  }

  // Representative encodings remain stable and every declared opcode uses a unique 16-bit encoding.
  TEST(VMInstructionTest, PreservesStableUniqueOpcodeEncodings)
  {
    struct Case
    {
        Opcode Code;
        std::uint16_t Encoding;
        std::uint32_t OperandCount;
        std::uint32_t EncodedSize;
    };
    const Case Cases[] = {
        {Opcode::Alloca, 0x0000, 2, 22},
        {Opcode::Load64, 0x0004, 2, 26},
        {Opcode::Store64i, 0x000C, 2, 22},
        {Opcode::Add32mm, 0x0013, 3, 38},
        {Opcode::Add32mi, 0x0014, 3, 34},
        {Opcode::Add64ii, 0x0018, 3, 30},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(opcodeName(Entry.Code));
      EXPECT_EQ(static_cast<std::uint16_t>(Entry.Code), Entry.Encoding);
      const auto OperandCount = opcodeOperandCount(Entry.Code);
      ASSERT_TRUE(OperandCount.has_value());
      EXPECT_EQ(*OperandCount, Entry.OperandCount);
      const auto EncodedSize = opcodeEncodedSize(Entry.Code);
      ASSERT_TRUE(EncodedSize.has_value());
      EXPECT_EQ(*EncodedSize, Entry.EncodedSize);
    }
    const Opcode Codes[] = {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) Opcode::Kind,
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
    };
    constexpr std::size_t CodeCount = sizeof(Codes) / sizeof(Codes[0]);
    for (std::size_t Index = 0; Index < CodeCount; ++Index)
    {
      SCOPED_TRACE(opcodeName(Codes[Index]));
      for (std::size_t OtherIndex = Index + 1; OtherIndex < CodeCount; ++OtherIndex)
      {
        SCOPED_TRACE(opcodeName(Codes[OtherIndex]));
        EXPECT_NE(static_cast<std::uint16_t>(Codes[Index]), static_cast<std::uint16_t>(Codes[OtherIndex]));
      }
    }
  }

  // Unknown 16-bit opcodes have no operand count or encoded size and retain explicit invalid metadata.
  TEST(VMInstructionTest, NamesUnknownOpcodesWithoutMisclassification)
  {
    const Opcode Invalid = static_cast<Opcode>(0xFFFF);
    EXPECT_EQ(opcodeName(Invalid), "InvalidOpcode");
    EXPECT_EQ(opcodeBitWidth(Invalid), 0U);
    EXPECT_FALSE(opcodeOperandCount(Invalid).has_value());
    EXPECT_FALSE(opcodeEncodedSize(Invalid).has_value());
  }
} // namespace ink::vm::test
