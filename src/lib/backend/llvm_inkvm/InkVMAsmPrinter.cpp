#include "InkVM.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"
#include "TargetInfo/InkVMTargetInfo.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/CodeGen/AsmPrinter.h>
#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstr.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCExpr.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetLoweringObjectFile.h>
#include <llvm/Target/TargetMachine.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#define DEBUG_TYPE "inkvm-asm-printer"
#define PASS_NAME "InkVM Assembly Printer"

using namespace llvm;

namespace
{
    bool isIdentifierStart(char Character)
    {
        return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || Character == '_' || Character == '.' || Character == '$';
    }

    bool isIdentifierContinue(char Character)
    {
        return isIdentifierStart(Character) || (Character >= '0' && Character <= '9');
    }

    bool isLabelIdentifier(StringRef Text)
    {
        if (Text.empty() || !isIdentifierStart(Text.front()))
        {
            return false;
        }
        for (const char Character : Text)
        {
            if (!isIdentifierContinue(Character))
            {
                return false;
            }
        }
        return true;
    }

    void writeEscapedString(raw_ostream& OS, StringRef Text)
    {
        static constexpr char HexDigits[] = "0123456789abcdef";
        OS << '"';
        for (const unsigned char Byte : Text.bytes())
        {
            switch (Byte)
            {
            case '\\':
                OS << "\\\\";
                break;
            case '"':
                OS << "\\\"";
                break;
            case '\n':
                OS << "\\n";
                break;
            case '\r':
                OS << "\\r";
                break;
            case '\t':
                OS << "\\t";
                break;
            default:
                if (Byte < 0x20 || Byte >= 0x7F)
                {
                    OS << "\\x" << HexDigits[Byte >> 4] << HexDigits[Byte & 0x0F];
                }
                else
                {
                    OS << static_cast<char>(Byte);
                }
                break;
            }
        }
        OS << '"';
    }

    void writeSymbolReference(raw_ostream& OS, StringRef Name)
    {
        OS << '@';
        if (isLabelIdentifier(Name))
        {
            OS << Name;
            return;
        }
        writeEscapedString(OS, Name);
    }

    bool isMetadataGlobal(const GlobalVariable& Global)
    {
        return Global.getSection() == "llvm.metadata" || Global.getName().starts_with("llvm.");
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

    void serializeConstantBytes(const Constant* ConstVal, const DataLayout& DL, std::vector<std::uint8_t>& OutBytes)
    {
        const std::size_t StoreSize = static_cast<std::size_t>(DL.getTypeStoreSize(ConstVal->getType()).getFixedValue());

        if (isa<ConstantAggregateZero>(ConstVal) || isa<UndefValue>(ConstVal) || isa<ConstantPointerNull>(ConstVal))
        {
            OutBytes.insert(OutBytes.end(), StoreSize, 0);
            return;
        }

        if (const auto* IntConst = dyn_cast<ConstantInt>(ConstVal))
        {
            appendAPIntLittleEndian(IntConst->getValue(), StoreSize, OutBytes);
            return;
        }

        if (const auto* FPConst = dyn_cast<ConstantFP>(ConstVal))
        {
            appendAPIntLittleEndian(FPConst->getValueAPF().bitcastToAPInt(), StoreSize, OutBytes);
            return;
        }

        if (const auto* DataSeq = dyn_cast<ConstantDataSequential>(ConstVal))
        {
            if (DataSeq->getElementByteSize() == 1)
            {
                const StringRef Raw = DataSeq->getRawDataValues();
                OutBytes.insert(OutBytes.end(), Raw.bytes_begin(), Raw.bytes_end());
                return;
            }
            for (unsigned Index = 0; Index < DataSeq->getNumElements(); ++Index)
            {
                serializeConstantBytes(DataSeq->getElementAsConstant(Index), DL, OutBytes);
            }
            return;
        }

        if (const auto* ArrayConst = dyn_cast<ConstantArray>(ConstVal))
        {
            for (unsigned Index = 0; Index < ArrayConst->getNumOperands(); ++Index)
            {
                serializeConstantBytes(ArrayConst->getOperand(Index), DL, OutBytes);
            }
            return;
        }

        if (const auto* VectorConst = dyn_cast<ConstantVector>(ConstVal))
        {
            for (unsigned Index = 0; Index < VectorConst->getNumOperands(); ++Index)
            {
                serializeConstantBytes(VectorConst->getOperand(Index), DL, OutBytes);
            }
            return;
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
                serializeConstantBytes(StructConst->getOperand(FieldIndex), DL, OutBytes);
            }
            const std::size_t EndOffset = BaseOffset + static_cast<std::size_t>(Layout->getSizeInBytes());
            if (OutBytes.size() < EndOffset)
            {
                OutBytes.resize(EndOffset, 0);
            }
            return;
        }

        report_fatal_error("InkVM global initializer kind is not supported yet (symbol relocations in .data require a future .addr directive)");
    }

