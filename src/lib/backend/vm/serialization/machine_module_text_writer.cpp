#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_format.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/name/name_pool.h"

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

namespace ink::vm
{
    namespace
    {
        using WriteResult = std::expected<void, core::Diagnostic>;
        using BlockLabelMap = std::unordered_map<const MachineBasicBlock*, std::string>;

        WriteResult writeFailure(std::string_view Description)
        {
            return std::unexpected(detail::makeWriteDiagnostic(Description));
        }

        // Reserves explicit basic-block identifiers upfront so generated aliases never collide with later blocks.
        class BlockLabelScope
        {
            public:
                void reserve(core::Name Name)
                {
                    const std::string_view Preferred = core::NamePool::instance().text(Name);
                    if (detail::isLabelIdentifier(Preferred))
                    {
                        Reserved.emplace(Preferred);
                    }
                }

                std::string allocate(core::Name Name)
                {
                    const std::string_view Preferred = core::NamePool::instance().text(Name);
                    if (detail::isLabelIdentifier(Preferred))
                    {
                        std::string Label(Preferred);
                        if (Used.insert(Label).second)
                        {
                            return Label;
                        }
                        const auto SuffixPosition = NextCollisionSuffix.try_emplace(Label, 1).first;
                        for (;;)
                        {
                            std::string Candidate = Label + "." + std::to_string(SuffixPosition->second++);
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

            private:
                std::unordered_set<std::string> Reserved;
                std::unordered_set<std::string> Used;
                std::unordered_map<std::string, std::size_t> NextCollisionSuffix;
                std::size_t NextAnonymousIndex = 0;
        };

        class ModuleTextWriter
        {
            public:
                std::expected<std::string, core::Diagnostic> writeModule(const MachineModule& Module)
                {
                    Text = std::string(detail::ModuleDirective) + " ";
                    if (const auto Result = writeQuotedName(Module.name()); !Result)
                    {
                        return std::unexpected(Result.error());
                    }
                    Text += '\n';
                    if (const auto Result = writeSymbolDeclarations(Module); !Result)
                    {
                        return std::unexpected(Result.error());
                    }
                    if (Module.globalVariableCount() != 0)
                    {
                        if (const auto Result = writeDataSection(Module); !Result)
                        {
                            return std::unexpected(Result.error());
                        }
                    }
                    if (Module.functionCount() != 0)
                    {
                        if (const auto Result = writeTextSection(Module); !Result)
                        {
                            return std::unexpected(Result.error());
                        }
                    }
                    return std::move(Text);
                }

            private:
                WriteResult writeSymbolDeclarations(const MachineModule& Module)
                {
                    for (const MachineSymbolDeclaration& Declaration : Module.symbolDeclarations())
                    {
                        const auto KindSpelling = detail::symbolKindName(Declaration.Kind);
                        if (!KindSpelling)
                        {
                            return writeFailure("unknown symbol declaration kind");
                        }
                        Text += detail::DeclareDirective;
                        Text += ' ';
                        Text += *KindSpelling;
                        Text += ' ';
                        if (const auto Result = writeSymbol(Declaration.Name); !Result)
                        {
                            return Result;
                        }
                        if (Declaration.IsConstant)
                        {
                            if (Declaration.Kind != MachineSymbolKind::GlobalVariable)
                            {
                                return writeFailure("only global declarations accept the readonly attribute");
                            }
                            Text += ' ';
                            Text += detail::ReadOnlyKeyword;
                        }
                        Text += '\n';
                    }
                    return {};
                }

                WriteResult writeDataSection(const MachineModule& Module)
                {
                    Text += '\n';
                    Text += detail::SectionDirective;
                    Text += ' ';
                    Text += detail::DataSectionName;
                    Text += '\n';
                    for (const MachineGlobalVariable& Global : Module.globalVariables())
                    {
                        if (const auto Result = writeGlobalVariable(Global); !Result)
                        {
                            return Result;
                        }
                    }
                    return {};
                }

                WriteResult writeGlobalVariable(const MachineGlobalVariable& Global)
                {
                    Text += detail::GlobalDirective;
                    if (Global.name().valid())
                    {
                        Text += ' ';
                        if (const auto Result = writeSymbol(Global.name()); !Result)
                        {
                            return Result;
                        }
                    }
                    Text += ' ';
                    Text += detail::SizeKeyword;
                    Text += ' ';
                    writeUnsigned(Global.byteSize());
                    if (Global.alignment() != 1)
                    {
                        Text += ' ';
                        Text += detail::AlignKeyword;
                        Text += ' ';
                        writeUnsigned(Global.alignment());
                    }
                    if (Global.isConstant())
                    {
                        Text += ' ';
                        Text += detail::ReadOnlyKeyword;
                    }
                    if (const auto Result = writeLinkage(Global.linkage(), Global.name()); !Result)
                    {
                        return Result;
                    }
                    Text += '\n';
                    writeInitializerBytes(Global.initialBytes());
                    for (const MachineGlobalRelocation& Relocation : Global.relocations())
                    {
                        if (const auto Result = writeGlobalRelocation(Relocation); !Result)
                        {
                            return Result;
                        }
                    }
                    Text += detail::EndGlobalDirective;
                    Text += '\n';
                    return {};
                }

                WriteResult writeGlobalRelocation(const MachineGlobalRelocation& Relocation)
                {
                    Text += "    ";
                    Text += detail::RelocDirective;
                    Text += ' ';
                    writeUnsigned(Relocation.ByteOffset);
                    Text += ", ";
                    if (const auto Result = writeSymbol(Relocation.Symbol); !Result)
                    {
                        return Result;
                    }
                    writeDisplacement(Relocation.Addend);
                    Text += '\n';
                    return {};
                }

                WriteResult writeTextSection(const MachineModule& Module)
                {
                    Text += '\n';
                    Text += detail::SectionDirective;
                    Text += ' ';
                    Text += detail::TextSectionName;
                    Text += '\n';
                    for (const MachineFunction& Function : Module.functions())
                    {
                        if (const auto Result = writeFunction(Function); !Result)
                        {
                            return Result;
                        }
                    }
                    return {};
                }

                WriteResult writeFunction(const MachineFunction& Function)
                {
                    Text += detail::FunctionDirective;
                    if (Function.name().valid())
                    {
                        Text += ' ';
                        if (const auto Result = writeSymbol(Function.name()); !Result)
                        {
                            return Result;
                        }
                    }
                    if (const auto Result = writeLinkage(Function.linkage(), Function.name()); !Result)
                    {
                        return Result;
                    }
                    Text += '\n';
                    writeFrameInfo(Function.frameInfo());
                    BlockLabelScope BlockScope;
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        BlockScope.reserve(Block.name());
                    }
                    BlockLabelMap BlockLabels;
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        BlockLabels.emplace(&Block, BlockScope.allocate(Block.name()));
                    }
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        const std::string& BlockLabel = BlockLabels.find(&Block)->second;
                        if (const auto Result = writeBasicBlock(Block, BlockLabel, BlockLabels); !Result)
                        {
                            return Result;
                        }
                    }
                    Text += detail::EndFunctionDirective;
                    Text += '\n';
                    return {};
                }

