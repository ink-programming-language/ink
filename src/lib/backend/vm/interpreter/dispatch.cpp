#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/core/support/core_define.h"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <tuple>
#include <type_traits>

namespace ink::vm
{
    namespace
    {
        template <typename ValueType, typename OperandType>
        FORCE_INLINE std::uint64_t readIntegerOperand(const Frame& FrameValue, RegisterId Register, std::uint64_t Word) noexcept
        {
            if constexpr (std::is_same_v<OperandType, ImmediateOperand>)
            {
                return Word;
            }
            else
            {
                // When assertions are enabled, dispatch validates operand register IDs before calling the handler.
                std::uint64_t RegisterValue;
                std::memcpy(&RegisterValue, GET_REGISTER(Register, FrameValue), sizeof(RegisterValue));
                const auto ValueWithOffset = RegisterValue + Word;
                if constexpr (std::is_same_v<OperandType, RegisterOperand>)
                {
                    return ValueWithOffset;
                }
                else
                {
                    static_assert(std::is_same_v<OperandType, AddressOperand>);
                    ValueType Value;
                    std::memcpy(&Value, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(ValueWithOffset)), sizeof(Value));
                    return Value;
                }
            }
        }

        template <typename ValueType>
        FORCE_INLINE void writeIntegerMemory(std::uint64_t Address, std::uint64_t Bits) noexcept
        {
            const auto Value = static_cast<ValueType>(Bits);
            std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(Address)), &Value, sizeof(Value));
        }

        template <Opcode Operation, typename ValueType>
        FORCE_INLINE void executeIntegerMemoryTransfer(const Frame& FrameValue, const std::uint64_t* Instruction, RegisterId DestinationRegister, RegisterId SourceRegister) noexcept
        {
            using Traits = OpcodeTraits<Operation>;
            using Operands = typename Traits::OperandTuple;
            static_assert(Traits::Category == InstructionCategory::Load || Traits::Category == InstructionCategory::Store);
            static_assert(std::is_same_v<std::tuple_element_t<0, Operands>, AddressOperand> && Traits::BitWidth == sizeof(ValueType) * 8);
            const auto DestinationAddress = readIntegerOperand<std::uint64_t, RegisterOperand>(FrameValue, DestinationRegister, Instruction[1]);
            const auto Value = readIntegerOperand<ValueType, std::tuple_element_t<1, Operands>>(FrameValue, SourceRegister, Instruction[2]);
            writeIntegerMemory<ValueType>(DestinationAddress, Value);
        }

        template <Opcode Operation, typename ValueType>
        FORCE_INLINE void executeRegisterLoad(Frame& FrameValue, const std::uint64_t* Instruction, RegisterId DestinationRegister, RegisterId SourceRegister) noexcept
        {
            using Traits = OpcodeTraits<Operation>;
            static_assert(Traits::Category == InstructionCategory::Load && Traits::Form == InstructionForm::Rm && Traits::BitWidth == sizeof(ValueType) * 8);
            const auto Address = READ_REGISTER(SourceRegister, FrameValue) + Instruction[2];
            ValueType Value;
            std::memcpy(&Value, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(Address)), sizeof(Value));
            check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);
            if constexpr (std::is_signed_v<ValueType>)
            {
                WRITE_REGISTER(DestinationRegister, FrameValue, static_cast<std::uint64_t>(static_cast<std::int64_t>(Value)));
            }
            else
            {
                WRITE_REGISTER(DestinationRegister, FrameValue, static_cast<std::uint64_t>(Value));
            }
        }

        template <Opcode Operation, typename ValueType>
        FORCE_INLINE void executeIntegerMemoryAdd(const Frame& FrameValue, const std::uint64_t* Instruction, RegisterId DestinationRegister, RegisterId LeftRegister, RegisterId RightRegister) noexcept
        {
            using Traits = OpcodeTraits<Operation>;
            using Operands = typename Traits::OperandTuple;
            static_assert(Traits::Category == InstructionCategory::Arithmetic && Traits::Domain == InstructionDomain::Integer);
            static_assert(std::is_same_v<std::tuple_element_t<0, Operands>, AddressOperand> && Traits::BitWidth == sizeof(ValueType) * 8);
            const auto DestinationAddress = readIntegerOperand<std::uint64_t, RegisterOperand>(FrameValue, DestinationRegister, Instruction[1]);
            auto Result = readIntegerOperand<ValueType, std::tuple_element_t<1, Operands>>(FrameValue, LeftRegister, Instruction[2]);
            Result += readIntegerOperand<ValueType, std::tuple_element_t<2, Operands>>(FrameValue, RightRegister, Instruction[3]);
            writeIntegerMemory<ValueType>(DestinationAddress, Result);
        }
    } // namespace

    bool Interpreter::dispatch()
    {
        static_assert(sizeof(std::uintptr_t) == sizeof(std::uint64_t), "VM address registers require a 64-bit host");
        static_assert(sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53, "VM FP64 instructions require IEEE 754 binary64");
        const auto CodeAddress = reinterpret_cast<std::uintptr_t>(CodeSegment);
        const auto StackAddress = reinterpret_cast<std::uintptr_t>(StackAllocation.get());
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
        // Keep instruction fetch independent of writes through the exposed register storage.
        auto LocalPC = Frame.getPC();
        for (;;)
        {
            // PC operands, native calls and failure paths observe this instruction's start address.
            Frame.setPC(LocalPC);
            const auto InstructionAddress = LocalPC;
#if INK_ENABLE_ASSERTIONS
            check(InstructionAddress >= CodeAddress && InstructionAddress - CodeAddress < CodeSegmentSize);
            const auto InstructionOffset = static_cast<std::size_t>(InstructionAddress - CodeAddress);
            check(InstructionOffset % InstructionEncoding::WordBytes == 0);
            check(CodeSegmentSize - InstructionOffset >= InstructionEncoding::WordBytes);
#endif
            const auto* const Instruction = reinterpret_cast<const std::uint64_t*>(static_cast<std::uintptr_t>(InstructionAddress));
            const auto Header = Instruction[0];
            const auto Operation = InstructionEncoding::decodeOpcode(Header);
#if INK_ENABLE_ASSERTIONS
            const auto EncodedInstructionBytes = InstructionEncoding::byteSize(Operation);
            check(EncodedInstructionBytes != 0);
            check(CodeSegmentSize - InstructionOffset >= EncodedInstructionBytes);
            check((Header & InstructionEncoding::ReservedMask) == 0);
            const bool FloatingPointOperation = InstructionEncoding::isFloatingPoint(Operation);
            for (std::size_t Index = 0; Index < InstructionEncoding::operandCount(Operation); ++Index)
            {
                const auto Register = InstructionEncoding::decodeRegister(Header, Index);
                if (FloatingPointOperation)
                {
                    check(Index == 0 ? isGeneralPurposeRegister(Register) : isFloatingPointSourceRegister(Register));
                    check(Instruction[Index + 1] == 0);
                }
                else
                {
                    check(isAddressRegister(Register));
                }
            }
#endif
            const auto DestinationRegister = InstructionEncoding::decodeRegister(Header, 0);
            const auto LeftRegister = InstructionEncoding::decodeRegister(Header, 1);
            const auto RightRegister = InstructionEncoding::decodeRegister(Header, 2);
            switch (Operation)
            {
#define EXECUTE_FLOATING_BINARY(Kind, Operator)                                                                                       \
    case Opcode::Kind:                                                                                                                \
        {                                                                                                                             \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes; \
            const auto Left = std::bit_cast<double>(READ_REGISTER(LeftRegister, Frame));                                              \
            const auto Right = std::bit_cast<double>(READ_REGISTER(RightRegister, Frame));                                            \
            const auto Result = Left Operator Right;                                                                                  \
            WRITE_REGISTER(DestinationRegister, Frame, std::bit_cast<std::uint64_t>(Result));                                         \
            LocalPC = InstructionAddress + InstructionBytes;                                                                          \
            continue;                                                                                                                 \
        }
                EXECUTE_FLOATING_BINARY(Fadd64rrr, +)
                EXECUTE_FLOATING_BINARY(Fsub64rrr, -)
                EXECUTE_FLOATING_BINARY(Fmul64rrr, *)
                EXECUTE_FLOATING_BINARY(Fdiv64rrr, /)
#undef EXECUTE_FLOATING_BINARY
            case Opcode::Fsqrt64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Fsqrt64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Source = std::bit_cast<double>(READ_REGISTER(LeftRegister, Frame));
                    const auto Result = std::sqrt(Source);
                    WRITE_REGISTER(DestinationRegister, Frame, std::bit_cast<std::uint64_t>(Result));
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Fneg64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Fneg64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Source = READ_REGISTER(LeftRegister, Frame);
                    WRITE_REGISTER(DestinationRegister, Frame, Source ^ (std::uint64_t{1} << 63));
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
#define FLOAT_COMPARE(Kind, Encoding, Spelling, Expression)                                                                           \
    case Opcode::Kind:                                                                                                                \
        {                                                                                                                             \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes; \
            const auto Left = std::bit_cast<double>(READ_REGISTER(LeftRegister, Frame));                                              \
            const auto Right = std::bit_cast<double>(READ_REGISTER(RightRegister, Frame));                                            \
            const bool Unordered = std::isnan(Left) || std::isnan(Right);                                                             \
            WRITE_REGISTER(DestinationRegister, Frame, static_cast<std::uint64_t>(Expression));                                       \
            LocalPC = InstructionAddress + InstructionBytes;                                                                          \
            continue;                                                                                                                 \
        }
#include "ink/backend/vm/instruction/float_compare.def"
#undef FLOAT_COMPARE
            case Opcode::Sitof64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Sitof64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Source = static_cast<std::int64_t>(READ_REGISTER(LeftRegister, Frame));
                    WRITE_REGISTER(DestinationRegister, Frame, std::bit_cast<std::uint64_t>(static_cast<double>(Source)));
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Uitof64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Uitof64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Source = READ_REGISTER(LeftRegister, Frame);
                    WRITE_REGISTER(DestinationRegister, Frame, std::bit_cast<std::uint64_t>(static_cast<double>(Source)));
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::F64tosi64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::F64tosi64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Source = std::bit_cast<double>(READ_REGISTER(LeftRegister, Frame));
                    std::int64_t Result = 0;
                    if (std::isnan(Source))
                    {
                        Result = 0;
                    }
                    else if (Source >= 0x1p63)
                    {
                        Result = std::numeric_limits<std::int64_t>::max();
                    }
                    else if (Source <= -0x1p63)
                    {
                        Result = std::numeric_limits<std::int64_t>::min();
                    }
                    else
                    {
                        Result = static_cast<std::int64_t>(Source);
                    }
                    WRITE_REGISTER(DestinationRegister, Frame, static_cast<std::uint64_t>(Result));
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::F64toui64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::F64toui64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Source = std::bit_cast<double>(READ_REGISTER(LeftRegister, Frame));
                    std::uint64_t Result = 0;
                    if (std::isnan(Source) || Source <= 0.0)
                    {
                        Result = 0;
                    }
                    else if (Source >= 0x1p64)
                    {
                        Result = std::numeric_limits<std::uint64_t>::max();
                    }
                    else
                    {
                        Result = static_cast<std::uint64_t>(Source);
                    }
                    WRITE_REGISTER(DestinationRegister, Frame, Result);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Mov64rr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Mov64rr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Value = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);
                    WRITE_REGISTER(DestinationRegister, Frame, Value);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Mov64ri:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Mov64ri>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);
                    WRITE_REGISTER(DestinationRegister, Frame, Instruction[2]);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
#define EXECUTE_REGISTER_LOAD(Kind, ValueType)                                                                                        \
    case Opcode::Kind:                                                                                                                \
        {                                                                                                                             \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes; \
            executeRegisterLoad<Opcode::Kind, ValueType>(Frame, Instruction, DestinationRegister, LeftRegister);                      \
            LocalPC = InstructionAddress + InstructionBytes;                                                                          \
            continue;                                                                                                                 \
        }
                EXECUTE_REGISTER_LOAD(Load8urm, std::uint8_t)
                EXECUTE_REGISTER_LOAD(Load16urm, std::uint16_t)
                EXECUTE_REGISTER_LOAD(Load32urm, std::uint32_t)
                EXECUTE_REGISTER_LOAD(Load8srm, std::int8_t)
                EXECUTE_REGISTER_LOAD(Load16srm, std::int16_t)
                EXECUTE_REGISTER_LOAD(Load32srm, std::int32_t)
                EXECUTE_REGISTER_LOAD(Load64rm, std::uint64_t)
#undef EXECUTE_REGISTER_LOAD

#define EXECUTE_MEMORY_TRANSFER(Kind, Width)                                                                                          \
    case Opcode::Kind:                                                                                                                \
        {                                                                                                                             \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes; \
            executeIntegerMemoryTransfer<Opcode::Kind, std::uint##Width##_t>(Frame, Instruction, DestinationRegister, LeftRegister);  \
            LocalPC = InstructionAddress + InstructionBytes;                                                                          \
            continue;                                                                                                                 \
        }
                // Load
                EXECUTE_MEMORY_TRANSFER(Load8, 8)
                EXECUTE_MEMORY_TRANSFER(Load16, 16)
                EXECUTE_MEMORY_TRANSFER(Load32, 32)
                EXECUTE_MEMORY_TRANSFER(Load64, 64)

                // Store
                EXECUTE_MEMORY_TRANSFER(Store8m, 8)
                EXECUTE_MEMORY_TRANSFER(Store8i, 8)
                EXECUTE_MEMORY_TRANSFER(Store16m, 16)
                EXECUTE_MEMORY_TRANSFER(Store16i, 16)
                EXECUTE_MEMORY_TRANSFER(Store32m, 32)
                EXECUTE_MEMORY_TRANSFER(Store32i, 32)
                EXECUTE_MEMORY_TRANSFER(Store64m, 64)
                EXECUTE_MEMORY_TRANSFER(Store64i, 64)
                EXECUTE_MEMORY_TRANSFER(Store8mr, 8)
                EXECUTE_MEMORY_TRANSFER(Store16mr, 16)
                EXECUTE_MEMORY_TRANSFER(Store32mr, 32)
                EXECUTE_MEMORY_TRANSFER(Store64mr, 64)
#undef EXECUTE_MEMORY_TRANSFER

            // Add
            case Opcode::Add64rrr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Add64rrr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto Right = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    const auto Result = Left + Right;
                    check(isWritableAddressRegister(DestinationRegister));
                    WRITE_REGISTER(DestinationRegister, Frame, Result);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Add64rrm:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Add64rrm>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto RightAddress = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    std::uint64_t Right;
                    std::memcpy(&Right, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(RightAddress)), sizeof(Right));
                    const auto Result = Left + Right;
                    check(isWritableAddressRegister(DestinationRegister));
                    WRITE_REGISTER(DestinationRegister, Frame, Result);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Add64rri:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Add64rri>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    check(isWritableAddressRegister(DestinationRegister));
                    WRITE_REGISTER(DestinationRegister, Frame, Left + Instruction[3]);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
