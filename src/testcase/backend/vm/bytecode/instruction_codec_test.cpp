#include "ink/backend/vm/bytecode/instruction_codec.h"

#include "ink/backend/vm/instruction/machine_instructions.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  namespace
  {
    BytecodeOperand registerOperand(RegisterId Register)
    {
      return {BytecodeOperandKind::Register, Register};
    }

    BytecodeOperand addressOperand(EncodedAddressBase Base, std::uint64_t Bits)
    {
      return {BytecodeOperandKind::Address, RegisterId::BP, Base, Bits};
    }

    BytecodeOperand immediateOperand(std::uint64_t Bits)
    {
      return {BytecodeOperandKind::Immediate, RegisterId::BP, EncodedAddressBase::BP, Bits};
    }

    template <typename OperandType>
    BytecodeOperand sampleOperand(std::size_t Index, std::uint32_t Width)
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        return registerOperand(Index == 0 ? RegisterId::SP : RegisterId::PC);
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        return addressOperand(EncodedAddressBase::BP, 0xfffffffffffffff0ULL + Index);
      }
      else if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
      {
        return addressOperand(EncodedAddressBase::PC, 0xffffffffffffffe0ULL);
      }
      else
      {
        static_assert(std::is_same_v<OperandType, ImmediateOperand>);
        const std::uint64_t Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
        return immediateOperand((0xfedcba9876543210ULL + Index) & Mask);
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    BytecodeInstruction sampleInstruction(std::index_sequence<Indices...>)
    {
      using Traits = OpcodeTraits<Operation>;
      return {Operation, {sampleOperand<std::tuple_element_t<Indices, typename Traits::OperandTuple>>(Indices, Traits::BitWidth)...}, static_cast<std::uint8_t>(Traits::OperandCount)};
    }

    template <Opcode Operation>
    void expectOpcodeRoundTrip()
    {
      const auto Instruction = sampleInstruction<Operation>(std::make_index_sequence<OpcodeTraits<Operation>::OperandCount>{});
      for (const auto Order : {ByteOrder::Little, ByteOrder::Big})
      {
        SCOPED_TRACE(static_cast<std::uint16_t>(Operation));
        SCOPED_TRACE(static_cast<std::uint8_t>(Order));
        const auto Bytes = InstructionCodec::encode(Instruction, Order);
        ASSERT_TRUE(Bytes);
        EXPECT_EQ(InstructionCodec::encodeUnchecked(Instruction, Order), *Bytes);
        EXPECT_EQ(InstructionCodec::byteSize(Operation), Bytes->size());
        const auto Decoded = InstructionCodec::decode(*Bytes, Order);
        ASSERT_TRUE(Decoded);
        EXPECT_EQ(Decoded->ByteSize, Bytes->size());
        EXPECT_EQ(Decoded->Instruction.Operation, Operation);
        ASSERT_EQ(Decoded->Instruction.OperandCount, Instruction.OperandCount);
        for (std::size_t Index = 0; Index < Instruction.OperandCount; ++Index)
        {
          EXPECT_EQ(Decoded->Instruction.Operands[Index], Instruction.Operands[Index]);
        }
        for (std::size_t Size = 0; Size < Bytes->size(); ++Size)
        {
          const auto Truncated = InstructionCodec::decode(std::span<const std::uint8_t>(*Bytes).first(Size), Order);
          ASSERT_FALSE(Truncated);
          EXPECT_EQ(Truncated.error(), BytecodeError::TruncatedInstruction);
        }
      }
    }
  } // namespace

  // Every opcode uses identical checked and trusted encodings, round-trips in both byte orders, and rejects every shorter prefix.
  TEST(VMInstructionCodecTest, RoundTripsAllOpcodesAndRejectsTruncation)
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) expectOpcodeRoundTrip<Opcode::Kind>();
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
  }

  // The documented SP adjustment has a fixed 12-byte encoding, including a negative displacement.
  TEST(VMInstructionCodecTest, EncodesKnownRegisterAdjustmentBytes)
  {
    const BytecodeInstruction Positive{Opcode::Add64rri, {registerOperand(RegisterId::SP), registerOperand(RegisterId::SP), immediateOperand(16)}, 3};
    const std::vector<std::uint8_t> LittlePositive{0x1b, 0x00, 0x01, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::vector<std::uint8_t> BigPositive{0x00, 0x1b, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
    ASSERT_TRUE(InstructionCodec::encode(Positive, ByteOrder::Little));
    EXPECT_EQ(*InstructionCodec::encode(Positive, ByteOrder::Little), LittlePositive);
    EXPECT_EQ(*InstructionCodec::encode(Positive, ByteOrder::Big), BigPositive);
    auto Negative = Positive;
    Negative.Operands[2].Bits = 0xfffffffffffffff0ULL;
    const std::vector<std::uint8_t> LittleNegative{0x1b, 0x00, 0x01, 0x01, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    EXPECT_EQ(*InstructionCodec::encode(Negative), LittleNegative);
  }

  // Branches, VM direct calls, native import calls, and RET retain their specified wire sizes.
  TEST(VMInstructionCodecTest, EncodesKnownControlFlowBytes)
  {
    const BytecodeInstruction Branch{Opcode::Jne, {addressOperand(EncodedAddressBase::PC, 0xfffffffffffffff5ULL)}, 1};
    const std::vector<std::uint8_t> BranchBytes{0x2d, 0x00, 0x02, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    EXPECT_EQ(*InstructionCodec::encode(Branch), BranchBytes);
    const BytecodeInstruction Call{Opcode::Calli, {immediateOperand(0x0807060504030201ULL)}, 1};
    const std::vector<std::uint8_t> CallBytes{0x3b, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    EXPECT_EQ(*InstructionCodec::encode(Call), CallBytes);
    const BytecodeInstruction Native{Opcode::CallNativei, {immediateOperand(0x01020304)}, 1};
    const std::vector<std::uint8_t> NativeBytes{0x00, 0x3c, 0x01, 0x02, 0x03, 0x04};
    EXPECT_EQ(*InstructionCodec::encode(Native, ByteOrder::Big), NativeBytes);
    const std::vector<std::uint8_t> ReturnBytes{0x3a, 0x00};
    EXPECT_EQ(*InstructionCodec::encode(BytecodeInstruction{}), ReturnBytes);
  }

  // Explicit native-address operands preserve all 64 address bits independently of byte order.
  TEST(VMInstructionCodecTest, RoundTripsNativeAddresses)
  {
    const BytecodeInstruction Instruction{Opcode::Load64, {addressOperand(EncodedAddressBase::SP, 8), addressOperand(EncodedAddressBase::Native, 0xfedcba9876543210ULL)}, 2};
    for (const auto Order : {ByteOrder::Little, ByteOrder::Big})
    {
      const auto Bytes = InstructionCodec::encode(Instruction, Order);
      ASSERT_TRUE(Bytes);
      ASSERT_EQ(Bytes->size(), 20);
      EXPECT_EQ((*Bytes)[11], 0xff);
      const auto Decoded = InstructionCodec::decode(*Bytes, Order);
      ASSERT_TRUE(Decoded);
      EXPECT_EQ(Decoded->Instruction.Operands[1], Instruction.Operands[1]);
    }
  }

  // Decoding consumes one instruction and leaves trailing bytes for the caller's next decode.
  TEST(VMInstructionCodecTest, ReportsConsumedSizeWithinAStream)
  {
    const std::array<std::uint8_t, 5> Bytes{0x3a, 0x00, 0x3a, 0x00, 0xff};
    const auto First = InstructionCodec::decode(Bytes);
    ASSERT_TRUE(First);
    EXPECT_EQ(First->ByteSize, 2);
    EXPECT_EQ(First->Instruction.Operation, Opcode::Ret);
    const auto Second = InstructionCodec::decode(std::span<const std::uint8_t>(Bytes).subspan(First->ByteSize));
    ASSERT_TRUE(Second);
    EXPECT_EQ(Second->ByteSize, 2);
  }

  // Unknown opcodes and invalid byte-order enum values fail before operands are interpreted.
  TEST(VMInstructionCodecTest, RejectsUnknownOpcodesAndInvalidByteOrders)
  {
    const std::array<std::uint8_t, 2> UnknownBytes{0xff, 0xff};
    const auto Unknown = InstructionCodec::decode(UnknownBytes);
    ASSERT_FALSE(Unknown);
    EXPECT_EQ(Unknown.error(), BytecodeError::UnknownOpcode);
    const std::array<std::uint8_t, 2> UnassignedBytes{0x00, 0x00};
    EXPECT_EQ(InstructionCodec::decode(UnassignedBytes).error(), BytecodeError::UnknownOpcode);
    EXPECT_FALSE(InstructionCodec::byteSize(static_cast<Opcode>(0xffff)));
    auto Instruction = BytecodeInstruction{};
    Instruction.Operation = static_cast<Opcode>(0xffff);
    EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::UnknownOpcode);
    const auto InvalidOrder = static_cast<ByteOrder>(0xff);
    EXPECT_EQ(InstructionCodec::encode(BytecodeInstruction{}, InvalidOrder).error(), BytecodeError::InvalidByteOrder);
    EXPECT_EQ(InstructionCodec::decode({}, InvalidOrder).error(), BytecodeError::InvalidByteOrder);
  }

  // Encoder validation rejects inconsistent operand shapes without indexing outside inline storage.
  TEST(VMInstructionCodecTest, RejectsIncorrectOperandCountsAndKinds)
  {
    BytecodeInstruction Instruction{Opcode::Calli, {immediateOperand(8)}, 0};
    EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::InvalidOperandCount);
    Instruction.OperandCount = 255;
    EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::InvalidOperandCount);
    Instruction.OperandCount = 1;
    Instruction.Operands[0] = registerOperand(RegisterId::BP);
    EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::InvalidOperandKind);
    Instruction.Operands[0].Kind = static_cast<BytecodeOperandKind>(255);
    EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::InvalidOperandKind);
  }

  // PC is valid as a source but is never a register destination; EFLAGS and unknown IDs are rejected.
  TEST(VMInstructionCodecTest, EnforcesExplicitRegisterAccessRules)
  {
    BytecodeInstruction Instruction{Opcode::Add64rri, {registerOperand(RegisterId::SP), registerOperand(RegisterId::PC), immediateOperand(8)}, 3};
    ASSERT_TRUE(InstructionCodec::encode(Instruction));
    Instruction.Operands[0].Register = RegisterId::PC;
    EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::InvalidRegister);
    Instruction.Operands[0].Register = RegisterId::SP;
    for (const auto Invalid : {RegisterId::EFLAGS, static_cast<RegisterId>(0xfe)})
    {
      Instruction.Operands[1].Register = Invalid;
      EXPECT_EQ(InstructionCodec::encode(Instruction).error(), BytecodeError::InvalidRegister);
    }
    Instruction.Operands[1].Register = RegisterId::PC;
    auto Bytes = *InstructionCodec::encode(Instruction);
    Bytes[2] = static_cast<std::uint8_t>(RegisterId::PC);
    EXPECT_EQ(InstructionCodec::decode(Bytes).error(), BytecodeError::InvalidRegister);
    Bytes[2] = static_cast<std::uint8_t>(RegisterId::BP);
    Bytes[3] = static_cast<std::uint8_t>(RegisterId::EFLAGS);
    EXPECT_EQ(InstructionCodec::decode(Bytes).error(), BytecodeError::InvalidRegister);
  }

  // Memory bases accept only BP/SP/PC/native, and branch targets specifically require PC-relative addresses.
  TEST(VMInstructionCodecTest, RejectsInvalidMemoryAndBranchBases)
  {
    BytecodeInstruction Memory{Opcode::Callm, {addressOperand(EncodedAddressBase::BP, 0)}, 1};
    for (const auto Base : {3, 4, 0xfe})
    {
      Memory.Operands[0].AddressBase = static_cast<EncodedAddressBase>(Base);
      EXPECT_EQ(InstructionCodec::encode(Memory).error(), BytecodeError::InvalidAddressBase);
      const std::vector<std::uint8_t> Bytes{0x3d, 0x00, static_cast<std::uint8_t>(Base), 0, 0, 0, 0, 0, 0, 0, 0};
      EXPECT_EQ(InstructionCodec::decode(Bytes).error(), BytecodeError::InvalidAddressBase);
    }
    BytecodeInstruction Branch{Opcode::Jmp, {addressOperand(EncodedAddressBase::PC, 0)}, 1};
    for (const auto Base : {EncodedAddressBase::BP, EncodedAddressBase::SP, EncodedAddressBase::Native})
    {
      Branch.Operands[0].AddressBase = Base;
      EXPECT_EQ(InstructionCodec::encode(Branch).error(), BytecodeError::InvalidBranchBase);
      const std::vector<std::uint8_t> Bytes{0x2b, 0x00, static_cast<std::uint8_t>(Base), 0, 0, 0, 0, 0, 0, 0, 0};
      EXPECT_EQ(InstructionCodec::decode(Bytes).error(), BytecodeError::InvalidBranchBase);
    }
  }

  // Ordinary narrow immediates retain low bits while native import indices reject u32 overflow.
  TEST(VMInstructionCodecTest, TruncatesNumericBitsButChecksNativeIndexWidth)
  {
    const BytecodeInstruction Store{Opcode::Store8i, {addressOperand(EncodedAddressBase::BP, 16), immediateOperand(std::numeric_limits<std::uint64_t>::max())}, 2};
    const auto Bytes = InstructionCodec::encode(Store);
    ASSERT_TRUE(Bytes);
    EXPECT_EQ(Bytes->size(), 12);
    EXPECT_EQ(Bytes->back(), 0xff);
    const auto Decoded = InstructionCodec::decode(*Bytes);
    ASSERT_TRUE(Decoded);
    EXPECT_EQ(Decoded->Instruction.Operands[1].Bits, 0xff);
    BytecodeInstruction Native{Opcode::CallNativei, {immediateOperand(std::numeric_limits<std::uint32_t>::max())}, 1};
    EXPECT_TRUE(InstructionCodec::encode(Native));
    ++Native.Operands[0].Bits;
    EXPECT_EQ(InstructionCodec::encode(Native).error(), BytecodeError::ImmediateOutOfRange);
  }
} // namespace ink::vm::test
