#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/core/config/config_manager.h"
#include "ink/core/support/core_define.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace ink::vm
{
    bool Interpreter::dispatch()
    {
        static_assert(sizeof(std::uintptr_t) == sizeof(std::uint64_t), "VM address registers require a 64-bit host");
        const auto CodeAddress = reinterpret_cast<std::uintptr_t>(CodeSegment);
        const auto StackAddress = reinterpret_cast<std::uintptr_t>(StackAllocation.get());
        const auto MaxRecursionDepth = core::ConfigManager::getSize<core::ConfigKind::VMMaxRecursionDepth>();
        const auto IsInstructionAddress = [this, CodeAddress](std::uint64_t Address)
        {
            return Address >= CodeAddress && Address - CodeAddress < CodeSegmentSize && (Address - CodeAddress) % InstructionEncoding::WordBytes == 0 && CodeSegmentSize - static_cast<std::size_t>(Address - CodeAddress) >= InstructionEncoding::WordBytes;
        };
        [[maybe_unused]] const auto IsStackPosition = [this, StackAddress](std::uint64_t Address)
        {
            return Address >= StackAddress && Address - StackAddress <= StackSize && (Address - StackAddress) % InstructionEncoding::WordBytes == 0;
        };
        constexpr std::size_t SaveAreaBytes = 2 * InstructionEncoding::WordBytes;
        std::size_t CallDepth = 0;
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
            case Opcode::Add64mmi:
                {
                    const auto DestinationAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    const auto LeftAddress = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    std::uint64_t Left;
                    std::memcpy(&Left, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(LeftAddress)), sizeof(Left));
                    const auto Result = Left + Instruction[3];
                    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(DestinationAddress)), &Result, sizeof(Result));
                    WRITE_REGISTER(RegisterId::PC, Frame, InstructionAddress + InstructionBytes);
                    continue;
                }
            case Opcode::Add64mmm:
                {
                    const auto DestinationAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    const auto LeftAddress = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto RightAddress = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    std::uint64_t Left;
                    std::uint64_t Right;
                    std::memcpy(&Left, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(LeftAddress)), sizeof(Left));
                    std::memcpy(&Right, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(RightAddress)), sizeof(Right));
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
#define EXECUTE_CALL(Target)                                                                                                                                                                     \
    do                                                                                                                                                                                           \
    {                                                                                                                                                                                            \
        const std::uint64_t CallTarget = (Target);                                                                                                                                               \
        const auto NextPC = InstructionAddress + InstructionBytes;                                                                                                                               \
        const auto StackPointer = READ_REGISTER(RegisterId::SP, Frame);                                                                                                                          \
        const auto BasePointer = READ_REGISTER(RegisterId::BP, Frame);                                                                                                                           \
        check(IsInstructionAddress(NextPC));                                                                                                                                                     \
        check(IsStackPosition(StackPointer) && IsStackPosition(BasePointer) && BasePointer <= StackPointer && (CallDepth == 0 || StackPointer - BasePointer >= SaveAreaBytes));                  \
        if (IsInstructionAddress(CallTarget))                                                                                                                                                    \
        {                                                                                                                                                                                        \
            const bool RecursionDepthExceeded = CallDepth >= MaxRecursionDepth;                                                                                                                  \
            REPORT_IF(RecursionDepthExceeded, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "call", fmt::format("maximum recursion depth exceeded (limit: {})", MaxRecursionDepth)); \
            if (RecursionDepthExceeded)                                                                                                                                                          \
            {                                                                                                                                                                                    \
                return false;                                                                                                                                                                    \
            }                                                                                                                                                                                    \
            const bool StackOverflow = SaveAreaBytes > StackSize - static_cast<std::size_t>(StackPointer - StackAddress);                                                                        \
            REPORT_IF(StackOverflow, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "call", "stack overflow");                                                                        \
            if (StackOverflow)                                                                                                                                                                   \
            {                                                                                                                                                                                    \
                return false;                                                                                                                                                                    \
            }                                                                                                                                                                                    \
            auto* const SaveArea = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(StackPointer));                                                                                   \
            std::memcpy(SaveArea, &BasePointer, sizeof(BasePointer));                                                                                                                            \
            std::memcpy(SaveArea + InstructionEncoding::WordBytes, &NextPC, sizeof(NextPC));                                                                                                     \
            WRITE_REGISTER(RegisterId::BP, Frame, StackPointer);                                                                                                                                 \
            WRITE_REGISTER(RegisterId::SP, Frame, StackPointer + SaveAreaBytes);                                                                                                                 \
            WRITE_REGISTER(RegisterId::PC, Frame, CallTarget);                                                                                                                                   \
            ++CallDepth;                                                                                                                                                                         \
        }                                                                                                                                                                                        \
        else                                                                                                                                                                                     \
        {                                                                                                                                                                                        \
            check(NativeStubAddresses.contains(CallTarget));                                                                                                                                     \
            const auto Stub = reinterpret_cast<void (*)(vm::Frame*)>(static_cast<std::uintptr_t>(CallTarget));                                                                                   \
            Stub(&Frame);                                                                                                                                                                        \
            WRITE_REGISTER(RegisterId::PC, Frame, NextPC);                                                                                                                                       \
        }                                                                                                                                                                                        \
    } while (false)
            case Opcode::Calli:
                {
                    EXECUTE_CALL(Instruction[1]);
                    continue;
                }
            case Opcode::Callm:
                {
                    const auto Address = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    std::uint64_t TargetAddress;
                    std::memcpy(&TargetAddress, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(Address)), sizeof(TargetAddress));
                    EXECUTE_CALL(TargetAddress);
                    continue;
                }
#undef EXECUTE_CALL
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
                {
                    if (CallDepth == 0)
                    {
                        return true;
                    }
                    [[maybe_unused]] const auto StackPointer = READ_REGISTER(RegisterId::SP, Frame);
                    const auto BasePointer = READ_REGISTER(RegisterId::BP, Frame);
                    check(IsStackPosition(StackPointer) && IsStackPosition(BasePointer) && BasePointer <= StackPointer && StackPointer - BasePointer >= SaveAreaBytes);
                    const auto* const SaveArea = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(BasePointer));
                    std::uint64_t SavedBP;
                    std::uint64_t SavedPC;
                    std::memcpy(&SavedBP, SaveArea, sizeof(SavedBP));
                    std::memcpy(&SavedPC, SaveArea + InstructionEncoding::WordBytes, sizeof(SavedPC));
                    check(IsStackPosition(SavedBP) && SavedBP <= BasePointer && IsInstructionAddress(SavedPC));
                    WRITE_REGISTER(RegisterId::SP, Frame, BasePointer);
                    WRITE_REGISTER(RegisterId::BP, Frame, SavedBP);
                    WRITE_REGISTER(RegisterId::PC, Frame, SavedPC);
                    --CallDepth;
                    continue;
                }
            default:
                REPORT_IF(true, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "instruction", "unsupported instruction opcode");
                return false;
            }
        }
    }
} // namespace ink::vm
