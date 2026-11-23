#include "machine_module_text_builders.h"

#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"

#include <algorithm>
#include <utility>

#define ERROR_IF(Condition, Span, Description) \
    do                                         \
    {                                          \
        if (Condition)                         \
        {                                      \
            return failAt(Span, Description);  \
        }                                      \
    } while (false)

namespace ink::vm::detail
{
    GlobalVariableBuilder::GlobalVariableBuilder(GlobalHeader Header) noexcept
        : Header(Header)
    {
    }

    bool GlobalVariableBuilder::hasSpaceFor(std::uint64_t ByteCount) const noexcept
    {
        return ByteCount <= Header.ByteSize - InitialBytes.size();
    }

    ParseStatus GlobalVariableBuilder::appendByte(std::uint8_t Value, TextSpan Span)
    {
        ERROR_IF(!hasSpaceFor(1), Span, "global initializer exceeds its declared byte size");
        InitialBytes.push_back(Value);
        return {};
    }

    ParseStatus GlobalVariableBuilder::appendBytes(std::string_view Bytes, TextSpan Span)
    {
        ERROR_IF(!hasSpaceFor(Bytes.size()), Span, "global initializer exceeds its declared byte size");
        InitialBytes.reserve(InitialBytes.size() + Bytes.size());
        for (const char Byte : Bytes)
        {
            InitialBytes.push_back(static_cast<std::uint8_t>(Byte));
        }
        return {};
    }

    ParseStatus GlobalVariableBuilder::appendZeros(std::uint64_t Count, TextSpan Span)
    {
        ERROR_IF(!hasSpaceFor(Count), Span, "global initializer exceeds its declared byte size");
        const std::size_t RemainingBudget = MaxMaterializedInitializerBytes - std::min(InitialBytes.size(), MaxMaterializedInitializerBytes);
        ERROR_IF(Count > RemainingBudget || Count > InitialBytes.max_size() - InitialBytes.size(), Span, "global initializer exceeds supported materialized storage limits");
        InitialBytes.insert(InitialBytes.end(), static_cast<std::size_t>(Count), std::uint8_t{0});
        return {};
    }

    ParseStatus GlobalVariableBuilder::appendLittleEndian(std::uint64_t Bits, std::size_t ByteWidth, TextSpan Span)
    {
        ERROR_IF(!hasSpaceFor(ByteWidth), Span, "global initializer exceeds its declared byte size");
        for (std::size_t ByteIndex = 0; ByteIndex < ByteWidth; ++ByteIndex)
        {
            InitialBytes.push_back(static_cast<std::uint8_t>((Bits >> (ByteIndex * 8)) & 0xFFU));
        }
        return {};
    }

    ParseStatus GlobalVariableBuilder::addRelocation(std::uint64_t ByteOffset, core::Name Symbol, std::int64_t Addend, TextSpan OffsetSpan)
    {
        ERROR_IF(Header.Alignment < MachineGlobalRelocation::ByteSize || ByteOffset % MachineGlobalRelocation::ByteSize != 0, OffsetSpan, "global relocations require alignment >= 8 and an eight-byte aligned byte offset");
        ERROR_IF(ByteOffset > Header.ByteSize || MachineGlobalRelocation::ByteSize > Header.ByteSize - ByteOffset, OffsetSpan, "global relocation exceeds its declared byte size");
        ERROR_IF(!SeenRelocationOffsets.insert(ByteOffset).second, OffsetSpan, "global relocation overlaps an earlier relocation");
        Relocations.push_back({{ByteOffset, Symbol, Addend}, OffsetSpan});
        return {};
    }

