#include "InkVMMachineModuleEmitter.h"

#include "InkVM.h"
#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineInstr.h>
#include <llvm/CodeGen/MachineOperand.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/TargetRegisterInfo.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/MCInstrDesc.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/name/name_pool.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#define DEBUG_TYPE "inkvm-machine-module-emitter"
#define PASS_NAME "InkVM MachineModule Emitter"

using namespace llvm;

namespace
{
    bool isMetadataGlobal(const GlobalVariable& Global)
    {
        return Global.getSection() == "llvm.metadata" || Global.getName().starts_with("llvm.");
    }

    std::string_view toStringView(StringRef Text) noexcept
    {
        return std::string_view(Text.data(), Text.size());
    }

    void appendAPIntLittleEndian(const APInt& Value, std::size_t ByteCount, std::vector<std::uint8_t>& OutBytes)
    {
        const unsigned BitWidth = Value.getBitWidth();
        for (std::size_t ByteIndex = 0; ByteIndex < ByteCount; ++ByteIndex)
        {
            const unsigned BitOffset = static_cast<unsigned>(ByteIndex * 8);
            if (BitOffset >= BitWidth)
            {
                OutBytes.push_back(0);
                continue;
            }
            const unsigned RemainingBits = std::min<unsigned>(8, BitWidth - BitOffset);
            const std::uint64_t ByteBits = Value.extractBitsAsZExtValue(RemainingBits, BitOffset);
            OutBytes.push_back(static_cast<std::uint8_t>(ByteBits));
        }
    }

    std::expected<void, std::string> serializeConstantBytes(const Constant* ConstVal, const DataLayout& DL, std::vector<std::uint8_t>& OutBytes)
    {
        if (ConstVal == nullptr || !ConstVal->getType()->isSized())
        {
            return std::unexpected("InkVM global initializer must have a sized type");
        }
        const TypeSize RawStoreSize = DL.getTypeStoreSize(ConstVal->getType());
        if (RawStoreSize.isScalable())
        {
            return std::unexpected("InkVM global initializer does not support scalable types");
        }
        const std::size_t StoreSize = static_cast<std::size_t>(RawStoreSize.getFixedValue());

        if (isa<ConstantAggregateZero>(ConstVal) || isa<UndefValue>(ConstVal) || isa<ConstantPointerNull>(ConstVal))
        {
            OutBytes.insert(OutBytes.end(), StoreSize, 0);
            return {};
        }

        if (const auto* IntConst = dyn_cast<ConstantInt>(ConstVal))
        {
            appendAPIntLittleEndian(IntConst->getValue(), StoreSize, OutBytes);
            return {};
        }

        if (const auto* FPConst = dyn_cast<ConstantFP>(ConstVal))
        {
            appendAPIntLittleEndian(FPConst->getValueAPF().bitcastToAPInt(), StoreSize, OutBytes);
            return {};
        }

        if (const auto* DataSeq = dyn_cast<ConstantDataSequential>(ConstVal))
        {
            if (DataSeq->getElementByteSize() == 1)
            {
                const StringRef Raw = DataSeq->getRawDataValues();
                OutBytes.insert(OutBytes.end(), Raw.bytes_begin(), Raw.bytes_end());
                return {};
            }
            for (unsigned Index = 0; Index < DataSeq->getNumElements(); ++Index)
            {
                if (auto Result = serializeConstantBytes(DataSeq->getElementAsConstant(Index), DL, OutBytes); !Result)
                {
                    return Result;
                }
            }
            return {};
        }

        if (const auto* ArrayConst = dyn_cast<ConstantArray>(ConstVal))
        {
            for (unsigned Index = 0; Index < ArrayConst->getNumOperands(); ++Index)
            {
                if (auto Result = serializeConstantBytes(ArrayConst->getOperand(Index), DL, OutBytes); !Result)
                {
                    return Result;
                }
            }
            return {};
        }

        if (const auto* VectorConst = dyn_cast<ConstantVector>(ConstVal))
        {
            for (unsigned Index = 0; Index < VectorConst->getNumOperands(); ++Index)
            {
                if (auto Result = serializeConstantBytes(VectorConst->getOperand(Index), DL, OutBytes); !Result)
                {
                    return Result;
                }
            }
            return {};
        }

        if (const auto* StructConst = dyn_cast<ConstantStruct>(ConstVal))
        {
            const StructLayout* Layout = DL.getStructLayout(StructConst->getType());
            const std::size_t BaseOffset = OutBytes.size();
            for (unsigned FieldIndex = 0; FieldIndex < StructConst->getNumOperands(); ++FieldIndex)
            {
                const std::size_t FieldOffset = BaseOffset + static_cast<std::size_t>(Layout->getElementOffset(FieldIndex));
                if (OutBytes.size() < FieldOffset)
                {
                    OutBytes.resize(FieldOffset, 0);
                }
                if (auto Result = serializeConstantBytes(StructConst->getOperand(FieldIndex), DL, OutBytes); !Result)
                {
                    return Result;
                }
            }
            const std::size_t EndOffset = BaseOffset + static_cast<std::size_t>(Layout->getSizeInBytes());
            if (OutBytes.size() < EndOffset)
            {
                OutBytes.resize(EndOffset, 0);
            }
            return {};
        }

        return std::unexpected("InkVM global initializer kind is not supported yet (symbol relocations in .data are not supported yet)");
    }