#define EXECUTE_MEMORY_ADD(Kind, Width)                                                                                                        \
    case Opcode::Kind:                                                                                                                         \
        {                                                                                                                                      \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes;          \
            executeIntegerMemoryAdd<Opcode::Kind, std::uint##Width##_t>(Frame, Instruction, DestinationRegister, LeftRegister, RightRegister); \
            LocalPC = InstructionAddress + InstructionBytes;                                                                                   \
            continue;                                                                                                                          \
        }
                EXECUTE_MEMORY_ADD(Add8mmm, 8)
                EXECUTE_MEMORY_ADD(Add8mmi, 8)
                EXECUTE_MEMORY_ADD(Add8mii, 8)
                EXECUTE_MEMORY_ADD(Add16mmm, 16)
                EXECUTE_MEMORY_ADD(Add16mmi, 16)
                EXECUTE_MEMORY_ADD(Add16mii, 16)
                EXECUTE_MEMORY_ADD(Add32mmm, 32)
                EXECUTE_MEMORY_ADD(Add32mmi, 32)
                EXECUTE_MEMORY_ADD(Add32mii, 32)
                EXECUTE_MEMORY_ADD(Add64mmm, 64)
                EXECUTE_MEMORY_ADD(Add64mmi, 64)
                EXECUTE_MEMORY_ADD(Add64mii, 64)
                EXECUTE_MEMORY_ADD(Add64mrr, 64)
                EXECUTE_MEMORY_ADD(Add64mrm, 64)
                EXECUTE_MEMORY_ADD(Add64mri, 64)
