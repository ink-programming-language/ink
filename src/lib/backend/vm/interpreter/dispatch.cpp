#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/core/support/core_define.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

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
            const auto InstructionAddress = READ_REGISTER(RegisterId::PC, Frame);
            check(InstructionAddress >= CodeAddress && InstructionAddress - CodeAddress < CodeSegmentSize);
            const auto InstructionOffset = static_cast<std::size_t>(InstructionAddress - CodeAddress);
            check(InstructionOffset % InstructionEncoding::WordBytes == 0);
            check(CodeSegmentSize - InstructionOffset >= InstructionEncoding::WordBytes);
            const auto* const Instruction = CodeWords.data() + InstructionOffset / InstructionEncoding::WordBytes;
            const auto Header = Instruction[0];
            const auto Operation = InstructionEncoding::decodeOpcode(Header);
            const auto InstructionBytes = InstructionEncoding::byteSize(Operation);
            check(InstructionBytes != 0);
            check(CodeSegmentSize - InstructionOffset >= InstructionBytes);
            check((Header & InstructionEncoding::ReservedMask) == 0);
            for (std::size_t Index = 0; Index < InstructionEncoding::operandCount(Operation); ++Index)
            {
                check(isAddressRegister(InstructionEncoding::decodeRegister(Header, Index)));
            }
            const auto DestinationRegister = InstructionEncoding::decodeRegister(Header, 0);
            const auto LeftRegister = InstructionEncoding::decodeRegister(Header, 1);
            const auto RightRegister = InstructionEncoding::decodeRegister(Header, 2);
            switch (Operation)
            {
            case Opcode::Add64rrr:
                {
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto Right = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    const auto Result = Left + Right;
                    check(isWritableAddressRegister(DestinationRegister));
                    WRITE_REGISTER(DestinationRegister, Frame, Result);
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Add64rrm:
                {
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto RightAddress = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    std::uint64_t Right;
                    std::memcpy(&Right, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(RightAddress)), sizeof(Right));
                    const auto Result = Left + Right;
                    check(isWritableAddressRegister(DestinationRegister));
                    WRITE_REGISTER(DestinationRegister, Frame, Result);
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Add64rri:
                {
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    check(isWritableAddressRegister(DestinationRegister));
                    WRITE_REGISTER(DestinationRegister, Frame, Left + Instruction[3]);
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Add64mrr:
                {
                    const auto DestinationAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto Right = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    const auto Result = Left + Right;
                    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(DestinationAddress)), &Result, sizeof(Result));
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Jmp:
            case Opcode::Jmpr:
                {
                    WRITE_REGISTER(RegisterId::PC, Frame, READ_REGISTER(DestinationRegister, Frame) + Instruction[1]);
                    continue;
                }
            case Opcode::Load64:
                {
                    const auto DestinationAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    const auto SourceAddress = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    std::uint64_t Value;
                    std::memcpy(&Value, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(SourceAddress)), sizeof(Value));
                    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(DestinationAddress)), &Value, sizeof(Value));
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Store32i:
                {
                    const auto DestinationAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    const auto Value = static_cast<std::uint32_t>(Instruction[2]);
                    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(DestinationAddress)), &Value, sizeof(Value));
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Store64i:
                {
                    const auto DestinationAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    const auto Value = Instruction[2];
                    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(DestinationAddress)), &Value, sizeof(Value));
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Jmpi:
                {
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + Instruction[1]);
                    continue;
                }
            case Opcode::Jmpm:
                {
                    const auto Address = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    std::uint64_t Target;
                    std::memcpy(&Target, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(Address)), sizeof(Target));
                    WRITE_REGISTER(RegisterId::PC, Frame, Target);
                    continue;
                }
#define COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Width, Operator, Signed)                                                                \
    case Opcode::Kind:                                                                                                                         \
        {                                                                                                                                      \
            using ValueType = std::conditional_t<Signed, std::int##Width##_t, std::uint##Width##_t>;                                           \
            using UnsignedValueType = std::uint##Width##_t;                                                                                    \
            constexpr bool MemoryRHS = InstructionForm::Form == InstructionForm::Mmb;                                                          \
            const auto LeftAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];                                               \
            ValueType LeftValue;                                                                                                               \
            ValueType RightValue;                                                                                                              \
            std::memcpy(&LeftValue, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(LeftAddress)), sizeof(LeftValue));               \
            if constexpr (MemoryRHS)                                                                                                           \
            {                                                                                                                                  \
                const auto RightAddress = READ_REGISTER(LeftRegister, Frame) + Instruction[2];                                                 \
                std::memcpy(&RightValue, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(RightAddress)), sizeof(RightValue));        \
            }                                                                                                                                  \
            else                                                                                                                               \
            {                                                                                                                                  \
                RightValue = std::bit_cast<ValueType>(static_cast<UnsignedValueType>(Instruction[2]));                                         \
            }                                                                                                                                  \
            const auto Target = (LeftValue Operator RightValue) ? InstructionAddress + Instruction[3] : InstructionAddress + InstructionBytes; \
            WRITE_REGISTER(RegisterId::PC, Frame, Target);                                                                                     \
            continue;                                                                                                                          \
        }
#include "ink/backend/vm/instruction/compare_branch.def"
#undef COMPARE_BRANCH
            case Opcode::Ret:
                return true;
            default:
                return Fail("unsupported instruction opcode");
            }
        }
    }
} // namespace ink::vm