    class InkVMAsmPrinter : public AsmPrinter
    {
        public:
            static char ID;

            InkVMAsmPrinter(TargetMachine& TM, std::unique_ptr<MCStreamer> Streamer)
                : AsmPrinter(TM, std::move(Streamer), ID)
            {
            }

            StringRef getPassName() const override
            {
                return PASS_NAME;
            }

            bool doInitialization(Module& M) override
            {
                auto* MMIWP = getAnalysisIfAvailable<MachineModuleInfoWrapperPass>();
                MMI = MMIWP != nullptr ? &MMIWP->getMMI() : nullptr;
                TM.getObjFileLowering()->Initialize(OutContext, TM);
                TextSectionEmitted = false;
                AnonymousFunctionCounter = 0;
                AnonymousGlobalCounter = 0;

                SmallString<128> Header;
                raw_svector_ostream OS(Header);
                const StringRef ModuleName = M.getName().empty() ? StringRef("inkvm") : StringRef(M.getName());
                OS << ".module ";
                writeEscapedString(OS, ModuleName);
                OutStreamer->emitRawText(OS.str());

                emitExternalDeclarations(M);
                emitDataSection(M);
                return false;
            }

            bool runOnMachineFunction(MachineFunction& MachineFunc) override
            {
                SetupMachineFunction(MachineFunc);

                if (!TextSectionEmitted)
                {
                    OutStreamer->emitRawText(".section .text");
                    TextSectionEmitted = true;
                }

                assignBasicBlockLabels(MachineFunc);
                emitFunctionOpenDirective(MachineFunc.getFunction());

                for (const MachineBasicBlock& MBB : MachineFunc)
                {
                    const auto LabelIt = BlockLabels.find(&MBB);
                    OutStreamer->emitRawText(LabelIt->second + ":");

                    for (const MachineInstr& MI : MBB)
                    {
                        if (MI.isMetaInstruction())
                        {
                            continue;
                        }
                        emitInstruction(&MI);
                    }
                }

                OutStreamer->emitRawText(".endfunction");
                BlockLabels.clear();
                return false;
            }

            void emitInstruction(const MachineInstr* MI) override
            {
                MCInst Lowered;
                lowerInstruction(*MI, Lowered);
                EmitToStreamer(*OutStreamer, Lowered);
            }

            bool doFinalization(Module& M) override
            {
                static_cast<void>(M);
                MF = nullptr;
                MMI = nullptr;
                OutStreamer->finish();
                OutStreamer->reset();
                return false;
            }

