#include "MCTargetDesc/InkVMInstPrinter.h"

#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCExpr.h>
#include <llvm/MC/MCInst.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdint>

#define DEBUG_TYPE "inkvm-asm-printer"

using namespace llvm;

#include "InkVMGenAsmWriter.inc"

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
        const std::int64_t Offset = Disp.getImm();
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
    else
    {
        llvm_unreachable("Unsupported InkVM memory displacement operand");
    }
    OS << ']';
}