    std::string_view formatGlobalDataError(ink::vm::MachineModuleDataError Error) noexcept
    {
        switch (Error)
        {
        case ink::vm::MachineModuleDataError::InvalidAlignment:
            return "InkVM global variable alignment must be a nonzero power of two";
        case ink::vm::MachineModuleDataError::InitializerTooLarge:
            return "InkVM global variable initializer exceeds its allocated byte size";
        case ink::vm::MachineModuleDataError::SizeOverflow:
            return "InkVM global variable initializer exceeds host storage limits";
        }
        return "InkVM global variable layout is invalid";
    }

    struct InstructionLoweringContext
    {
            InkVMMachineModuleBuilder& ModuleBuilder;
            const TargetRegisterInfo& TRI;
            const DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*>& BlockMap;
            const SmallVectorImpl<const MachineOperand*>& ExplicitOperands;
    };

    bool decodePhysicalRegister(const MachineOperand& MO, const TargetRegisterInfo& TRI, InkVMMachineModuleBuilder& ModuleBuilder, ink::vm::RegisterId& OutRegister)
    {
        if (!MO.isReg() || !MO.getReg().isPhysical() || MO.getSubReg() != 0)
        {
            ModuleBuilder.fail("InkVM instruction expects a physical register without subregister index");
            return false;
        }
        const std::uint16_t Encoding = TRI.getEncodingValue(MO.getReg());
        if (Encoding > 0xFF)
        {
            ModuleBuilder.fail("InkVM register encoding exceeds 8-bit VM register space");
            return false;
        }
        const auto Register = static_cast<ink::vm::RegisterId>(Encoding);
        if (!ink::vm::isArithmeticRegister(Register))
        {
            ModuleBuilder.fail("InkVM instruction references an unknown VM register ID");
            return false;
        }
        OutRegister = Register;
        return true;
    }

    bool decodeSymbolOperand(const MachineOperand& MO, InkVMMachineModuleBuilder& ModuleBuilder, ink::core::Name& OutName, std::int64_t& OutOffset)
    {
        if (MO.isGlobal())
        {
            const GlobalValue* Global = MO.getGlobal();
            if (Global == nullptr || Global->getName().empty())
            {
                ModuleBuilder.fail("InkVM symbol operand references an anonymous global or function");
                return false;
            }
            const auto Interned = ModuleBuilder.internName(toStringView(Global->getName()));
            if (!Interned)
            {
                ModuleBuilder.fail(Interned.error());
                return false;
            }
            OutName = *Interned;
            OutOffset = MO.getOffset();
            return true;
        }
        if (MO.isSymbol())
        {
            const char* SymbolText = MO.getSymbolName();
            if (SymbolText == nullptr || SymbolText[0] == '\0')
            {
                ModuleBuilder.fail("InkVM external symbol operand has an empty name");
                return false;
            }
            const auto Interned = ModuleBuilder.internName(std::string_view(SymbolText));
            if (!Interned)
            {
                ModuleBuilder.fail(Interned.error());
                return false;
            }
            if (!ModuleBuilder.recordExternalSymbolDeclaration(*Interned))
            {
                return false;
            }
            OutName = *Interned;
            OutOffset = MO.getOffset();
            return true;
        }
        return false;
    }

