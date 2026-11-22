#include "ink/backend/vm/module/machine_module_verifier.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/opcode_info.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace ink::vm
{
    namespace
    {
        class MachineModuleVerifier final
        {
            public:
                explicit MachineModuleVerifier(const core::DiagnosticEngine& Diagnostics) noexcept
                    : Diagnostics(Diagnostics),
                      Pool(core::NamePool::instance())
                {
                }

                bool verify(const MachineModule& ModuleValue)
                {
                    if (!isValidOptionalName(ModuleValue.name()))
                    {
                        report("module", "name is not present in the process name pool");
                    }
                    verifySymbolDeclarations(ModuleValue);
                    for (const MachineGlobalVariable& Global : ModuleValue.globalVariables())
                    {
                        verifyGlobalVariable(ModuleValue, Global);
                    }
                    for (const MachineFunction& Function : ModuleValue.functions())
                    {
                        verifyFunction(ModuleValue, Function);
                    }
                    return Valid;
                }

            private:
                void report(std::string_view Context, std::string_view Description)
                {
                    Diagnostics.report<core::DiagnosticKind::VMModuleInvalid>({}, Context, Description);
                    Valid = false;
                }

                bool isValidOptionalName(core::Name Name) const noexcept
                {
                    return !Name.valid() || Pool.contains(Name);
                }

                bool isValidSymbolName(core::Name Symbol) const noexcept
                {
                    return Symbol.valid() && Pool.contains(Symbol) && !Pool.text(Symbol).empty();
                }

                void verifyLinkage(std::string_view Context, MachineSymbolLinkage Linkage, core::Name Name)
                {
                    if (Linkage == MachineSymbolLinkage::Local)
                    {
                        return;
                    }
                    if (Linkage != MachineSymbolLinkage::External || !isValidSymbolName(Name))
                    {
                        report(Context, "definition linkage must be local or external, and external definitions require a nonempty name");
                    }
                }

                void verifySymbolDeclarations(const MachineModule& ModuleValue)
                {
                    for (const MachineSymbolDeclaration& Declaration : ModuleValue.symbolDeclarations())
                    {
                        if (Declaration.Kind != MachineSymbolKind::Function && Declaration.Kind != MachineSymbolKind::GlobalVariable)
                        {
                            report("symbol declaration", "unknown symbol declaration kind");
                        }
                        if (!isValidSymbolName(Declaration.Name))
                        {
                            report("symbol declaration", "symbol name must be nonempty and present in the process name pool");
                        }
                        if (Declaration.IsConstant && Declaration.Kind != MachineSymbolKind::GlobalVariable)
                        {
                            report("symbol declaration", "only global declarations accept the readonly attribute");
                        }
                    }
                }

                void verifyGlobalVariable(const MachineModule& ModuleValue, const MachineGlobalVariable& Global)
                {
                    if (Global.parent() != &ModuleValue)
                    {
                        report("global variable", "global variable has an invalid parent module");
                    }
                    if (Global.name().valid() && !isValidSymbolName(Global.name()))
                    {
                        report("global variable", "symbol name must be nonempty and present in the process name pool");
                    }
                    verifyLinkage("global variable", Global.linkage(), Global.name());
                    if (!std::has_single_bit(Global.alignment()))
                    {
                        report("global variable", "data alignment must be a nonzero power of two");
                    }
                    if (std::cmp_greater(Global.initialBytes().size(), Global.byteSize()))
                    {
                        report("global variable", "global initializer exceeds its declared byte size");
                    }
                    const auto InitialBytes = Global.initialBytes();
                    const auto Relocations = Global.relocations();
                    std::unordered_set<std::uint64_t> SeenOffsets;
                    if (Relocations.size() > 1)
                    {
                        SeenOffsets.reserve(Relocations.size());
                    }
                    for (const MachineGlobalRelocation& Relocation : Relocations)
                    {
                        if (!isValidSymbolName(Relocation.Symbol))
                        {
                            report("global relocation", "symbol name must be nonempty and present in the process name pool");
                        }
                        if (Global.alignment() < MachineGlobalRelocation::ByteSize || Relocation.ByteOffset % MachineGlobalRelocation::ByteSize != 0)
                        {
                            report("global relocation", "global relocations require alignment >= 8 and an eight-byte aligned byte offset");
                        }
                        if (Relocation.ByteOffset > Global.byteSize() || MachineGlobalRelocation::ByteSize > Global.byteSize() - Relocation.ByteOffset)
                        {
                            report("global relocation", "global relocation exceeds its declared byte size");
                        }
                        if (Relocations.size() > 1 && !SeenOffsets.insert(Relocation.ByteOffset).second)
                        {
                            report("global relocation", "global relocation overlaps an earlier relocation");
                        }
                        if (std::cmp_less(Relocation.ByteOffset, InitialBytes.size()))
                        {
                            const auto Start = static_cast<std::size_t>(Relocation.ByteOffset);
                            const std::size_t OverlapCount = std::min<std::size_t>(InitialBytes.size() - Start, static_cast<std::size_t>(MachineGlobalRelocation::ByteSize));
                            for (std::size_t Index = 0; Index < OverlapCount; ++Index)
                            {
                                if (InitialBytes[Start + Index] != 0)
                                {
                                    report("global relocation", "global relocation overlaps nonzero initializer bytes");
                                    break;
                                }
                            }
                        }
                    }
                }

                void verifyOperand(const Operand& Value, const std::unordered_set<const MachineBasicBlock*>& Blocks)
                {
                    switch (Value.getKind())
                    {
                    case OperandKind::Register:
                        {
                            const auto& Register = static_cast<const RegisterOperand&>(Value);
                            if (!isArithmeticRegister(Register.getRegister()))
                            {
                                report("instruction", "instruction contains an unknown register ID");
                            }
                            return;
                        }
                    case OperandKind::Address:
                        {
                            const auto& Address = static_cast<const AddressOperand&>(Value);
                            if (const auto Register = Address.getRegister())
                            {
                                if (!isAddressRegister(*Register))
                                {
                                    report("instruction", "instruction contains an unknown register ID");
                                }
                                return;
                            }
                            if (!Address.getSymbol() || !isValidSymbolName(*Address.getSymbol()))
                            {
                                report("instruction", "symbol name must be nonempty and present in the process name pool");
                            }
                            return;
                        }
                    case OperandKind::Immediate:
                        {
                            const auto& Immediate = static_cast<const ImmediateOperand&>(Value);
                            if (Immediate.bits())
                            {
                                return;
                            }
                            if (!Immediate.getSymbol() || !isValidSymbolName(*Immediate.getSymbol()))
                            {
                                report("instruction", "symbol name must be nonempty and present in the process name pool");
                            }
                            return;
                        }
                    case OperandKind::BasicBlock:
                        {
                            const MachineBasicBlock* Target = static_cast<const BasicBlockOperand&>(Value).getBlock();
                            if (!Blocks.contains(Target))
                            {
                                report("instruction", "a branch target must belong to its function");
                            }
                            return;
                        }
                    }
                    report("instruction", "instruction contains an unknown operand kind");
                }

                void verifyFunction(const MachineModule& ModuleValue, const MachineFunction& Function)
                {
                    if (Function.parent() != &ModuleValue)
                    {
                        report("function", "function has an invalid parent module");
                    }
                    if (Function.name().valid() && !isValidSymbolName(Function.name()))
                    {
                        report("function", "symbol name must be nonempty and present in the process name pool");
                    }
                    verifyLinkage("function", Function.linkage(), Function.name());
                    for (const MachineStackSlot& Slot : Function.frameInfo().stackSlots())
                    {
                        if (!std::has_single_bit(Slot.alignment()))
                        {
                            report("stack slot", "stack slot alignment must be a nonzero power of two");
                        }
                    }
                    std::unordered_set<const MachineBasicBlock*> Blocks;
                    Blocks.reserve(Function.basicBlockCount());
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        if (Block.parent() != &Function)
                        {
                            report("basic block", "basic block has an invalid parent function");
                        }
                        if (!isValidOptionalName(Block.name()))
                        {
                            report("basic block", "name is not present in the process name pool");
                        }
                        Blocks.insert(&Block);
                    }
                    for (const MachineBasicBlock& Block : Function.basicBlocks())
                    {
                        for (const MachineInstruction& Instruction : Block.instructions())
                        {
                            if (Instruction.parent() != &Block)
                            {
                                report("instruction", "instruction has an invalid parent basic block");
                            }
                            if (!findOpcodeInfo(Instruction.getOpcode()))
                            {
                                report("instruction", "instruction has an unknown opcode");
                                continue;
                            }
                            if (const auto Error = validateInstructionConstraints(Instruction))
                            {
                                report("instruction", *Error);
                            }
                            for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
                            {
                                const Operand* Value = Instruction.getOperand(Index);
                                if (!Value)
                                {
                                    report("instruction", "instruction operand is missing");
                                    continue;
                                }
                                verifyOperand(*Value, Blocks);
                            }
                        }
                    }
                }

                const core::DiagnosticEngine& Diagnostics;
                const core::NamePool& Pool;
                bool Valid = true;
        };
    } // namespace

    bool verifyMachineModule(const MachineModule& ModuleValue, const core::DiagnosticEngine& Diagnostics)
    {
        return MachineModuleVerifier(Diagnostics).verify(ModuleValue);
    }
} // namespace ink::vm
