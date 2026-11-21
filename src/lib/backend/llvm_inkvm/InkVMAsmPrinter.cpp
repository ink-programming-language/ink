#include "InkVM.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"
#include "TargetInfo/InkVMTargetInfo.h"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/CodeGen/AsmPrinter.h>
#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstr.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCExpr.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Pass.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetLoweringObjectFile.h>
#include <llvm/Target/TargetMachine.h>

#include <cstddef>
#include <string>

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

                SmallString<128> Header;
                raw_svector_ostream OS(Header);
                const StringRef ModuleName = M.getName().empty() ? StringRef("inkvm") : StringRef(M.getName());
                OS << ".module ";
                writeEscapedString(OS, ModuleName);
                OutStreamer->emitRawText(OS.str());

                emitExternalDeclarations(M);
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
                    if (!Global.isDeclaration())
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