    template <typename OperandType, std::uint32_t BitWidth>
    bool decodeVMOperand(const InstructionLoweringContext& Context, std::size_t& Cursor, std::optional<OperandType>& OutOperand)
    {
        if (Cursor >= Context.ExplicitOperands.size())
        {
            Context.ModuleBuilder.fail("InkVM instruction is missing required operands");
            return false;
        }

        if constexpr (std::is_same_v<OperandType, ink::vm::RegisterOperand>)
        {
            const MachineOperand& MO = *Context.ExplicitOperands[Cursor++];
            ink::vm::RegisterId Register = ink::vm::RegisterId::BP;
            if (!decodePhysicalRegister(MO, Context.TRI, Context.ModuleBuilder, Register))
            {
                return false;
            }
            OutOperand.emplace(Register, 0);
            return true;
        }
        else if constexpr (std::is_same_v<OperandType, ink::vm::AddressOperand>)
        {
            const MachineOperand& First = *Context.ExplicitOperands[Cursor];
            if (First.isReg())
            {
                if (Cursor + 1 >= Context.ExplicitOperands.size() || !Context.ExplicitOperands[Cursor + 1]->isImm())
                {
                    Context.ModuleBuilder.fail("InkVM register-relative memory operand requires an immediate displacement");
                    return false;
                }
                ink::vm::RegisterId BaseRegister = ink::vm::RegisterId::BP;
                if (!decodePhysicalRegister(First, Context.TRI, Context.ModuleBuilder, BaseRegister))
                {
                    return false;
                }
                if (!ink::vm::isAddressRegister(BaseRegister))
                {
                    Context.ModuleBuilder.fail("InkVM memory operand uses an invalid address base register");
                    return false;
                }
                const std::int64_t Displacement = Context.ExplicitOperands[Cursor + 1]->getImm();
                Cursor += 2;
                OutOperand.emplace(BaseRegister, Displacement);
                return true;
            }
            if (First.isGlobal() || First.isSymbol())
            {
                ink::core::Name SymbolName;
                std::int64_t Offset = 0;
                if (!decodeSymbolOperand(First, Context.ModuleBuilder, SymbolName, Offset))
                {
                    return false;
                }
                ++Cursor;
                OutOperand.emplace(SymbolName, Offset);
                return true;
            }
            Context.ModuleBuilder.fail("InkVM memory operand must be register-relative or symbol-relative");
            return false;
        }
        else if constexpr (std::is_same_v<OperandType, ink::vm::ImmediateOperand>)
        {
            const MachineOperand& MO = *Context.ExplicitOperands[Cursor];
            if (MO.isImm())
            {
                ++Cursor;
                OutOperand.emplace(static_cast<std::uint64_t>(MO.getImm()));
                return true;
            }
            if (MO.isGlobal() || MO.isSymbol())
            {
                if constexpr (BitWidth != 64)
                {
                    Context.ModuleBuilder.fail("InkVM symbol immediate requires a 64-bit immediate field");
                    return false;
                }
                else
                {
                    ink::core::Name SymbolName;
                    std::int64_t Offset = 0;
                    if (!decodeSymbolOperand(MO, Context.ModuleBuilder, SymbolName, Offset))
                    {
                        return false;
                    }
                    ++Cursor;
                    OutOperand.emplace(SymbolName, Offset);
                    return true;
                }
            }
            Context.ModuleBuilder.fail("InkVM immediate operand must be a numeric immediate or symbol address");
            return false;
        }
        else
        {
            static_assert(std::is_same_v<OperandType, ink::vm::BasicBlockOperand>);
            const MachineOperand& MO = *Context.ExplicitOperands[Cursor++];
            if (!MO.isMBB() || MO.getMBB() == nullptr)
            {
                Context.ModuleBuilder.fail("InkVM branch target operand must be a MachineBasicBlock");
                return false;
            }
            const auto Found = Context.BlockMap.find(MO.getMBB());
            if (Found == Context.BlockMap.end() || Found->second == nullptr)
            {
                Context.ModuleBuilder.fail("InkVM branch target MachineBasicBlock does not belong to its function");
                return false;
            }
            OutOperand.emplace(Found->second);
            return true;
        }
    }

    template <ink::vm::Opcode Operation>
    struct OpcodeEmitterAdapter
    {
            using Traits = ink::vm::OpcodeTraits<Operation>;
            using Tuple = typename Traits::OperandTuple;
            static constexpr std::size_t OperandCount = Traits::OperandCount;

