#include "ink/backend/vm/bytecode/instruction_codec.h"

#include "ink/backend/vm/instruction/machine_instructions.h"

#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ink::vm
{
  namespace
  {
    struct InstructionLayout
    {
        std::array<BytecodeOperandKind, 3> Kinds{};
        std::uint8_t OperandCount = 0;
        std::uint32_t BitWidth = 0;
        bool HasDestination = false;
        bool IsBranch = false;
    };

    template <typename OperandType>
    constexpr BytecodeOperandKind wireKind() noexcept
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        return BytecodeOperandKind::Register;
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand> || std::is_same_v<OperandType, BasicBlockOperand>)
      {
        return BytecodeOperandKind::Address;
      }
      else
      {
        static_assert(std::is_same_v<OperandType, ImmediateOperand>);
        return BytecodeOperandKind::Immediate;
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    constexpr InstructionLayout makeLayout(std::index_sequence<Indices...>) noexcept
    {
      using Traits = OpcodeTraits<Operation>;
      static_assert(Traits::OperandCount <= 3);
      return {{wireKind<std::tuple_element_t<Indices, typename Traits::OperandTuple>>()...}, static_cast<std::uint8_t>(Traits::OperandCount), Traits::BitWidth, Traits::HasDestination, Traits::Category == InstructionCategory::Branch};
    }

    std::optional<InstructionLayout> instructionLayout(Opcode Operation) noexcept
    {
      switch (Operation)
      {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) \
  case Opcode::Kind:                                                                                                    \
    return makeLayout<Opcode::Kind>(std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{});
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
      }
      return std::nullopt;
    }

    std::size_t operandSize(BytecodeOperandKind Kind, std::uint32_t BitWidth) noexcept
    {
      switch (Kind)
      {
      case BytecodeOperandKind::Register:
        return 1;
      case BytecodeOperandKind::Address:
        return 9;
      case BytecodeOperandKind::Immediate:
        return BitWidth / 8;
      }
      return 0;
    }

    std::size_t instructionSize(const InstructionLayout &Layout) noexcept
    {
      std::size_t Size = 2;
      for (std::size_t Index = 0; Index < Layout.OperandCount; ++Index)
      {
        Size += operandSize(Layout.Kinds[Index], Layout.BitWidth);
      }
      return Size;
    }

    bool validByteOrder(ByteOrder Order) noexcept
    {
      return Order == ByteOrder::Little || Order == ByteOrder::Big;
    }

    bool validAddressBase(EncodedAddressBase Base) noexcept
    {
      return Base == EncodedAddressBase::BP || Base == EncodedAddressBase::SP || Base == EncodedAddressBase::PC || Base == EncodedAddressBase::Native;
    }

    std::expected<void, BytecodeError> validateOperands(const BytecodeInstruction &Instruction, const InstructionLayout &Layout) noexcept
    {
      if (Instruction.OperandCount != Layout.OperandCount)
      {
        return std::unexpected(BytecodeError::InvalidOperandCount);
      }
      for (std::size_t Index = 0; Index < Layout.OperandCount; ++Index)
      {
        const auto &Operand = Instruction.Operands[Index];
        if (Operand.Kind != Layout.Kinds[Index])
        {
          return std::unexpected(BytecodeError::InvalidOperandKind);
        }
        if (Operand.Kind == BytecodeOperandKind::Register)
        {
          const bool IsDestination = Index == 0 && Layout.HasDestination;
          if (IsDestination ? !isWritableAddressRegister(Operand.Register) : !isArithmeticRegister(Operand.Register))
          {
            return std::unexpected(BytecodeError::InvalidRegister);
          }
        }
        else if (Operand.Kind == BytecodeOperandKind::Address)
        {
          if (!validAddressBase(Operand.AddressBase))
          {
            return std::unexpected(BytecodeError::InvalidAddressBase);
          }
          if (Layout.IsBranch && Operand.AddressBase != EncodedAddressBase::PC)
          {
            return std::unexpected(BytecodeError::InvalidBranchBase);
          }
        }
        else if (Instruction.Operation == Opcode::CallNativei && Operand.Bits > std::numeric_limits<std::uint32_t>::max())
        {
          return std::unexpected(BytecodeError::ImmediateOutOfRange);
        }
      }
      return {};
    }

    void appendInteger(std::vector<std::uint8_t> &Bytes, std::uint64_t Bits, std::size_t ByteCount, ByteOrder Order)
    {
      for (std::size_t Index = 0; Index < ByteCount; ++Index)
      {
        const std::size_t Shift = (Order == ByteOrder::Little ? Index : ByteCount - 1 - Index) * 8;
        Bytes.push_back(static_cast<std::uint8_t>(Bits >> Shift));
      }
    }

    std::vector<std::uint8_t> encodeInstruction(const BytecodeInstruction &Instruction, const InstructionLayout &Layout, ByteOrder Order)
    {
      std::vector<std::uint8_t> Bytes;
      Bytes.reserve(instructionSize(Layout));
      appendInteger(Bytes, static_cast<std::uint16_t>(Instruction.Operation), 2, Order);
      for (std::size_t Index = 0; Index < Layout.OperandCount; ++Index)
      {
        const auto &Operand = Instruction.Operands[Index];
        switch (Layout.Kinds[Index])
        {
        case BytecodeOperandKind::Register:
          Bytes.push_back(static_cast<std::uint8_t>(Operand.Register));
          break;
        case BytecodeOperandKind::Address:
          Bytes.push_back(static_cast<std::uint8_t>(Operand.AddressBase));
          appendInteger(Bytes, Operand.Bits, 8, Order);
          break;
        case BytecodeOperandKind::Immediate:
          appendInteger(Bytes, Operand.Bits, Layout.BitWidth / 8, Order);
          break;
        }
      }
      return Bytes;
    }

    std::uint64_t readInteger(std::span<const std::uint8_t> Bytes, std::size_t &Offset, std::size_t ByteCount, ByteOrder Order) noexcept
    {
      std::uint64_t Bits = 0;
      for (std::size_t Index = 0; Index < ByteCount; ++Index)
      {
        const std::size_t Shift = (Order == ByteOrder::Little ? Index : ByteCount - 1 - Index) * 8;
        Bits |= static_cast<std::uint64_t>(Bytes[Offset++]) << Shift;
      }
      return Bits;
    }
  } // namespace

  std::optional<std::size_t> InstructionCodec::byteSize(Opcode Operation) noexcept
  {
    const auto Layout = instructionLayout(Operation);
    return Layout ? std::optional{instructionSize(*Layout)} : std::nullopt;
  }

  std::expected<std::vector<std::uint8_t>, BytecodeError> InstructionCodec::encode(const BytecodeInstruction &Instruction, ByteOrder Order)
  {
    if (!validByteOrder(Order))
    {
      return std::unexpected(BytecodeError::InvalidByteOrder);
    }
    const auto Layout = instructionLayout(Instruction.Operation);
    if (!Layout)
    {
      return std::unexpected(BytecodeError::UnknownOpcode);
    }
    if (const auto Validation = validateOperands(Instruction, *Layout); !Validation)
    {
      return std::unexpected(Validation.error());
    }
    return encodeInstruction(Instruction, *Layout, Order);
  }

  std::vector<std::uint8_t> InstructionCodec::encodeUnchecked(const BytecodeInstruction &Instruction, ByteOrder Order)
  {
    return encodeInstruction(Instruction, *instructionLayout(Instruction.Operation), Order);
  }

  std::expected<DecodedInstruction, BytecodeError> InstructionCodec::decode(std::span<const std::uint8_t> Bytes, ByteOrder Order) noexcept
  {
    if (!validByteOrder(Order))
    {
      return std::unexpected(BytecodeError::InvalidByteOrder);
    }
    if (Bytes.size() < 2)
    {
      return std::unexpected(BytecodeError::TruncatedInstruction);
    }
    std::size_t Offset = 0;
    const auto Operation = static_cast<Opcode>(readInteger(Bytes, Offset, 2, Order));
    const auto Layout = instructionLayout(Operation);
    if (!Layout)
    {
      return std::unexpected(BytecodeError::UnknownOpcode);
    }
    const std::size_t Size = instructionSize(*Layout);
    if (Bytes.size() < Size)
    {
      return std::unexpected(BytecodeError::TruncatedInstruction);
    }
    DecodedInstruction Result;
    Result.ByteSize = Size;
    Result.Instruction.Operation = Operation;
    Result.Instruction.OperandCount = Layout->OperandCount;
    for (std::size_t Index = 0; Index < Layout->OperandCount; ++Index)
    {
      auto &Operand = Result.Instruction.Operands[Index];
      Operand.Kind = Layout->Kinds[Index];
      switch (Operand.Kind)
      {
      case BytecodeOperandKind::Register:
        Operand.Register = static_cast<RegisterId>(Bytes[Offset++]);
        break;
      case BytecodeOperandKind::Address:
        Operand.AddressBase = static_cast<EncodedAddressBase>(Bytes[Offset++]);
        Operand.Bits = readInteger(Bytes, Offset, 8, Order);
        break;
      case BytecodeOperandKind::Immediate:
        Operand.Bits = readInteger(Bytes, Offset, Layout->BitWidth / 8, Order);
        break;
      }
    }
    if (const auto Validation = validateOperands(Result.Instruction, *Layout); !Validation)
    {
      return std::unexpected(Validation.error());
    }
    return Result;
  }
} // namespace ink::vm
