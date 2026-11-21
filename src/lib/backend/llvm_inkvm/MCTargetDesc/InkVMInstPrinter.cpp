#include "MCTargetDesc/InkVMInstPrinter.h"

#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCExpr.h>
#include <llvm/MC/MCInst.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

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
