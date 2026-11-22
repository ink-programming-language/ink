#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_format.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/backend/vm/module/machine_module_verifier.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <algorithm>
#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::vm
{
    namespace
    {
        // Assigns unique, collision-free textual labels to all basic blocks in a verified function.
        // Reserves explicit identifier names upfront using stable NamePool views so generated aliases never collide.
        class BlockLabelTable final
        {
            public:
                explicit BlockLabelTable(const MachineFunction& Function)
                {
                    const core::NamePool& Pool = core::NamePool::instance();
                    const std::size_t BlockCount = Function.basicBlockCount();
                    std::unordered_set<std::string_view> Reserved;
                    std::unordered_set<std::string> Used;
                    std::unordered_map<std::string_view, std::size_t> NextCollisionSuffix;
                    Reserved.reserve(BlockCount);
                    Used.reserve(BlockCount);
                    Labels.reserve(BlockCount);

                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        const std::string_view Preferred = Pool.text(Block.name());
                        if (detail::isLabelIdentifier(Preferred))
                        {
                            Reserved.insert(Preferred);
                        }
                    }

                    std::size_t NextAnonymousIndex = 0;
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        const std::string_view Preferred = Pool.text(Block.name());
                        Labels.emplace(&Block, allocateLabel(Preferred, Reserved, Used, NextCollisionSuffix, NextAnonymousIndex));
                    }
                }

                std::string_view labelFor(const MachineBasicBlock* Block) const noexcept
                {
                    const auto Found = Labels.find(Block);
                    return Found == Labels.end() ? std::string_view{} : std::string_view(Found->second);
                }

            private:
                static std::string allocateLabel(std::string_view Preferred, const std::unordered_set<std::string_view>& Reserved, std::unordered_set<std::string>& Used, std::unordered_map<std::string_view, std::size_t>& NextCollisionSuffix, std::size_t& NextAnonymousIndex)
                {
                    if (detail::isLabelIdentifier(Preferred))
                    {
                        std::string Label(Preferred);
                        if (Used.insert(Label).second)
                        {
                            return Label;
                        }
                        std::size_t& NextSuffix = NextCollisionSuffix.try_emplace(Preferred, 1).first->second;
                        for (;;)
                        {
                            std::string Candidate = Label + "." + std::to_string(NextSuffix++);
                            if (!Reserved.contains(Candidate) && Used.insert(Candidate).second)
                            {
                                return Candidate;
                            }
                        }
                    }
                    for (;;)
                    {
                        std::string Candidate = ".L" + std::to_string(NextAnonymousIndex++);
                        if (!Reserved.contains(Candidate) && Used.insert(Candidate).second)
                        {
                            return Candidate;
                        }
                    }
                }

                std::unordered_map<const MachineBasicBlock*, std::string> Labels;
        };

        std::size_t estimateModuleTextSize(const MachineModule& Module) noexcept
        {
            std::size_t Estimate = 32 + Module.symbolDeclarations().size() * 32;
            for (const MachineGlobalVariable& Global : Module.globalVariables())
            {
                Estimate += 64 + Global.relocationCount() * 32 + std::min<std::size_t>(Global.initialBytes().size() * 5, 65536);
            }
            for (const MachineFunction& Function : Module.functions())
            {
                Estimate += 48 + Function.frameInfo().stackSlotCount() * 28;
                for (const MachineBasicBlock& Block : Function.basicBlocks())
                {
                    Estimate += 16 + Block.instructionCount() * 32;
                }
            }
            return Estimate;
        }

        // Pure infallible canonical formatter; callers must validate the MachineModule via checkMachineModule first.
        class ModuleTextPrinter final
        {
            public:
                explicit ModuleTextPrinter(std::string& Output) noexcept
                    : Text(Output),
                      Pool(core::NamePool::instance())
                {
                }

                void printModule(const MachineModule& Module)
                {
                    Text.reserve(Text.size() + estimateModuleTextSize(Module));
                    Text.append(detail::ModuleDirective);
                    Text.push_back(' ');
                    printQuotedName(Module.name());
                    Text.push_back('\n');
                    printSymbolDeclarations(Module);
                    if (Module.globalVariableCount() != 0)
                    {
                        printDataSection(Module);
                    }
                    if (Module.functionCount() != 0)
                    {
                        printTextSection(Module);
                    }
                }

            private:
                void printSymbolDeclarations(const MachineModule& Module)
                {
                    for (const MachineSymbolDeclaration& Declaration : Module.symbolDeclarations())
                    {
                        Text.append(detail::DeclareDirective);
                        Text.push_back(' ');
                        Text.append(detail::symbolKindName(Declaration.Kind));
                        Text.push_back(' ');
                        printSymbol(Declaration.Name);
                        if (Declaration.IsConstant)
                        {
                            Text.push_back(' ');
                            Text.append(detail::ReadOnlyKeyword);
                        }
                        Text.push_back('\n');
                    }
                }

                void printDataSection(const MachineModule& Module)
                {
                    Text.push_back('\n');
                    Text.append(detail::SectionDirective);
                    Text.push_back(' ');
                    Text.append(detail::DataSectionName);
                    Text.push_back('\n');
                    for (const MachineGlobalVariable& Global : Module.globalVariables())
                    {
                        printGlobalVariable(Global);
                    }
                }

                void printGlobalVariable(const MachineGlobalVariable& Global)
                {
                    Text.append(detail::GlobalDirective);
                    if (Global.name().valid())
                    {
                        Text.push_back(' ');
                        printSymbol(Global.name());
                    }
                    Text.push_back(' ');
                    Text.append(detail::SizeKeyword);
                    Text.push_back(' ');
                    printUnsigned(Global.byteSize());
                    if (Global.alignment() != 1)
                    {
                        Text.push_back(' ');
                        Text.append(detail::AlignKeyword);
                        Text.push_back(' ');
                        printUnsigned(Global.alignment());
                    }
                    if (Global.isConstant())
                    {
                        Text.push_back(' ');
                        Text.append(detail::ReadOnlyKeyword);
                    }
                    printLinkage(Global.linkage());
                    Text.push_back('\n');
                    printInitializerBytes(Global.initialBytes());
                    printGlobalRelocations(Global.relocations());
                    Text.append(detail::EndGlobalDirective);
                    Text.push_back('\n');
                }

                void printGlobalRelocations(std::span<const MachineGlobalRelocation> Relocations)
                {
                    if (Relocations.size() <= 1 || std::ranges::is_sorted(Relocations, {}, &MachineGlobalRelocation::ByteOffset))
                    {
                        for (const MachineGlobalRelocation& Relocation : Relocations)
                        {
                            printGlobalRelocation(Relocation);
                        }
                        return;
                    }
                    std::vector<MachineGlobalRelocation> Ordered(Relocations.begin(), Relocations.end());
                    std::ranges::sort(Ordered, {}, &MachineGlobalRelocation::ByteOffset);
                    for (const MachineGlobalRelocation& Relocation : Ordered)
                    {
                        printGlobalRelocation(Relocation);
                    }
                }

                void printGlobalRelocation(const MachineGlobalRelocation& Relocation)
                {
                    Text.append("    ");
                    Text.append(detail::RelocDirective);
                    Text.push_back(' ');
                    printUnsigned(Relocation.ByteOffset);
                    Text.append(", ");
                    printSymbol(Relocation.Symbol);
                    printDisplacement(Relocation.Addend);
                    Text.push_back('\n');
                }

                void printTextSection(const MachineModule& Module)
                {
                    Text.push_back('\n');
                    Text.append(detail::SectionDirective);
                    Text.push_back(' ');
                    Text.append(detail::TextSectionName);
                    Text.push_back('\n');
                    for (const MachineFunction& Function : Module.functions())
                    {
                        printFunction(Function);
                    }
                }

                void printFunction(const MachineFunction& Function)
                {
                    Text.append(detail::FunctionDirective);
                    if (Function.name().valid())
                    {
                        Text.push_back(' ');
                        printSymbol(Function.name());
                    }
                    printLinkage(Function.linkage());
                    Text.push_back('\n');
                    printFrameInfo(Function.frameInfo());
                    const BlockLabelTable BlockLabels(Function);
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        printBasicBlock(Block, BlockLabels.labelFor(&Block), BlockLabels);
                    }
                    Text.append(detail::EndFunctionDirective);
                    Text.push_back('\n');
                }

                void printFrameInfo(const MachineFrameInfo& Frame)
                {
                    for (const MachineStackSlot& Slot : Frame.stackSlots())
                    {
                        Text.append("    ");
                        Text.append(detail::StackSlotDirective);
                        Text.push_back(' ');
                        Text.append(detail::SizeKeyword);
                        Text.push_back(' ');
                        printUnsigned(Slot.byteSize());
                        if (Slot.alignment() != 1)
                        {
                            Text.push_back(' ');
                            Text.append(detail::AlignKeyword);
                            Text.push_back(' ');
                            printUnsigned(Slot.alignment());
                        }
                        Text.push_back('\n');
                    }
                    if (const auto BaseOffset = Frame.baseOffset())
                    {
                        Text.append("    ");
                        Text.append(detail::FrameDirective);
                        Text.push_back(' ');
                        Text.append(detail::BaseKeyword);
                        Text.push_back(' ');
                        printUnsigned(*BaseOffset);
                        Text.push_back('\n');
                    }
                }

                void printBasicBlock(const MachineBasicBlock& Block, std::string_view BlockLabel, const BlockLabelTable& BlockLabels)
                {
                    Text.append(BlockLabel);
                    Text.push_back(':');
                    if (BlockLabel != Pool.text(Block.name()))
                    {
                        Text.push_back(' ');
                        printQuotedName(Block.name());
                    }
                    Text.push_back('\n');
                    for (const MachineInstruction& Instruction : Block.instructions())
                    {
                        printInstruction(Instruction, BlockLabels);
                    }
                }

                void printInstruction(const MachineInstruction& Instruction, const BlockLabelTable& BlockLabels)
                {
                    Text.append("    ");
                    Text.append(Instruction.getName());
                    for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
                    {
                        Text.append(Index == 0 ? " " : ", ");
                        printOperand(*Instruction.getOperand(Index), BlockLabels);
                    }
                    Text.push_back('\n');
                }

                void printOperand(const Operand& Value, const BlockLabelTable& BlockLabels)
                {
                    switch (Value.getKind())
                    {
                    case OperandKind::Register:
                        {
                            const auto& Register = static_cast<const RegisterOperand&>(Value);
                            Text.append(getRegisterName(Register.getRegister()));
                            printDisplacement(Register.getByteOffset());
                            return;
                        }
                    case OperandKind::Immediate:
                        {
                            const auto& Immediate = static_cast<const ImmediateOperand&>(Value);
                            if (const auto Bits = Immediate.bits())
                            {
                                printImmediateBits(*Bits);
                                return;
                            }
                            printSymbol(*Immediate.getSymbol());
                            printDisplacement(*Immediate.getByteOffset());
                            return;
                        }
                    case OperandKind::BasicBlock:
                        {
                            const MachineBasicBlock* Target = static_cast<const BasicBlockOperand&>(Value).getBlock();
                            Text.append(BlockLabels.labelFor(Target));
                            return;
                        }
                    case OperandKind::Address:
                        {
                            const auto& Address = static_cast<const AddressOperand&>(Value);
                            Text.push_back('[');
                            if (const auto Register = Address.getRegister())
                            {
                                Text.append(getRegisterName(*Register));
                            }
                            else
                            {
                                printSymbol(*Address.getSymbol());
                            }
                            printDisplacement(Address.getByteOffset());
                            Text.push_back(']');
                            return;
                        }
                    }
                }

                void printLinkage(MachineSymbolLinkage Linkage)
                {
                    if (Linkage == MachineSymbolLinkage::External)
                    {
                        Text.append(" external");
                    }
                }

                void printQuotedName(core::Name Name)
                {
                    detail::appendQuotedText(Text, Pool.text(Name));
                }

                void printSymbol(core::Name Symbol)
                {
                    Text.push_back('@');
                    const std::string_view Spelling = Pool.text(Symbol);
                    if (detail::isLabelIdentifier(Spelling))
                    {
                        Text.append(Spelling);
                        return;
                    }
                    detail::appendQuotedText(Text, Spelling);
                }

                void printUnsigned(std::uint64_t Value, int Base = 10)
                {
                    char Buffer[32];
                    const auto Result = std::to_chars(Buffer, Buffer + sizeof(Buffer), Value, Base);
                    Text.append(Buffer, Result.ptr);
                }

                void printImmediateBits(std::uint64_t Bits)
                {
                    const auto Signed = std::bit_cast<std::int64_t>(Bits);
                    if (Signed >= -detail::DecimalImmediateLimit && Signed <= detail::DecimalImmediateLimit)
                    {
                        if (Signed < 0)
                        {
                            Text.push_back('-');
                            printUnsigned(std::uint64_t{0} - Bits);
                        }
                        else
                        {
                            printUnsigned(Bits);
                        }
                        return;
                    }
                    Text.append("0x");
                    printUnsigned(Bits, 16);
                }

                void printDisplacement(std::int64_t Offset)
                {
                    if (Offset > 0)
                    {
                        Text.append(" + ");
                        printUnsigned(static_cast<std::uint64_t>(Offset));
                    }
                    else if (Offset < 0)
                    {
                        Text.append(" - ");
                        printUnsigned(std::uint64_t{0} - static_cast<std::uint64_t>(Offset));
                    }
                }

                static std::size_t countLeadingZeros(std::span<const std::uint8_t> Bytes, std::size_t Start) noexcept
                {
                    std::size_t Count = 0;
                    while (Start + Count < Bytes.size() && Bytes[Start + Count] == 0)
                    {
                        ++Count;
                    }
                    return Count;
                }

                static bool hasZeroRunAtLeast(std::span<const std::uint8_t> Bytes, std::size_t Start, std::size_t MinimumLength) noexcept
                {
                    if (Bytes.size() - Start < MinimumLength)
                    {
                        return false;
                    }
                    for (std::size_t Offset = 0; Offset < MinimumLength; ++Offset)
                    {
                        if (Bytes[Start + Offset] != 0)
                        {
                            return false;
                        }
                    }
                    return true;
                }

                void printInitializerBytes(std::span<const std::uint8_t> Bytes)
                {
                    std::size_t Index = 0;
                    while (Index < Bytes.size())
                    {
                        if (hasZeroRunAtLeast(Bytes, Index, detail::CanonicalZeroRunMinBytes))
                        {
                            const std::size_t ZeroRun = countLeadingZeros(Bytes, Index);
                            Text.append("    ");
                            Text.append(detail::ZeroDirective);
                            Text.push_back(' ');
                            printUnsigned(ZeroRun);
                            Text.push_back('\n');
                            Index += ZeroRun;
                            continue;
                        }
                        Text.append("    ");
                        Text.append(detail::ByteDirective);
                        Text.push_back(' ');
                        for (std::size_t Column = 0; Column < 16 && Index < Bytes.size(); ++Column, ++Index)
                        {
                            if (Column != 0 && hasZeroRunAtLeast(Bytes, Index, detail::CanonicalZeroRunMinBytes))
                            {
                                break;
                            }
                            if (Column != 0)
                            {
                                Text.append(", ");
                            }
                            Text.append("0x");
                            Text.push_back(detail::LowerHexDigits[Bytes[Index] >> 4]);
                            Text.push_back(detail::LowerHexDigits[Bytes[Index] & 0x0F]);
                        }
                        Text.push_back('\n');
                    }
                }

                std::string& Text;
                const core::NamePool& Pool;
        };
    } // namespace

    std::expected<void, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue, std::string& Output)
    {
        core::DiagnosticEngine Diagnostics;
        core::CollectingDiagnosticConsumer Collector;
        Diagnostics.addConsumer(Collector);
        if (!verifyMachineModule(ModuleValue, Diagnostics))
        {
            auto Reported = Collector.takeDiagnostics();
            if (!Reported.empty())
            {
                return std::unexpected(std::move(Reported.front()));
            }
            return std::unexpected(core::makeDiagnostic<core::DiagnosticKind::VMModuleInvalid>(core::SourceRange{}, "module", "module verification failed"));
        }
        ModuleTextPrinter(Output).printModule(ModuleValue);
        return {};
    }

    std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue)
    {
        std::string Output;
        if (const auto Result = writeMachineModuleText(ModuleValue, Output); !Result)
        {
            return std::unexpected(Result.error());
        }
        return Output;
    }
} // namespace ink::vm
