#include "InkVMInstructionLowering.h"

#include "InkVMMachineModuleEmitter.h"
#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstr.h>
#include <llvm/CodeGen/MachineOperand.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/TargetRegisterInfo.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/MC/MCInstrDesc.h>

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/support/core_define.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

// Propagates a failed lowering sub-step (returns false; error already recorded on ModuleBuilder).
#define RETURN_IF_FAILED(Expression) \
    do                               \
    {                                \
        if (!(Expression))           \
        {                            \
            return false;            \
        }                            \
    } while (false)

// Records an emission error and returns failure (false) when Condition holds.
#define ERROR_IF(Condition, Message) \
    do                               \
    {                                \
        if (Condition)               \
        {                            \
            return error(Message);   \
        }                            \
    } while (false)

namespace llvm
{
    namespace
    {
        std::string_view toStringView(StringRef Text) noexcept
        {
            return std::string_view(Text.data(), Text.size());
        }

        class FunctionInstructionLowerer final
        {
            public:
                FunctionInstructionLowerer(InkVMMachineModuleBuilder& ModuleBuilder, const TargetRegisterInfo& TRI, const TargetInstrInfo& TII, const DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*>& BlockMap) noexcept
                    : ModuleBuilder(ModuleBuilder),
                      TRI(TRI),
                      TII(TII),
                      BlockMap(BlockMap)
                {
                }

                bool lowerFunction(const MachineFunction& MF)
                {
                    for (const MachineBasicBlock& MBB : MF)
                    {
                        RETURN_IF_FAILED(lowerBasicBlock(MBB));
                    }
                    return true;
                }

            private:
                bool error(std::string Message)
                {
                    ModuleBuilder.fail(std::move(Message));
                    return false;
                }

                bool lowerBasicBlock(const MachineBasicBlock& MBB)
                {
                    ink::vm::MachineBasicBlock* VMBlock = BlockMap.lookup(&MBB);
                    checkMsgf(VMBlock != nullptr, "InkVM MachineBasicBlock missing from function block map");
                    InstBuilder.setInsertPoint(*VMBlock);
                    for (const MachineInstr& MI : MBB)
                    {
                        RETURN_IF_FAILED(lowerInstruction(MI));
                    }
                    return true;
                }

                void collectExplicitOperands(const MachineInstr& MI)
                {
                    ExplicitOperands.clear();
                    for (const MachineOperand& MO : MI.operands())
                    {
                        if ((MO.isReg() && MO.isImplicit()) || MO.isMetadata() || MO.isRegMask())
                        {
                            continue;
                        }
                        ExplicitOperands.push_back(&MO);
                    }
                }

                bool lowerInstruction(const MachineInstr& MI)
                {
                    if (MI.isMetaInstruction())
                    {
                        return true;
                    }
                    const MCInstrDesc& Desc = MI.getDesc();
                    ERROR_IF(Desc.isPseudo() || !ink::backend::llvm_inkvm::hasEncodedVMOpcode(Desc.TSFlags), "InkVM encountered unlowered or non-VM MachineInstr: " + TII.getName(MI.getOpcode()).str());

                    collectExplicitOperands(MI);
                    const auto VMOpcode = static_cast<ink::vm::Opcode>(ink::backend::llvm_inkvm::decodeVMOpcodeFromTSFlags(Desc.TSFlags));
                    return emitInstruction(VMOpcode);
                }

                bool decodePhysicalRegister(const MachineOperand& MO, ink::vm::RegisterId& OutRegister)
                {
                    ERROR_IF(!MO.isReg() || !MO.getReg().isPhysical() || MO.getSubReg() != 0, "InkVM instruction expects a physical register without subregister index");
                    const std::uint16_t Encoding = TRI.getEncodingValue(MO.getReg());
                    ERROR_IF(Encoding > 0xFF, "InkVM register encoding exceeds 8-bit VM register space");
                    const auto Register = static_cast<ink::vm::RegisterId>(Encoding);
                    ERROR_IF(!ink::vm::isArithmeticRegister(Register), "InkVM instruction references an unknown VM register ID");
                    OutRegister = Register;
                    return true;
                }