        private:
            void emitExternalDeclarations(const Module& M)
            {
                for (const GlobalVariable& Global : M.globals())
                {
                    if (!Global.isDeclaration() || isMetadataGlobal(Global))
                    {
                        continue;
                    }
                    SmallString<128> Line;
                    raw_svector_ostream OS(Line);
                    OS << ".declare global ";
                    writeSymbolReference(OS, Global.getName());
                    if (Global.isConstant())
                    {
                        OS << " readonly";
                    }
                    OutStreamer->emitRawText(OS.str());
                }

                for (const Function& Func : M.functions())
                {
                    if (!Func.isDeclaration() || Func.isIntrinsic())
                    {
                        continue;
                    }
                    SmallString<128> Line;
                    raw_svector_ostream OS(Line);
                    OS << ".declare function ";
                    writeSymbolReference(OS, Func.getName());
                    OutStreamer->emitRawText(OS.str());
                }
            }

            void emitDataSection(const Module& M)
            {
                const DataLayout& DL = M.getDataLayout();
                bool DataSectionEmitted = false;
                static constexpr char UpperHex[] = "0123456789ABCDEF";

                for (const GlobalVariable& Global : M.globals())
                {
                    if (Global.isDeclaration() || isMetadataGlobal(Global))
                    {
                        continue;
                    }

                    if (!DataSectionEmitted)
                    {
                        OutStreamer->emitRawText(".section .data");
                        DataSectionEmitted = true;
                    }

                    Type* ValueTy = Global.getValueType();
                    const std::uint64_t ByteSize = DL.getTypeAllocSize(ValueTy).getFixedValue();
                    const std::uint64_t Alignment = std::max<std::uint64_t>(1, Global.getAlign().value_or(DL.getABITypeAlign(ValueTy)).value());

                    SmallString<128> HeaderLine;
                    raw_svector_ostream HeaderOS(HeaderLine);
                    HeaderOS << ".global ";
                    const StringRef Name = Global.getName();
                    if (isLabelIdentifier(Name))
                    {
                        HeaderOS << Name;
                    }
                    else
                    {
                        HeaderOS << ".G" << AnonymousGlobalCounter++ << ' ';
                        writeEscapedString(HeaderOS, Name);
                    }
                    HeaderOS << ", " << ByteSize << ", " << Alignment;
                    if (Global.isConstant())
                    {
                        HeaderOS << " readonly";
                    }
                    if (!Global.hasLocalLinkage())
                    {
                        HeaderOS << " external";
                    }
                    OutStreamer->emitRawText(HeaderOS.str());

                    if (Global.hasInitializer())
                    {
                        std::vector<std::uint8_t> Bytes;
                        Bytes.reserve(static_cast<std::size_t>(ByteSize));
                        serializeConstantBytes(Global.getInitializer(), DL, Bytes);
                        while (!Bytes.empty() && Bytes.back() == 0)
                        {
                            Bytes.pop_back();
                        }

                        constexpr std::size_t BytesPerLine = 16;
                        for (std::size_t Offset = 0; Offset < Bytes.size(); Offset += BytesPerLine)
                        {
                            const std::size_t Count = std::min(BytesPerLine, Bytes.size() - Offset);
                            SmallString<128> ByteLine;
                            raw_svector_ostream ByteOS(ByteLine);
                            ByteOS << "    .byte ";
                            for (std::size_t Index = 0; Index < Count; ++Index)
                            {
                                if (Index > 0)
                                {
                                    ByteOS << ", ";
                                }
                                const std::uint8_t Value = Bytes[Offset + Index];
                                ByteOS << "0x" << UpperHex[Value >> 4] << UpperHex[Value & 0x0F];
                            }
                            OutStreamer->emitRawText(ByteOS.str());
                        }
                    }

                    OutStreamer->emitRawText(".endglobal");
                }
            }

            void emitFunctionOpenDirective(const Function& Func)
            {
                SmallString<128> Line;
                raw_svector_ostream OS(Line);
                OS << ".function ";
                const StringRef Name = Func.getName();
                if (isLabelIdentifier(Name))
                {
                    OS << Name;
                }
                else
                {
                    OS << ".F" << AnonymousFunctionCounter++ << ' ';
                    writeEscapedString(OS, Name);
                }
                if (!Func.hasLocalLinkage())
                {
                    OS << " external";
                }
                OutStreamer->emitRawText(OS.str());
            }

