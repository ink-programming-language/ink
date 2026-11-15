#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/instruction/machine_instruction.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace ink::vm
{
  bool Interpreter::dispatch()
  {
    static_assert(sizeof(std::uintptr_t) == sizeof(std::uint64_t), "VM address registers require a 64-bit host");
    const auto Fail = [this](std::string_view Description)
    {
      Diagnostics.report<core::DiagnosticKind::VMExecutionFailed>({}, "instruction", Description);
      return false;
    };
    const auto CodeAddress = reinterpret_cast<std::uintptr_t>(CodeSegment);
    for (;;)
    {
      std::uint64_t InstructionAddress;
      std::memcpy(&InstructionAddress, GET_REGISTER(RegisterId::PC, Frame), sizeof(InstructionAddress));
      if (InstructionAddress < CodeAddress || InstructionAddress - CodeAddress >= CodeSegmentSize)
      {
        return Fail("program counter is outside the loaded code segment or execution reached its end without ret");
      }
      const auto InstructionOffset = static_cast<std::size_t>(InstructionAddress - CodeAddress);
      const auto RemainingBytes = CodeSegmentSize - InstructionOffset;
      if (RemainingBytes < sizeof(std::uint16_t))
      {
        return Fail("truncated instruction opcode");
      }
      const auto *const Instruction = CodeSegment + InstructionOffset;
      std::uint16_t OperationBits;
      std::memcpy(&OperationBits, Instruction, sizeof(OperationBits));
      const auto Operation = static_cast<Opcode>(OperationBits);
      constexpr std::size_t InstructionBytes = sizeof(std::uint16_t) + 3 * (sizeof(std::uint8_t) + sizeof(std::uint64_t));
      std::uint64_t Left;
      std::uint64_t Right;
      std::uint8_t *Destination;
      switch (Operation)
      {
      case Opcode::Add64rrr:
      {
        const auto DestinationRegister = static_cast<RegisterId>(Instruction[2]);
        const auto LeftRegister = static_cast<RegisterId>(Instruction[11]);
        const auto RightRegister = static_cast<RegisterId>(Instruction[20]);
        std::uint64_t LeftOffset;
        std::uint64_t RightOffset;
        std::memcpy(&LeftOffset, Instruction + 12, sizeof(LeftOffset));
        std::memcpy(&RightOffset, Instruction + 21, sizeof(RightOffset));
        std::memcpy(&Left, GET_REGISTER(LeftRegister, Frame), sizeof(Left));
        std::memcpy(&Right, GET_REGISTER(RightRegister, Frame), sizeof(Right));
        Left += LeftOffset;
        Right += RightOffset;
        Destination = SET_REGISTER(DestinationRegister, Frame);
        break;
      }
      case Opcode::Add64rrm:
      {
        const auto DestinationRegister = static_cast<RegisterId>(Instruction[2]);
        const auto LeftRegister = static_cast<RegisterId>(Instruction[11]);
        const auto RightRegister = static_cast<RegisterId>(Instruction[20]);
        std::uint64_t LeftOffset;
        std::uint64_t RightOffset;
        std::uint64_t RightAddress;
        std::memcpy(&LeftOffset, Instruction + 12, sizeof(LeftOffset));
        std::memcpy(&RightOffset, Instruction + 21, sizeof(RightOffset));
        std::memcpy(&Left, GET_REGISTER(LeftRegister, Frame), sizeof(Left));
        std::memcpy(&RightAddress, GET_REGISTER(RightRegister, Frame), sizeof(RightAddress));
        Left += LeftOffset;
        RightAddress += RightOffset;
        std::memcpy(&Right, reinterpret_cast<const void *>(static_cast<std::uintptr_t>(RightAddress)), sizeof(Right));
        Destination = SET_REGISTER(DestinationRegister, Frame);
        break;
      }
      case Opcode::Add64mrr:
      {
        const auto DestinationRegister = static_cast<RegisterId>(Instruction[2]);
        const auto LeftRegister = static_cast<RegisterId>(Instruction[11]);
        const auto RightRegister = static_cast<RegisterId>(Instruction[20]);
        std::uint64_t DestinationOffset;
        std::uint64_t LeftOffset;
        std::uint64_t RightOffset;
        std::uint64_t DestinationAddress;
        std::memcpy(&DestinationOffset, Instruction + 3, sizeof(DestinationOffset));
        std::memcpy(&LeftOffset, Instruction + 12, sizeof(LeftOffset));
        std::memcpy(&RightOffset, Instruction + 21, sizeof(RightOffset));
        std::memcpy(&DestinationAddress, GET_REGISTER(DestinationRegister, Frame), sizeof(DestinationAddress));
        std::memcpy(&Left, GET_REGISTER(LeftRegister, Frame), sizeof(Left));
        std::memcpy(&Right, GET_REGISTER(RightRegister, Frame), sizeof(Right));
        DestinationAddress += DestinationOffset;
        Left += LeftOffset;
        Right += RightOffset;
        Destination = reinterpret_cast<std::uint8_t *>(static_cast<std::uintptr_t>(DestinationAddress));
        break;
      }
      case Opcode::Ret:
        return true;
      default:
        return Fail("unsupported instruction opcode");
      }
      const auto Result = Left + Right;
      vm::EFlags Flags = 0;
      if (Result < Left)
      {
        Flags |= eflagMask(EFlag::CF);
      }
      if (Result == 0)
      {
        Flags |= eflagMask(EFlag::ZF);
      }
      if ((Result >> 63) != 0)
      {
        Flags |= eflagMask(EFlag::SF);
      }
      if (((~(Left ^ Right) & (Left ^ Result)) >> 63) != 0)
      {
        Flags |= eflagMask(EFlag::OF);
      }
      std::memcpy(Destination, &Result, sizeof(Result));
      *reinterpret_cast<vm::EFlags *>(SET_REGISTER(RegisterId::EFLAGS, Frame)) = Flags;
      const auto *NextPC = Instruction + InstructionBytes;
      std::memcpy(SET_REGISTER(RegisterId::PC, Frame), &NextPC, sizeof(NextPC));
    }
  }
} // namespace ink::vm