                void writeFrameInfo(const MachineFrameInfo& Frame)
                {
                    for (const MachineStackSlot& Slot : Frame.stackSlots())
                    {
                        Text += "    ";
                        Text += detail::StackSlotDirective;
                        Text += ' ';
                        Text += detail::SizeKeyword;
                        Text += ' ';
                        writeUnsigned(Slot.byteSize());
                        if (Slot.alignment() != 1)
                        {
                            Text += ' ';
                            Text += detail::AlignKeyword;
                            Text += ' ';
                            writeUnsigned(Slot.alignment());
                        }
                        Text += '\n';
                    }
                    if (const auto BaseOffset = Frame.baseOffset())
                    {
                        Text += "    ";
                        Text += detail::FrameDirective;
                        Text += ' ';
                        Text += detail::BaseKeyword;
                        Text += ' ';
                        writeUnsigned(*BaseOffset);
                        Text += '\n';
                    }
                }

                WriteResult writeBasicBlock(const MachineBasicBlock& Block, std::string_view BlockLabel, const BlockLabelMap& BlockLabels)
                {
                    Text += BlockLabel;
                    Text += ':';
                    if (const auto Result = writeOriginalNameIfDifferent(BlockLabel, Block.name()); !Result)
                    {
                        return Result;
                    }
                    Text += '\n';
                    for (const MachineInstruction& Instruction : Block.instructions())
                    {
                        if (const auto Result = writeInstruction(Instruction, BlockLabels); !Result)
                        {
                            return Result;
                        }
                    }
                    return {};
                }