#undef EXECUTE_MEMORY_ADD

#define EXECUTE_REGISTER_BINARY(Kind, Operator)                                                                                       \
    case Opcode::Kind:                                                                                                                \
        {                                                                                                                             \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes; \
            constexpr bool RegisterRHS = OpcodeTraits<Opcode::Kind>::Form == InstructionForm::Rrr;                                    \
            const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];                                                    \
            const auto Right = RegisterRHS ? READ_REGISTER(RightRegister, Frame) + Instruction[3] : Instruction[3];                   \
            check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);                                             \
            WRITE_REGISTER(DestinationRegister, Frame, Left Operator Right);                                                          \
            LocalPC = InstructionAddress + InstructionBytes;                                                                          \
            continue;                                                                                                                 \
        }
                EXECUTE_REGISTER_BINARY(Sub64rrr, -)
                EXECUTE_REGISTER_BINARY(Sub64rri, -)
                EXECUTE_REGISTER_BINARY(Mul64rrr, *)
                EXECUTE_REGISTER_BINARY(Mul64rri, *)
                EXECUTE_REGISTER_BINARY(And64rrr, &)
                EXECUTE_REGISTER_BINARY(And64rri, &)
                EXECUTE_REGISTER_BINARY(Or64rrr, |)
                EXECUTE_REGISTER_BINARY(Or64rri, |)
                EXECUTE_REGISTER_BINARY(Xor64rrr, ^)
                EXECUTE_REGISTER_BINARY(Xor64rri, ^)