            template <std::size_t... Indices>
            static bool emitWithIndices(ink::vm::MachineInstructionBuilder& Builder, const InstructionLoweringContext& Context, std::index_sequence<Indices...>)
            {
                std::size_t Cursor = 0;
                std::tuple<std::optional<std::tuple_element_t<Indices, Tuple>>...> DecodedOperands;
                const bool DecodedAll = (decodeVMOperand<std::tuple_element_t<Indices, Tuple>, Traits::BitWidth>(Context, Cursor, std::get<Indices>(DecodedOperands)) && ...);
                if (!DecodedAll)
                {
                    return false;
                }
                if (Cursor != Context.ExplicitOperands.size())
                {
                    Context.ModuleBuilder.fail("InkVM instruction has unexpected trailing operands");
                    return false;
                }
                ink::vm::MachineInstruction* Created = Builder.tryCreate<Operation>(*std::get<Indices>(DecodedOperands)...);
                if (Created == nullptr)
                {
                    Context.ModuleBuilder.fail("InkVM MachineInstructionBuilder rejected lowered operands");
                    return false;
                }
                if (const auto ValidationError = ink::vm::validateInstructionConstraints(*Created))
                {
                    Context.ModuleBuilder.fail(std::string(*ValidationError));
                    return false;
                }
                return true;
            }

            static bool emit(ink::vm::MachineInstructionBuilder& Builder, const InstructionLoweringContext& Context)
            {
                return emitWithIndices(Builder, Context, std::make_index_sequence<OperandCount>{});
            }
    };

    bool emitLoweredInstruction(ink::vm::Opcode VMOpcode, ink::vm::MachineInstructionBuilder& Builder, const InstructionLoweringContext& Context)
    {
        switch (VMOpcode)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case ink::vm::Opcode::Kind:                                                                                      \
        return OpcodeEmitterAdapter<ink::vm::Opcode::Kind>::emit(Builder, Context);
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        default:
            Context.ModuleBuilder.fail("InkVM instruction carries an unknown VM opcode encoding");
            return false;
        }
    }

    class InkVMMachineModuleEmitter final : public MachineFunctionPass
    {
        public:
            static char ID;

            explicit InkVMMachineModuleEmitter(InkVMMachineModuleEmitSink& Sink)
                : MachineFunctionPass(ID),
                  Sink(Sink)
            {
            }

            StringRef getPassName() const override
            {
                return PASS_NAME;
            }

            void getAnalysisUsage(AnalysisUsage& AU) const override
            {
                AU.setPreservesAll();
                MachineFunctionPass::getAnalysisUsage(AU);
            }

            bool doInitialization(Module& M) override
            {
                Sink.Output.reset();
                Sink.ErrorMessage.clear();
                Sink.Failed = false;
                Builder.initialize(M);
                return false;
            }

            bool runOnMachineFunction(MachineFunction& MF) override
            {
                if (!Builder.hasFailed())
                {
                    Builder.emitFunction(MF);
                }
                return false;
            }

            bool doFinalization(Module& M) override
            {
                static_cast<void>(M);
                auto Result = Builder.takeModule();
                if (Result)
                {
                    Sink.Output = std::move(*Result);
                    Sink.ErrorMessage.clear();
                    Sink.Failed = false;
                }
                else
                {
                    Sink.Output.reset();
                    Sink.ErrorMessage = std::move(Result.error());
                    Sink.Failed = true;
                }
                return false;
            }

        private:
            InkVMMachineModuleEmitSink& Sink;
            InkVMMachineModuleBuilder Builder;
    };
} // namespace

char InkVMMachineModuleEmitter::ID = 0;

INITIALIZE_PASS(InkVMMachineModuleEmitter, DEBUG_TYPE, PASS_NAME, false, false)

namespace llvm
{
    void InkVMMachineModuleBuilder::fail(std::string Message)
    {
        if (!Failed)
        {
            Failed = true;
            ErrorMessage = std::move(Message);
        }
    }

    std::expected<ink::core::Name, std::string> InkVMMachineModuleBuilder::internName(std::string_view Text)
    {
        const ink::core::Name Interned = ink::core::NamePool::instance().intern(Text);
        if (!Text.empty() && !Interned.valid())
        {
            return std::unexpected("InkVM name pool capacity exhausted");
        }
        return Interned;
    }

