#pragma once

#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/core/support/core_define.h"

#include <cstddef>
#include <cstdint>

namespace ink::vm
{
    /**
     * Little-endian, eight-byte-aligned encoding: one header word followed by up to three operand words.
     * Header bits: 0..15 = opcode, 16..23 / 24..31 / 32..39 = operand register IDs, 40..63 = zero.
     * Immediate and unused register fields contain RegisterId::ZERO; slots follow semantic operand order.
     * Operand slot offsets are 8 * (Index + 1) for zero-based Index; total size is 8 * (1 + OperandCount).
     * Slots hold 64-bit offset/address bits or immediates; narrow immediates are zero-extended.
     * PC-relative offsets use the instruction start. Encoded alignment does not constrain data addresses.
     */
    struct InstructionEncoding
    {
            static constexpr std::size_t WordBytes = sizeof(std::uint64_t);
            static constexpr std::size_t MaxOperands = 3;
            static constexpr std::uint64_t ReservedMask = 0xFFFFFF0000000000ULL;

            static FORCE_INLINE constexpr std::size_t operandOffset(std::size_t Index) noexcept
            {
                return (Index + 1) * WordBytes;
            }

            static FORCE_INLINE constexpr std::uint64_t encodeRegister(std::uint64_t Header, std::size_t Index, RegisterId Register) noexcept
            {
                const auto Shift = 16 + Index * 8;
                return (Header & ~(std::uint64_t{0xFF} << Shift)) | (static_cast<std::uint64_t>(Register) << Shift);
            }

            static FORCE_INLINE constexpr std::uint64_t encodeOpcode(Opcode Operation) noexcept
            {
                auto Header = static_cast<std::uint64_t>(Operation);
                for (std::size_t Index = 0; Index < MaxOperands; ++Index)
                {
                    Header = encodeRegister(Header, Index, RegisterId::ZERO);
                }
                return Header;
            }

            static FORCE_INLINE constexpr Opcode decodeOpcode(std::uint64_t Header) noexcept
            {
                return static_cast<Opcode>(Header & 0xFFFF);
            }

            static FORCE_INLINE constexpr RegisterId decodeRegister(std::uint64_t Header, std::size_t Index) noexcept
            {
                return static_cast<RegisterId>((Header >> (16 + Index * 8)) & 0xFF);
            }

            static FORCE_INLINE constexpr std::size_t operandCount(Opcode Operation) noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, ...) \
    case Opcode::Kind:    \
        return OpcodeTraits<Opcode::Kind>::OperandCount;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return MaxOperands + 1;
            }

            // Unknown opcodes have no valid encoding size.
            static FORCE_INLINE constexpr std::size_t byteSize(Opcode Operation) noexcept
            {
                const auto Count = operandCount(Operation);
                return Count <= MaxOperands ? (Count + 1) * WordBytes : 0;
            }

            static FORCE_INLINE constexpr bool isFloatingPoint(Opcode Operation) noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category) \
    case Opcode::Kind:                                                                                       \
        return InstructionCategory::Category == InstructionCategory::FloatingPoint;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return false;
            }
    };
} // namespace ink::vm
