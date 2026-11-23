#pragma once

#include "machine_module_text_format.h"

#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/instruction/machine_operand.h"
#include "ink/backend/vm/instruction/opcode_info.h"
#include "ink/backend/vm/module/machine_module.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::vm::detail
{
    struct GlobalHeader
    {
            core::Name Name = core::NAME_None;
            std::uint64_t ByteSize = 0;
            std::uint64_t Alignment = 1;
            MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
            bool IsConstant = false;
    };

    // Accumulates and validates global initializer bytes and relocations before committing to a MachineModule.
    class GlobalVariableBuilder final
    {
        public:
            explicit GlobalVariableBuilder(GlobalHeader Header) noexcept;

            ParseStatus appendByte(std::uint8_t Value, TextSpan Span);
            ParseStatus appendBytes(std::string_view Bytes, TextSpan Span);
            ParseStatus appendZeros(std::uint64_t Count, TextSpan Span);
            ParseStatus appendLittleEndian(std::uint64_t Bits, std::size_t ByteWidth, TextSpan Span);
            ParseStatus addRelocation(std::uint64_t ByteOffset, core::Name Symbol, std::int64_t Addend, TextSpan OffsetSpan);
            ParseStatus finish(MachineModule& Module, TextSpan EndSpan) const;

        private:
            struct PendingRelocation
            {
                    MachineGlobalRelocation Entry{};
                    TextSpan OffsetSpan{};
            };

            bool hasSpaceFor(std::uint64_t ByteCount) const noexcept;
            ParseStatus verifyRelocationsAgainstInitialBytes() const;

            GlobalHeader Header;
            std::vector<std::uint8_t> InitialBytes;
            std::vector<PendingRelocation> Relocations;
            std::unordered_set<std::uint64_t> SeenRelocationOffsets;
    };

    struct FunctionHeader
    {
            core::Name Name = core::NAME_None;
            MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
    };

    struct ParsedInstructionOperands
    {
            std::array<MachineOperand, InstructionEncoding::MaxOperands> Operands{};
            std::array<OperandKind, InstructionEncoding::MaxOperands> Kinds{};
            std::size_t Count = 0;
            std::optional<Spanned<std::string_view>> BranchTarget;
    };

    // Creates basic blocks directly in source order and resolves branch targets at .endfunction without placeholder reordering.
    class FunctionBuilder final
    {
        public:
            explicit FunctionBuilder(MachineFunction& Function) noexcept;

            ParseStatus addStackSlot(std::uint64_t ByteSize, std::uint64_t Alignment, TextSpan DirectiveSpan);
            ParseStatus layoutFrame(std::uint64_t BaseOffset, TextSpan BaseSpan);
            ParseResult<MachineBasicBlock*> createBasicBlock(std::string_view Label, core::Name BlockName, TextSpan LabelSpan);
            ParseStatus addInstruction(MachineBasicBlock& Block, const OpcodeInfo& Info, ParsedInstructionOperands&& Parsed, TextSpan MnemonicSpan);
            ParseStatus finish();

        private:
            struct PendingInstruction
            {
                    MachineBasicBlock* Block = nullptr;
                    Opcode Operation = Opcode::Ret;
                    std::uint8_t OperandCount = 0;
                    std::array<MachineOperand, InstructionEncoding::MaxOperands> Operands{};
                    std::optional<Spanned<std::string_view>> BranchTarget;
                    TextSpan MnemonicSpan{};
            };

            MachineFunction& Function;
            // Keys borrow raw identifier slices from the parser's input buffer, which outlives FunctionBuilder.
            std::unordered_map<std::string_view, MachineBasicBlock*> BlocksByLabel;
            std::vector<PendingInstruction> PendingInstructions;
    };
} // namespace ink::vm::detail