                bool decodeSymbolOperand(const MachineOperand& MO, ink::core::Name& OutName, std::int64_t& OutOffset)
                {
                    checkMsgf(MO.isGlobal() || MO.isSymbol(), "InkVM symbol operand decoder requires a global or external symbol operand");
                    if (MO.isGlobal())
                    {
                        const GlobalValue* Global = MO.getGlobal();
                        ERROR_IF(Global == nullptr || Global->getName().empty(), "InkVM symbol operand references an anonymous global or function");
                        OutName = ModuleBuilder.internName(toStringView(Global->getName()));
                        OutOffset = MO.getOffset();
                        return true;
                    }
                    const char* SymbolText = MO.getSymbolName();
                    ERROR_IF(SymbolText == nullptr || SymbolText[0] == '\0', "InkVM external symbol operand has an empty name");
                    const ink::core::Name Interned = ModuleBuilder.internName(std::string_view(SymbolText));
                    RETURN_IF_FAILED(ModuleBuilder.recordExternalSymbolDeclaration(Interned));
                    OutName = Interned;
                    OutOffset = MO.getOffset();
                    return true;
                }

                bool decodeRegisterOperand(std::size_t& Cursor, std::optional<ink::vm::RegisterOperand>& OutOperand)
                {
                    const MachineOperand& MO = *ExplicitOperands[Cursor++];
                    ink::vm::RegisterId Register = ink::vm::RegisterId::BP;
                    RETURN_IF_FAILED(decodePhysicalRegister(MO, Register));
                    OutOperand.emplace(Register, 0);
                    return true;
                }

                bool decodeAddressOperand(std::size_t& Cursor, std::optional<ink::vm::AddressOperand>& OutOperand)
                {
                    const MachineOperand& First = *ExplicitOperands[Cursor];
                    if (First.isReg())
                    {
                        ERROR_IF(Cursor + 1 >= ExplicitOperands.size() || !ExplicitOperands[Cursor + 1]->isImm(), "InkVM register-relative memory operand requires an immediate displacement");
                        ink::vm::RegisterId BaseRegister = ink::vm::RegisterId::BP;
                        RETURN_IF_FAILED(decodePhysicalRegister(First, BaseRegister));
                        ERROR_IF(!ink::vm::isAddressRegister(BaseRegister), "InkVM memory operand uses an invalid address base register");
                        const std::int64_t Displacement = ExplicitOperands[Cursor + 1]->getImm();
                        Cursor += 2;
                        OutOperand.emplace(BaseRegister, Displacement);
                        return true;
                    }
                    if (First.isGlobal() || First.isSymbol())
                    {
                        ink::core::Name SymbolName;
                        std::int64_t Offset = 0;
                        RETURN_IF_FAILED(decodeSymbolOperand(First, SymbolName, Offset));
                        ++Cursor;
                        OutOperand.emplace(SymbolName, Offset);
                        return true;
                    }
                    return error("InkVM memory operand must be register-relative or symbol-relative");
                }

                bool decodeImmediateOperand(std::size_t& Cursor, std::uint32_t BitWidth, std::optional<ink::vm::ImmediateOperand>& OutOperand)
                {
                    const MachineOperand& MO = *ExplicitOperands[Cursor];
                    if (MO.isImm())
                    {
                        ++Cursor;
                        OutOperand.emplace(static_cast<std::uint64_t>(MO.getImm()));
                        return true;
                    }
                    if (MO.isGlobal() || MO.isSymbol())
                    {
                        ERROR_IF(BitWidth != 64, "InkVM symbol immediate requires a 64-bit immediate field");
                        ink::core::Name SymbolName;
                        std::int64_t Offset = 0;
                        RETURN_IF_FAILED(decodeSymbolOperand(MO, SymbolName, Offset));
                        ++Cursor;
                        OutOperand.emplace(SymbolName, Offset);
                        return true;
                    }
                    return error("InkVM immediate operand must be a numeric immediate or symbol address");
                }

                bool decodeBasicBlockOperand(std::size_t& Cursor, std::optional<ink::vm::BasicBlockOperand>& OutOperand)
                {
                    const MachineOperand& MO = *ExplicitOperands[Cursor++];
                    ERROR_IF(!MO.isMBB() || MO.getMBB() == nullptr, "InkVM branch target operand must be a MachineBasicBlock");
                    const auto Found = BlockMap.find(MO.getMBB());
                    ERROR_IF(Found == BlockMap.end() || Found->second == nullptr, "InkVM branch target MachineBasicBlock does not belong to its function");
                    OutOperand.emplace(Found->second);
                    return true;
                }