    ParseStatus GlobalVariableBuilder::verifyRelocationsAgainstInitialBytes() const
    {
        for (const PendingRelocation& Relocation : Relocations)
        {
            const std::uint64_t ByteOffset = Relocation.Entry.ByteOffset;
            if (!std::cmp_less(ByteOffset, InitialBytes.size()))
            {
                continue;
            }
            const auto Start = static_cast<std::size_t>(ByteOffset);
            const std::size_t OverlapCount = std::min<std::size_t>(InitialBytes.size() - Start, static_cast<std::size_t>(MachineGlobalRelocation::ByteSize));
            for (std::size_t Offset = 0; Offset < OverlapCount; ++Offset)
            {
                ERROR_IF(InitialBytes[Start + Offset] != 0, Relocation.OffsetSpan, "global relocation overlaps nonzero initializer bytes");
            }
        }
        return {};
    }

    ParseStatus GlobalVariableBuilder::finish(MachineModule& Module, TextSpan EndSpan) const
    {
        if (auto Status = verifyRelocationsAgainstInitialBytes(); !Status)
        {
            return Status;
        }
        std::vector<MachineGlobalRelocation> Entries;
        Entries.reserve(Relocations.size());
        for (const PendingRelocation& Relocation : Relocations)
        {
            Entries.push_back(Relocation.Entry);
        }
        ERROR_IF(!Module.createGlobalVariable(Header.ByteSize, Header.Alignment, InitialBytes, Header.Name, Header.Linkage, Header.IsConstant, Entries), EndSpan, "invalid global data layout");
        return {};
    }

    FunctionBuilder::FunctionBuilder(MachineFunction& Function) noexcept
        : Function(Function)
    {
    }

    ParseStatus FunctionBuilder::addStackSlot(std::uint64_t ByteSize, std::uint64_t Alignment, TextSpan DirectiveSpan)
    {
        ERROR_IF(!Function.frameInfo().createStackSlot(ByteSize, Alignment), DirectiveSpan, "invalid stack slot size or alignment");
        return {};
    }

    ParseStatus FunctionBuilder::layoutFrame(std::uint64_t BaseOffset, TextSpan BaseSpan)
    {
        ERROR_IF(!Function.frameInfo().layout(BaseOffset), BaseSpan, "frame layout overflows unsigned 64-bit byte range");
        return {};
    }

    ParseResult<MachineBasicBlock*> FunctionBuilder::createBasicBlock(std::string_view Label, core::Name BlockName, TextSpan LabelSpan)
    {
        ERROR_IF(BlocksByLabel.contains(Label), LabelSpan, "duplicate block label in the function");
        MachineBasicBlock& Block = Function.createBasicBlock(BlockName);
        BlocksByLabel.emplace(Label, &Block);
        return &Block;
    }

    ParseStatus FunctionBuilder::addInstruction(MachineBasicBlock& Block, const OpcodeInfo& Info, ParsedInstructionOperands&& Parsed, TextSpan MnemonicSpan)
    {
        ERROR_IF(const auto ValidationError = validateInstructionConstraints(Info, {Parsed.Operands.data(), Parsed.Count}), MnemonicSpan, *ValidationError);
        PendingInstructions.push_back(PendingInstruction{&Block, Info.Operation, static_cast<std::uint8_t>(Parsed.Count), std::move(Parsed.Operands), Parsed.BranchTarget, MnemonicSpan});
        return {};
    }

    ParseStatus FunctionBuilder::finish()
    {
        MachineInstructionBuilder Builder;
        MachineBasicBlock* ActiveBlock = nullptr;
        for (PendingInstruction& Item : PendingInstructions)
        {
            if (Item.BranchTarget)
            {
                const auto Found = BlocksByLabel.find(Item.BranchTarget->Value);
                ERROR_IF(Found == BlocksByLabel.end(), Item.BranchTarget->Span, "unknown basic block label in this function");
                Item.Operands[Item.OperandCount - 1] = MachineOperand(BasicBlockOperand(Found->second));
            }
            if (Item.Block != ActiveBlock)
            {
                Builder.setInsertPoint(*Item.Block);
                ActiveBlock = Item.Block;
            }
            ERROR_IF(!Builder.tryCreate(Item.Operation, {Item.Operands.data(), Item.OperandCount}), Item.MnemonicSpan, "instruction operands cannot be inserted into their function");
        }
        return {};
    }
} // namespace ink::vm::detail

#undef ERROR_IF