                WriteResult writeInstruction(const MachineInstruction& Instruction, const BlockLabelMap& BlockLabels)
                {
                    if (const auto ValidationError = validateInstructionConstraints(Instruction))
                    {
                        return writeFailure(*ValidationError);
                    }
                    Text += "    ";
                    Text += Instruction.getName();
                    for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
                    {
                        Text += Index == 0 ? " " : ", ";
                        if (const auto Result = writeOperand(*Instruction.getOperand(Index), BlockLabels); !Result)
                        {
                            return Result;
                        }
                    }
                    Text += '\n';
                    return {};
                }

                WriteResult writeOperand(const Operand& Value, const BlockLabelMap& BlockLabels)
                {
                    switch (Value.getKind())
                    {
                    case OperandKind::Register:
                        {
                            const auto& Register = static_cast<const RegisterOperand&>(Value);
                            if (const auto Result = writeRegister(Register.getRegister()); !Result)
                            {
                                return Result;
                            }
                            writeDisplacement(Register.getByteOffset());
                            return {};
                        }
                    case OperandKind::Immediate:
                        {
                            const auto& Immediate = static_cast<const ImmediateOperand&>(Value);
                            if (const auto Bits = Immediate.bits())
                            {
                                writeImmediateBits(*Bits);
                                return {};
                            }
                            if (const auto Result = writeSymbol(*Immediate.getSymbol()); !Result)
                            {
                                return Result;
                            }
                            writeDisplacement(*Immediate.getByteOffset());
                            return {};
                        }
                    case OperandKind::BasicBlock:
                        {
                            const MachineBasicBlock* Target = static_cast<const BasicBlockOperand&>(Value).getBlock();
                            const auto Found = BlockLabels.find(Target);
                            if (Found == BlockLabels.end())
                            {
                                return writeFailure("a branch target must belong to its function");
                            }
                            Text += Found->second;
                            return {};
                        }
                    case OperandKind::Address:
                        {
                            const auto& Address = static_cast<const AddressOperand&>(Value);
                            Text += '[';
                            if (const auto Register = Address.getRegister())
                            {
                                if (const auto Result = writeRegister(*Register); !Result)
                                {
                                    return Result;
                                }
                            }
                            else
                            {
                                if (const auto Result = writeSymbol(*Address.getSymbol()); !Result)
                                {
                                    return Result;
                                }
                            }
                            writeDisplacement(Address.getByteOffset());
                            Text += ']';
                            return {};
                        }
                    }
                    return writeFailure("instruction contains an unknown operand kind");
                }

                WriteResult writeOriginalNameIfDifferent(std::string_view AllocatedLabel, core::Name Name)
                {
                    if (AllocatedLabel == core::NamePool::instance().text(Name))
                    {
                        return {};
                    }
                    Text += ' ';
                    return writeQuotedName(Name);
                }

                WriteResult writeLinkage(MachineSymbolLinkage Linkage, core::Name Name)
                {
                    if (Linkage == MachineSymbolLinkage::Local)
                    {
                        return {};
                    }
                    const core::NamePool& Pool = core::NamePool::instance();
                    if (Linkage != MachineSymbolLinkage::External || !Name.valid() || !Pool.contains(Name) || Pool.text(Name).empty())
                    {
                        return writeFailure("definition linkage must be local or external, and external definitions require a nonempty name");
                    }
                    Text += " external";
                    return {};
                }