#undef EXECUTE_REGISTER_BINARY

            case Opcode::Sdiv64rrr:
            case Opcode::Srem64rrr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Sdiv64rrr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Left = static_cast<std::int64_t>(READ_REGISTER(LeftRegister, Frame) + Instruction[2]);
                    const auto Right = static_cast<std::int64_t>(READ_REGISTER(RightRegister, Frame) + Instruction[3]);
                    if (Right == 0) [[unlikely]]
                    {
                        REPORT_IF(true, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "division", "division by zero");
                        return false;
                    }
                    if (Left == std::numeric_limits<std::int64_t>::min() && Right == -1) [[unlikely]]
                    {
                        REPORT_IF(true, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "division", "signed integer division overflow");
                        return false;
                    }
                    check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);
                    const auto Result = Operation == Opcode::Sdiv64rrr ? Left / Right : Left % Right;
                    WRITE_REGISTER(DestinationRegister, Frame, static_cast<std::uint64_t>(Result));
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }
            case Opcode::Udiv64rrr:
            case Opcode::Urem64rrr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Udiv64rrr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Left = READ_REGISTER(LeftRegister, Frame) + Instruction[2];
                    const auto Right = READ_REGISTER(RightRegister, Frame) + Instruction[3];
                    if (Right == 0) [[unlikely]]
                    {
                        REPORT_IF(true, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "division", "division by zero");
                        return false;
                    }
                    check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);
                    const auto Result = Operation == Opcode::Udiv64rrr ? Left / Right : Left % Right;
                    WRITE_REGISTER(DestinationRegister, Frame, Result);
                    LocalPC = InstructionAddress + InstructionBytes;
                    continue;
                }

