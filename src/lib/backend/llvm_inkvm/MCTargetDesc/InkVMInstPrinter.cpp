#include "MCTargetDesc/InkVMInstPrinter.h"

#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCExpr.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCSymbol.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdint>

#define DEBUG_TYPE "inkvm-asm-printer"

using namespace llvm;

#include "InkVMGenAsmWriter.inc"

namespace
{
    bool isSymbolIdentifier(StringRef Text)
    {
        if (Text.empty())
        {
            return false;
        }
        const auto IsStart = [](char Character)
        {
            return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || Character == '_' || Character == '.' || Character == '$';
        };
        if (!IsStart(Text.front()))
        {
            return false;
        }
        for (const char Character : Text)
        {
            if (!IsStart(Character) && !(Character >= '0' && Character <= '9'))
            {
                return false;
            }
        }
        return true;
    }

    void writeSymbolName(raw_ostream& OS, StringRef Name)
    {
        static constexpr char HexDigits[] = "0123456789abcdef";
        OS << '@';
        if (isSymbolIdentifier(Name))
        {
            OS << Name;
            return;
        }
        OS << '"';
        for (const unsigned char Byte : Name.bytes())
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

    void writeSignedDisplacement(raw_ostream& OS, std::int64_t Offset)
    {
        if (Offset > 0)
        {
            OS << " + " << Offset;
        }
        else if (Offset < 0)
        {
            const std::uint64_t Magnitude = std::uint64_t{0} - static_cast<std::uint64_t>(Offset);
            OS << " - " << Magnitude;
        }
    }
} // namespace

void InkVMInstPrinter::printRegName(raw_ostream& OS, MCRegister Reg)
{
    OS << getRegisterName(Reg);
}

void InkVMInstPrinter::printInst(const MCInst* MI, uint64_t Address, StringRef Annot, const MCSubtargetInfo& STI, raw_ostream& OS)
{
    static_cast<void>(STI);
    printInstruction(MI, Address, OS);
    printAnnotation(OS, Annot);
}

void InkVMInstPrinter::printOperand(const MCInst* MI, unsigned OpNo, raw_ostream& OS)
{
    const MCOperand& Op = MI->getOperand(OpNo);
    if (Op.isReg())
    {
        OS << getRegisterName(Op.getReg());
        return;
    }
    if (Op.isImm())
    {
        OS << Op.getImm();
        return;
    }
    if (Op.isExpr())
    {
        MAI.printExpr(OS, *Op.getExpr());
        return;
    }
    llvm_unreachable("Unsupported InkVM MCOperand kind");
}

void InkVMInstPrinter::printMemOperand(const MCInst* MI, unsigned OpNo, raw_ostream& OS)
{
    const MCOperand& Base = MI->getOperand(OpNo);
    const MCOperand& Disp = MI->getOperand(OpNo + 1);

    OS << '[';
    if (Base.isReg())
    {
        OS << getRegisterName(Base.getReg());
    }
    else if (Base.isExpr())
    {
        MAI.printExpr(OS, *Base.getExpr());
    }
    else
    {
        llvm_unreachable("Unsupported InkVM memory base operand");
    }

    if (Disp.isImm())
    {
        writeSignedDisplacement(OS, Disp.getImm());
    }
    else
    {
        llvm_unreachable("Unsupported InkVM memory displacement operand");
    }
    OS << ']';
}

void InkVMInstPrinter::printSymbolOperand(const MCInst* MI, unsigned OpNo, raw_ostream& OS)
{
    const MCOperand& Op = MI->getOperand(OpNo);
    if (Op.isImm())
    {
        OS << Op.getImm();
        return;
    }
    if (!Op.isExpr())
    {
        llvm_unreachable("Unsupported InkVM symbol operand");
    }

    const MCExpr* Expr = Op.getExpr();
    if (const auto* SymRef = dyn_cast<MCSymbolRefExpr>(Expr))
    {
        writeSymbolName(OS, SymRef->getSymbol().getName());
        return;
    }
    if (const auto* BinExpr = dyn_cast<MCBinaryExpr>(Expr))
    {
        if (const auto* SymRef = dyn_cast<MCSymbolRefExpr>(BinExpr->getLHS()))
        {
            if (const auto* ConstOffset = dyn_cast<MCConstantExpr>(BinExpr->getRHS()))
            {
                writeSymbolName(OS, SymRef->getSymbol().getName());
                const std::int64_t Offset = BinExpr->getOpcode() == MCBinaryExpr::Sub ? -ConstOffset->getValue() : ConstOffset->getValue();
                writeSignedDisplacement(OS, Offset);
                return;
            }
        }
    }
    llvm_unreachable("Unsupported InkVM symbol expression");
}