            void assignBasicBlockLabels(const MachineFunction& Function)
            {
                BlockLabels.clear();
                StringSet<> UsedLabels;
                for (const MachineBasicBlock& MBB : Function)
                {
                    std::string Candidate;
                    if (const BasicBlock* IRBlock = MBB.getBasicBlock(); IRBlock != nullptr && isLabelIdentifier(IRBlock->getName()))
                    {
                        Candidate = IRBlock->getName().str();
                    }
                    else
                    {
                        Candidate = "bb" + std::to_string(MBB.getNumber());
                    }

                    if (!UsedLabels.insert(Candidate).second)
                    {
                        std::size_t Suffix = 1;
                        for (;;)
                        {
                            std::string Disambiguated = Candidate + "." + std::to_string(Suffix++);
                            if (UsedLabels.insert(Disambiguated).second)
                            {
                                Candidate = std::move(Disambiguated);
                                break;
                            }
                        }
                    }

                    BlockLabels.try_emplace(&MBB, std::move(Candidate));
                }
            }

            void lowerInstruction(const MachineInstr& MI, MCInst& OutMI) const
            {
                OutMI.setOpcode(MI.getOpcode());
                for (const MachineOperand& MO : MI.operands())
                {
                    if (MO.isReg() && MO.isImplicit())
                    {
                        continue;
                    }
                    if (MO.isMetadata() || MO.isRegMask())
                    {
                        continue;
                    }
                    OutMI.addOperand(lowerOperand(MO));
                }
            }

            MCOperand lowerOperand(const MachineOperand& MO) const
            {
                switch (MO.getType())
                {
                case MachineOperand::MO_Register:
                    return MCOperand::createReg(MO.getReg());
                case MachineOperand::MO_Immediate:
                    return MCOperand::createImm(MO.getImm());
                case MachineOperand::MO_MachineBasicBlock:
                    {
                        const auto LabelIt = BlockLabels.find(MO.getMBB());
                        const StringRef Label = LabelIt != BlockLabels.end() ? StringRef(LabelIt->second) : MO.getMBB()->getSymbol()->getName();
                        MCSymbol* Sym = OutContext.getOrCreateSymbol(Label);
                        return MCOperand::createExpr(MCSymbolRefExpr::create(Sym, OutContext));
                    }
                case MachineOperand::MO_GlobalAddress:
                    {
                        MCSymbol* Sym = OutContext.getOrCreateSymbol(MO.getGlobal()->getName());
                        const MCExpr* Expr = MCSymbolRefExpr::create(Sym, OutContext);
                        if (const std::int64_t Offset = MO.getOffset(); Offset != 0)
                        {
                            Expr = MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, OutContext), OutContext);
                        }
                        return MCOperand::createExpr(Expr);
                    }
                case MachineOperand::MO_ExternalSymbol:
                    {
                        MCSymbol* Sym = OutContext.getOrCreateSymbol(MO.getSymbolName());
                        return MCOperand::createExpr(MCSymbolRefExpr::create(Sym, OutContext));
                    }
                default:
                    llvm_unreachable("Unsupported InkVM MachineOperand kind");
                }
            }

            bool TextSectionEmitted = false;
            std::size_t AnonymousFunctionCounter = 0;
            std::size_t AnonymousGlobalCounter = 0;
            DenseMap<const MachineBasicBlock*, std::string> BlockLabels;
    };
} // namespace

char InkVMAsmPrinter::ID = 0;

INITIALIZE_PASS(InkVMAsmPrinter, DEBUG_TYPE, PASS_NAME, false, false)

namespace llvm
{
    extern "C" void LLVMInitializeInkVMAsmPrinter()
    {
        RegisterAsmPrinter<InkVMAsmPrinter> RegisteredAsmPrinter(getTheInkVMTarget());
    }
} // namespace llvm