#define EXECUTE_REGISTER_SHIFT(Kind, ValueType, Operator)                                                                             \
    case Opcode::Kind:                                                                                                                \
        {                                                                                                                             \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes; \
            constexpr bool RegisterRHS = OpcodeTraits<Opcode::Kind>::Form == InstructionForm::Rrr;                                    \
            const auto Left = static_cast<ValueType>(READ_REGISTER(LeftRegister, Frame) + Instruction[2]);                            \
            const auto Right = RegisterRHS ? READ_REGISTER(RightRegister, Frame) + Instruction[3] : Instruction[3];                   \
            const auto Shift = Right & 63U;                                                                                           \
            check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);                                             \
            WRITE_REGISTER(DestinationRegister, Frame, static_cast<std::uint64_t>(Left Operator Shift));                              \
            LocalPC = InstructionAddress + InstructionBytes;                                                                          \
            continue;                                                                                                                 \
        }
                EXECUTE_REGISTER_SHIFT(Shl64rrr, std::uint64_t, <<)
                EXECUTE_REGISTER_SHIFT(Shl64rri, std::uint64_t, <<)
                EXECUTE_REGISTER_SHIFT(Lshr64rrr, std::uint64_t, >>)
                EXECUTE_REGISTER_SHIFT(Lshr64rri, std::uint64_t, >>)
                EXECUTE_REGISTER_SHIFT(Ashr64rrr, std::int64_t, >>)
                EXECUTE_REGISTER_SHIFT(Ashr64rri, std::int64_t, >>)