                template <typename OperandType, std::uint32_t BitWidth>
                bool decodeOperand(std::size_t& Cursor, std::optional<OperandType>& OutOperand)
                {
                    ERROR_IF(Cursor >= ExplicitOperands.size(), "InkVM instruction is missing required operands");

                    if constexpr (std::is_same_v<OperandType, ink::vm::RegisterOperand>)
                    {
                        return decodeRegisterOperand(Cursor, OutOperand);
                    }
                    else if constexpr (std::is_same_v<OperandType, ink::vm::AddressOperand>)
                    {
                        return decodeAddressOperand(Cursor, OutOperand);
                    }
                    else if constexpr (std::is_same_v<OperandType, ink::vm::ImmediateOperand>)
                    {
                        return decodeImmediateOperand(Cursor, BitWidth, OutOperand);
                    }
                    else
                    {
                        static_assert(std::is_same_v<OperandType, ink::vm::BasicBlockOperand>);
                        return decodeBasicBlockOperand(Cursor, OutOperand);
                    }
                }

                template <ink::vm::Opcode Operation, std::size_t... Indices>
                bool emitOpcodeWithIndices(std::index_sequence<Indices...>)
                {
                    using Traits = ink::vm::OpcodeTraits<Operation>;
                    using Tuple = typename Traits::OperandTuple;

                    std::size_t Cursor = 0;
                    std::tuple<std::optional<std::tuple_element_t<Indices, Tuple>>...> DecodedOperands;
                    const bool DecodedAll = (decodeOperand<std::tuple_element_t<Indices, Tuple>, Traits::BitWidth>(Cursor, std::get<Indices>(DecodedOperands)) && ...);
                    RETURN_IF_FAILED(DecodedAll);
                    ERROR_IF(Cursor != ExplicitOperands.size(), "InkVM instruction has unexpected trailing operands");

                    ink::vm::MachineInstruction* Created = InstBuilder.tryCreate<Operation>(*std::get<Indices>(DecodedOperands)...);
                    ERROR_IF(Created == nullptr, "InkVM MachineInstructionBuilder rejected lowered operands");
                    if (const auto ValidationError = ink::vm::validateInstructionConstraints(*Created))
                    {
                        return error(std::string(*ValidationError));
                    }
                    return true;
                }

                template <ink::vm::Opcode Operation>
                bool emitOpcode()
                {
                    return emitOpcodeWithIndices<Operation>(std::make_index_sequence<ink::vm::OpcodeTraits<Operation>::OperandCount>{});
                }

                bool emitInstruction(ink::vm::Opcode VMOpcode)
                {
                    switch (VMOpcode)
                    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case ink::vm::Opcode::Kind:                                                                                      \
        return emitOpcode<ink::vm::Opcode::Kind>();
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                    default:
                        return error("InkVM instruction carries an unknown VM opcode encoding");
                    }
                }

                InkVMMachineModuleBuilder& ModuleBuilder;
                const TargetRegisterInfo& TRI;
                const TargetInstrInfo& TII;
                const DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*>& BlockMap;
                ink::vm::MachineInstructionBuilder InstBuilder;
                SmallVector<const MachineOperand*, 4> ExplicitOperands;
        };
    } // namespace

    bool lowerInkVMFunctionInstructions(const MachineFunction& MF, const DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*>& BlockMap, InkVMMachineModuleBuilder& ModuleBuilder)
    {
        const TargetSubtargetInfo& STI = MF.getSubtarget();
        const TargetRegisterInfo* TRI = STI.getRegisterInfo();
        const TargetInstrInfo* TII = STI.getInstrInfo();
        if (TRI == nullptr || TII == nullptr)
        {
            ModuleBuilder.fail("InkVM subtarget is missing register or instruction info");
            return false;
        }
        FunctionInstructionLowerer Lowerer(ModuleBuilder, *TRI, *TII, BlockMap);
        return Lowerer.lowerFunction(MF);
    }
} // namespace llvm

#undef ERROR_IF
#undef RETURN_IF_FAILED
