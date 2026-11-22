#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_format.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/name/name_pool.h"

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
            return std::unexpected(detail::makeTextDiagnostic(0, Description));
        }

        // Reserves real identifier names upfront so generated aliases never collide with later explicit names.
        class LabelScope
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

                std::string allocate(core::Name Name, std::string_view AnonymousPrefix)
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
                    std::size_t& NextIndex = NextAnonymousIndex[AnonymousPrefix];
                    for (;;)
                    {
                        std::string Candidate = std::string(AnonymousPrefix) + std::to_string(NextIndex++);
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
                std::unordered_map<std::string_view, std::size_t> NextAnonymousIndex;
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
                    if (const auto Result = writeSymbolDeclarations(Module); !Result)
                    {
                        return std::unexpected(Result.error());
                    }
                    LabelScope ModuleLabels;
                    for (const MachineGlobalVariable& Global : Module.globalVariables())
                    {
                        ModuleLabels.reserve(Global.name());
                    }
                    for (const MachineFunction& Function : Module.functions())
                    {
                        ModuleLabels.reserve(Function.name());
                    }
                    if (const auto Result = writeDataSection(Module, ModuleLabels); !Result)
                    {
                        return std::unexpected(Result.error());
                    }
                    if (const auto Result = writeTextSection(Module, ModuleLabels); !Result)
                    {
                        return std::unexpected(Result.error());
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
                        Text += "\n";
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
                    }
                    return {};
                }

                WriteResult writeDataSection(const MachineModule& Module, LabelScope& ModuleLabels)
                {
                    Text += "\n\n";
                    Text += detail::SectionDirective;
                    Text += ' ';
                    Text += detail::DataSectionName;
                    Text += '\n';
                    for (const MachineGlobalVariable& Global : Module.globalVariables())
                    {
                        if (const auto Result = writeGlobalVariable(Global, ModuleLabels); !Result)
                        {
                            return Result;
                        }
                    }
                    return {};
                }

                WriteResult writeGlobalVariable(const MachineGlobalVariable& Global, LabelScope& ModuleLabels)
                {
                    Text += detail::GlobalDirective;
                    Text += ' ';
                    const std::string Label = ModuleLabels.allocate(Global.name(), ".G");
                    if (const auto Result = writeLabelAndOriginalName(Label, Global.name()); !Result)
                    {
                        return Result;
                    }
                    Text += ", ";
                    writeUnsigned(Global.byteSize());
                    Text += ", ";
                    writeUnsigned(Global.alignment());
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

                WriteResult writeTextSection(const MachineModule& Module, LabelScope& ModuleLabels)
                {
                    Text += '\n';
                    Text += detail::SectionDirective;
                    Text += ' ';
                    Text += detail::TextSectionName;
                    Text += '\n';
                    for (const MachineFunction& Function : Module.functions())
                    {
                        if (const auto Result = writeFunction(Function, ModuleLabels); !Result)
                        {
                            return Result;
                        }
                    }
                    return {};
                }

                WriteResult writeFunction(const MachineFunction& Function, LabelScope& ModuleLabels)
                {
                    Text += detail::FunctionDirective;
                    Text += ' ';
                    const std::string Label = ModuleLabels.allocate(Function.name(), ".F");
                    if (const auto Result = writeLabelAndOriginalName(Label, Function.name()); !Result)
                    {
                        return Result;
                    }
                    if (const auto Result = writeLinkage(Function.linkage(), Function.name()); !Result)
                    {
                        return Result;
                    }
                    Text += '\n';
                    writeFrameInfo(Function.frameInfo());
                    LabelScope BlockScope;
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        BlockScope.reserve(Block.name());
                    }
                    BlockLabelMap BlockLabels;
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        BlockLabels.emplace(&Block, BlockScope.allocate(Block.name(), ".L"));
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
                        writeUnsigned(Slot.byteSize());
                        Text += ", ";
                        writeUnsigned(Slot.alignment());
                        Text += '\n';
                    }
                    if (const auto BaseOffset = Frame.baseOffset())
                    {
                        Text += "    ";
                        Text += detail::FrameDirective;
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
                                Text += "0x";
                                writeUnsigned(*Bits, 16);
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

                WriteResult writeLabelAndOriginalName(std::string_view AllocatedLabel, core::Name Name)
                {
                    Text += AllocatedLabel;
                    return writeOriginalNameIfDifferent(AllocatedLabel, Name);
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

                void writeInitializerBytes(std::span<const std::uint8_t> Bytes)
                {
                    for (std::size_t Index = 0; Index < Bytes.size(); ++Index)
                    {
                        if (Index % 16 == 0)
                        {
                            Text += "    ";
                            Text += detail::ByteDirective;
                            Text += ' ';
                        }
                        else
                        {
                            Text += ", ";
                        }
                        Text += "0x";
                        Text += detail::LowerHexDigits[Bytes[Index] >> 4];
                        Text += detail::LowerHexDigits[Bytes[Index] & 0x0F];
                        if (Index % 16 == 15 || Index + 1 == Bytes.size())
                        {
                            Text += '\n';
                        }
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