#undef EXECUTE_REGISTER_SHIFT

#define INTEGER_COMPARE(Kind, Encoding, Spelling, Form, Operator, Signed)                                                                   \
    case Opcode::Kind:                                                                                                                      \
        {                                                                                                                                   \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes;       \
            using ValueType = std::conditional_t<Signed, std::int64_t, std::uint64_t>;                                                      \
            constexpr bool RegisterRHS = InstructionForm::Form == InstructionForm::Rrr;                                                     \
            const auto Left = static_cast<ValueType>(READ_REGISTER(LeftRegister, Frame) + Instruction[2]);                                  \
            const auto Right = static_cast<ValueType>(RegisterRHS ? READ_REGISTER(RightRegister, Frame) + Instruction[3] : Instruction[3]); \
            check(isWritableAddressRegister(DestinationRegister) && Instruction[1] == 0);                                                   \
            WRITE_REGISTER(DestinationRegister, Frame, static_cast<std::uint64_t>(Left Operator Right));                                    \
            LocalPC = InstructionAddress + InstructionBytes;                                                                                \
            continue;                                                                                                                       \
        }
#include "ink/backend/vm/instruction/integer_compare.def"
#undef INTEGER_COMPARE
            case Opcode::Jmp:
            case Opcode::Jmpr:
                {
                    LocalPC = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    continue;
                }
            case Opcode::Jmpi:
                {
                    LocalPC = InstructionAddress + Instruction[1];
                    continue;
                }
            case Opcode::Jmpm:
                {
                    const auto Address = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    std::uint64_t Target;
                    std::memcpy(&Target, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(Address)), sizeof(Target));
                    LocalPC = Target;
                    continue;
                }
#define EXECUTE_VM_CALL(Target)                                                                                                                                                 \
    do                                                                                                                                                                          \
    {                                                                                                                                                                           \
        const std::uint64_t CallTarget = (Target);                                                                                                                              \
        const auto NextPC = InstructionAddress + InstructionBytes;                                                                                                              \
        const auto StackPointer = READ_REGISTER(RegisterId::SP, Frame);                                                                                                         \
        const auto BasePointer = READ_REGISTER(RegisterId::BP, Frame);                                                                                                          \
        check(IsInstructionAddress(NextPC));                                                                                                                                    \
        check(IsStackPosition(StackPointer) && IsStackPosition(BasePointer) && BasePointer <= StackPointer && (CallDepth == 0 || StackPointer - BasePointer >= SaveAreaBytes)); \
        check(IsInstructionAddress(CallTarget));                                                                                                                                \
        const bool StackOverflow = SaveAreaBytes > StackSize - static_cast<std::size_t>(StackPointer - StackAddress);                                                           \
        if (StackOverflow) [[unlikely]]                                                                                                                                         \
        {                                                                                                                                                                       \
            REPORT_IF(true, Diagnostics, core::DiagnosticKind::VMExecutionFailed, {}, "call", "stack overflow");                                                                \
            return false;                                                                                                                                                       \
        }                                                                                                                                                                       \
        auto* const SaveArea = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(StackPointer));                                                                      \
        std::memcpy(SaveArea, &BasePointer, sizeof(BasePointer));                                                                                                               \
        std::memcpy(SaveArea + InstructionEncoding::WordBytes, &NextPC, sizeof(NextPC));                                                                                        \
        WRITE_REGISTER(RegisterId::BP, Frame, StackPointer);                                                                                                                    \
        WRITE_REGISTER(RegisterId::SP, Frame, StackPointer + SaveAreaBytes);                                                                                                    \
        LocalPC = CallTarget;                                                                                                                                                   \
        ++CallDepth;                                                                                                                                                            \
    } while (false)