    bool InkVMMachineModuleBuilder::recordExternalSymbolDeclaration(ink::core::Name SymbolName)
    {
        if (Output == nullptr)
        {
            fail("InkVM MachineModule emitter was not initialized");
            return false;
        }
        if (SymbolName.valid() && KnownSymbols.insert(SymbolName).second)
        {
            Output->declareSymbol(SymbolName, ink::vm::MachineSymbolKind::Function, false);
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::initialize(const Module& IRModule)
    {
        Output.reset();
        KnownSymbols.clear();
        ErrorMessage.clear();
        Failed = false;

        const StringRef RawModuleName = IRModule.getName().empty() ? StringRef("inkvm") : StringRef(IRModule.getName());
        const auto ModuleName = internName(toStringView(RawModuleName));
        if (!ModuleName)
        {
            fail(ModuleName.error());
            return false;
        }

        Output = std::make_unique<ink::vm::MachineModule>(*ModuleName);
        if (!emitExternalDeclarations(IRModule) || !emitDataGlobals(IRModule))
        {
            return false;
        }

        for (const Function& Func : IRModule.functions())
        {
            if (Func.isDeclaration() || Func.getName().empty())
            {
                continue;
            }
            const auto FuncName = internName(toStringView(Func.getName()));
            if (!FuncName)
            {
                fail(FuncName.error());
                return false;
            }
            KnownSymbols.insert(*FuncName);
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::emitExternalDeclarations(const Module& IRModule)
    {
        for (const GlobalVariable& Global : IRModule.globals())
        {
            if (!Global.isDeclaration() || isMetadataGlobal(Global))
            {
                continue;
            }
            if (Global.getName().empty())
            {
                fail("InkVM external global declaration requires a nonempty name");
                return false;
            }
            const auto GlobalName = internName(toStringView(Global.getName()));
            if (!GlobalName)
            {
                fail(GlobalName.error());
                return false;
            }
            Output->declareSymbol(*GlobalName, ink::vm::MachineSymbolKind::GlobalVariable, Global.isConstant());
            KnownSymbols.insert(*GlobalName);
        }

        for (const Function& Func : IRModule.functions())
        {
            if (!Func.isDeclaration() || Func.isIntrinsic())
            {
                continue;
            }
            if (Func.getName().empty())
            {
                fail("InkVM external function declaration requires a nonempty name");
                return false;
            }
            const auto FuncName = internName(toStringView(Func.getName()));
            if (!FuncName)
            {
                fail(FuncName.error());
                return false;
            }
            Output->declareSymbol(*FuncName, ink::vm::MachineSymbolKind::Function, false);
            KnownSymbols.insert(*FuncName);
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::emitDataGlobals(const Module& IRModule)
    {
        const DataLayout& DL = IRModule.getDataLayout();
        for (const GlobalVariable& Global : IRModule.globals())
        {
            if (Global.isDeclaration() || isMetadataGlobal(Global))
            {
                continue;
            }

            Type* ValueTy = Global.getValueType();
            if (ValueTy == nullptr || !ValueTy->isSized())
            {
                fail("InkVM global variable must have a sized type");
                return false;
            }
            const TypeSize RawAllocSize = DL.getTypeAllocSize(ValueTy);
            if (RawAllocSize.isScalable())
            {
                fail("InkVM global variable does not support scalable types");
                return false;
            }
            const std::uint64_t ByteSize = RawAllocSize.getFixedValue();
            const std::uint64_t Alignment = std::max<std::uint64_t>(1, Global.getAlign().value_or(DL.getABITypeAlign(ValueTy)).value());
            const ink::vm::MachineSymbolLinkage Linkage = Global.hasLocalLinkage() ? ink::vm::MachineSymbolLinkage::Local : ink::vm::MachineSymbolLinkage::External;
            if (Linkage == ink::vm::MachineSymbolLinkage::External && Global.getName().empty())
            {
                fail("InkVM external global definition requires a nonempty name");
                return false;
            }

            const auto GlobalName = internName(toStringView(Global.getName()));
            if (!GlobalName)
            {
                fail(GlobalName.error());
                return false;
            }

            std::vector<std::uint8_t> InitialBytes;
            if (Global.hasInitializer())
            {
                InitialBytes.reserve(static_cast<std::size_t>(ByteSize));
                if (auto Serialized = serializeConstantBytes(Global.getInitializer(), DL, InitialBytes); !Serialized)
                {
                    fail(std::move(Serialized.error()));
                    return false;
                }
                while (!InitialBytes.empty() && InitialBytes.back() == 0)
                {
                    InitialBytes.pop_back();
                }
            }

            if (const auto Created = Output->createGlobalVariable(ByteSize, Alignment, InitialBytes, *GlobalName, Linkage, Global.isConstant()); !Created)
            {
                fail(std::string(formatGlobalDataError(Created.error())));
                return false;
            }
            if (GlobalName->valid())
            {
                KnownSymbols.insert(*GlobalName);
            }
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::emitFunction(const MachineFunction& MF)
    {
        if (Failed)
        {
            return false;
        }
        if (Output == nullptr)
        {
            fail("InkVM MachineModule emitter was not initialized");
            return false;
        }

        const Function& IRFunc = MF.getFunction();
        const ink::vm::MachineSymbolLinkage Linkage = IRFunc.hasLocalLinkage() ? ink::vm::MachineSymbolLinkage::Local : ink::vm::MachineSymbolLinkage::External;
        if (Linkage == ink::vm::MachineSymbolLinkage::External && IRFunc.getName().empty())
        {
            fail("InkVM external function definition requires a nonempty name");
            return false;
        }

        const auto FuncName = internName(toStringView(IRFunc.getName()));
        if (!FuncName)
        {
            fail(FuncName.error());
            return false;
        }

        ink::vm::MachineFunction& VMFunc = Output->createFunction(*FuncName, Linkage);
        if (FuncName->valid())
        {
            KnownSymbols.insert(*FuncName);
        }

        DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*> BlockMap;
        BlockMap.reserve(MF.size());
        for (const MachineBasicBlock& MBB : MF)
        {
            ink::core::Name BlockName = ink::core::NAME_None;
            if (const BasicBlock* IRBlock = MBB.getBasicBlock(); IRBlock != nullptr && !IRBlock->getName().empty())
            {
                const auto InternedBlockName = internName(toStringView(IRBlock->getName()));
                if (!InternedBlockName)
                {
                    fail(InternedBlockName.error());
                    return false;
                }
                BlockName = *InternedBlockName;
            }
            ink::vm::MachineBasicBlock& VMBlock = VMFunc.createBasicBlock(BlockName);
            BlockMap.try_emplace(&MBB, &VMBlock);
        }

        const TargetSubtargetInfo& STI = MF.getSubtarget();
        const TargetRegisterInfo* TRI = STI.getRegisterInfo();
        const TargetInstrInfo* TII = STI.getInstrInfo();
        if (TRI == nullptr || TII == nullptr)
        {
            fail("InkVM subtarget is missing register or instruction info");
            return false;
        }

        ink::vm::MachineInstructionBuilder Builder;
        SmallVector<const MachineOperand*, 4> ExplicitOperands;
        for (const MachineBasicBlock& MBB : MF)
        {
            ink::vm::MachineBasicBlock* VMBlock = BlockMap.lookup(&MBB);
            Builder.setInsertPoint(*VMBlock);
            for (const MachineInstr& MI : MBB)
            {
                if (MI.isMetaInstruction())
                {
                    continue;
                }
                const MCInstrDesc& Desc = MI.getDesc();
                if (Desc.isPseudo() || !ink::backend::llvm_inkvm::hasEncodedVMOpcode(Desc.TSFlags))
                {
                    fail("InkVM encountered unlowered or non-VM MachineInstr: " + TII->getName(MI.getOpcode()).str());
                    return false;
                }

                ExplicitOperands.clear();
                for (const MachineOperand& MO : MI.operands())
                {
                    if ((MO.isReg() && MO.isImplicit()) || MO.isMetadata() || MO.isRegMask())
                    {
                        continue;
                    }
                    ExplicitOperands.push_back(&MO);
                }

                const auto VMOpcode = static_cast<ink::vm::Opcode>(ink::backend::llvm_inkvm::decodeVMOpcodeFromTSFlags(Desc.TSFlags));
                const InstructionLoweringContext Context{*this, *TRI, BlockMap, ExplicitOperands};
                if (!emitLoweredInstruction(VMOpcode, Builder, Context))
                {
                    return false;
                }
            }
        }
        return true;
    }

    std::expected<std::unique_ptr<ink::vm::MachineModule>, std::string> InkVMMachineModuleBuilder::takeModule()
    {
        if (Failed)
        {
            Output.reset();
            return std::unexpected(ErrorMessage.empty() ? std::string("InkVM MachineModule emission failed") : std::move(ErrorMessage));
        }
        if (Output == nullptr)
        {
            return std::unexpected("InkVM MachineModule emitter produced no module");
        }
        return std::move(Output);
    }

    FunctionPass* createInkVMMachineModuleEmitter(InkVMTargetMachine& TM, InkVMMachineModuleEmitSink& Sink)
    {
        static_cast<void>(TM);
        return new InkVMMachineModuleEmitter(Sink);
    }
} // namespace llvm