                WriteResult writeQuotedName(core::Name Name)
                {
                    const core::NamePool& Pool = core::NamePool::instance();
                    if (Name.valid() && !Pool.contains(Name))
                    {
                        return writeFailure("name is not present in the process name pool");
                    }
                    detail::appendQuotedText(Text, Pool.text(Name));
                    return {};
                }

                WriteResult writeSymbol(core::Name Symbol)
                {
                    const core::NamePool& Pool = core::NamePool::instance();
                    if (!Symbol.valid() || !Pool.contains(Symbol) || Pool.text(Symbol).empty())
                    {
                        return writeFailure("symbol name must be nonempty and present in the process name pool");
                    }
                    Text += '@';
                    const std::string_view Spelling = Pool.text(Symbol);
                    if (detail::isLabelIdentifier(Spelling))
                    {
                        Text += Spelling;
                        return {};
                    }
                    detail::appendQuotedText(Text, Spelling);
                    return {};
                }

                WriteResult writeRegister(RegisterId Register)
                {
                    if (!isArithmeticRegister(Register))
                    {
                        return writeFailure("instruction contains an unknown register ID");
                    }
                    Text += getRegisterName(Register);
                    return {};
                }

                void writeUnsigned(std::uint64_t Value, int Base = 10)
                {
                    char Buffer[32];
                    const auto Result = std::to_chars(Buffer, Buffer + sizeof(Buffer), Value, Base);
                    Text.append(Buffer, Result.ptr);
                }

                void writeImmediateBits(std::uint64_t Bits)
                {
                    const auto Signed = std::bit_cast<std::int64_t>(Bits);
                    if (Signed >= -detail::DecimalImmediateLimit && Signed <= detail::DecimalImmediateLimit)
                    {
                        if (Signed < 0)
                        {
                            Text += '-';
                            writeUnsigned(std::uint64_t{0} - Bits);
                        }
                        else
                        {
                            writeUnsigned(Bits);
                        }
                        return;
                    }
                    Text += "0x";
                    writeUnsigned(Bits, 16);
                }

                void writeDisplacement(std::int64_t Offset)
                {
                    if (Offset > 0)
                    {
                        Text += " + ";
                        writeUnsigned(static_cast<std::uint64_t>(Offset));
                    }
                    else if (Offset < 0)
                    {
                        Text += " - ";
                        writeUnsigned(std::uint64_t{0} - static_cast<std::uint64_t>(Offset));
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

                void writeInitializerBytes(std::span<const std::uint8_t> Bytes)
                {
                    std::size_t Index = 0;
                    while (Index < Bytes.size())
                    {
                        const std::size_t ZeroRun = countLeadingZeros(Bytes, Index);
                        if (ZeroRun >= detail::CanonicalZeroRunMinBytes)
                        {
                            Text += "    ";
                            Text += detail::ZeroDirective;
                            Text += ' ';
                            writeUnsigned(ZeroRun);
                            Text += '\n';
                            Index += ZeroRun;
                            continue;
                        }
                        Text += "    ";
                        Text += detail::ByteDirective;
                        Text += ' ';
                        for (std::size_t Column = 0; Column < 16 && Index < Bytes.size(); ++Column, ++Index)
                        {
                            if (Column != 0 && countLeadingZeros(Bytes, Index) >= detail::CanonicalZeroRunMinBytes)
                            {
                                break;
                            }
                            if (Column != 0)
                            {
                                Text += ", ";
                            }
                            Text += "0x";
                            Text += detail::LowerHexDigits[Bytes[Index] >> 4];
                            Text += detail::LowerHexDigits[Bytes[Index] & 0x0F];
                        }
                        Text += '\n';
                    }
                }

                std::string Text;
        };
    } // namespace

    std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue)
    {
        return ModuleTextWriter{}.writeModule(ModuleValue);
    }
} // namespace ink::vm