#define EXECUTE_NATIVE_CALL(Target)                                                                                                                                             \
    do                                                                                                                                                                          \
    {                                                                                                                                                                           \
        const std::uint64_t CallTarget = (Target);                                                                                                                              \
        const auto NextPC = InstructionAddress + InstructionBytes;                                                                                                              \
        [[maybe_unused]] const auto StackPointer = READ_REGISTER(RegisterId::SP, Frame);                                                                                        \
        [[maybe_unused]] const auto BasePointer = READ_REGISTER(RegisterId::BP, Frame);                                                                                         \
        check(IsInstructionAddress(NextPC));                                                                                                                                    \
        check(IsStackPosition(StackPointer) && IsStackPosition(BasePointer) && BasePointer <= StackPointer && (CallDepth == 0 || StackPointer - BasePointer >= SaveAreaBytes)); \
        check(NativeStubAddresses.contains(CallTarget));                                                                                                                        \
        const auto Stub = reinterpret_cast<void (*)(vm::Frame*)>(static_cast<std::uintptr_t>(CallTarget));                                                                      \
        Stub(&Frame);                                                                                                                                                           \
        LocalPC = NextPC;                                                                                                                                                       \
    } while (false)

            case Opcode::Calli:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Calli>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    EXECUTE_NATIVE_CALL(Instruction[1]);
                    continue;
                }
            case Opcode::Callvmi:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Callvmi>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    EXECUTE_VM_CALL(Instruction[1]);
                    continue;
                }
            case Opcode::Callm:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Callm>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto Address = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    std::uint64_t TargetAddress;
                    std::memcpy(&TargetAddress, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(Address)), sizeof(TargetAddress));
                    if (IsInstructionAddress(TargetAddress))
                    {
                        EXECUTE_VM_CALL(TargetAddress);
                    }
                    else
                    {
                        EXECUTE_NATIVE_CALL(TargetAddress);
                    }
                    continue;
                }
            case Opcode::Callr:
                {
                    constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Callr>::OperandCount + 1) * InstructionEncoding::WordBytes;
                    const auto TargetAddress = READ_REGISTER(DestinationRegister, Frame) + Instruction[1];
                    if (IsInstructionAddress(TargetAddress))
                    {
                        EXECUTE_VM_CALL(TargetAddress);
                    }
                    else
                    {
                        EXECUTE_NATIVE_CALL(TargetAddress);
                    }
                    continue;
                }
#undef EXECUTE_NATIVE_CALL
#undef EXECUTE_VM_CALL
#define COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Width, Operator, Signed)                                                                \
    case Opcode::Kind:                                                                                                                         \
        {                                                                                                                                      \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes;          \
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
            LocalPC = Target;                                                                                                                  \
            continue;                                                                                                                          \
        }
#include "ink/backend/vm/instruction/compare_branch.def"
#undef COMPARE_BRANCH
#define REGISTER_COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Operator, Signed)                                                               \
    case Opcode::Kind:                                                                                                                          \
        {                                                                                                                                       \
            constexpr std::size_t InstructionBytes = (OpcodeTraits<Opcode::Kind>::OperandCount + 1) * InstructionEncoding::WordBytes;           \
            using ValueType = std::conditional_t<Signed, std::int64_t, std::uint64_t>;                                                          \
            constexpr bool RegisterRHS = InstructionForm::Form == InstructionForm::Rrb;                                                         \
            const auto LeftValue = static_cast<ValueType>(READ_REGISTER(DestinationRegister, Frame) + Instruction[1]);                          \
            const auto RightValue = static_cast<ValueType>(RegisterRHS ? READ_REGISTER(LeftRegister, Frame) + Instruction[2] : Instruction[2]); \
            const auto Target = (LeftValue Operator RightValue) ? InstructionAddress + Instruction[3] : InstructionAddress + InstructionBytes;  \
            LocalPC = Target;                                                                                                                   \
            continue;                                                                                                                           \
        }
#include "ink/backend/vm/instruction/register_compare_branch.def"
#undef REGISTER_COMPARE_BRANCH
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
                    LocalPC = SavedPC;
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
