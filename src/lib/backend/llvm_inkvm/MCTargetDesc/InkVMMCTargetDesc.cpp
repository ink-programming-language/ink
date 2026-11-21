#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include "MCTargetDesc/InkVMMCAsmInfo.h"
#include "TargetInfo/InkVMTargetInfo.h"

#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/TargetRegistry.h>

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "InkVMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "InkVMGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "InkVMGenRegisterInfo.inc"

namespace llvm
{
    namespace
    {
        MCInstrInfo* createInkVMMCInstrInfo()
        {
            auto* Info = new MCInstrInfo();
            InitInkVMMCInstrInfo(Info);
            return Info;
        }

        MCRegisterInfo* createInkVMMCRegisterInfo(const Triple& TargetTriple)
        {
            static_cast<void>(TargetTriple);
            auto* Info = new MCRegisterInfo();
            InitInkVMMCRegisterInfo(Info, InkVM::PC);
            return Info;
        }

        MCAsmInfo* createInkVMMCAsmInfo(const MCRegisterInfo& RegInfo, const Triple& TargetTriple, const MCTargetOptions& Options)
        {
            static_cast<void>(RegInfo);
            static_cast<void>(Options);
            return new InkVMMCAsmInfo(TargetTriple);
        }

        MCSubtargetInfo* createInkVMMCSubtargetInfo(const Triple& TargetTriple, StringRef CPU, StringRef Features)
        {
            const StringRef ResolvedCPU = CPU.empty() ? StringRef("generic") : CPU;
            return createInkVMMCSubtargetInfoImpl(TargetTriple, ResolvedCPU, ResolvedCPU, Features);
        }
    } // namespace

    extern "C" void LLVMInitializeInkVMTargetMC()
    {
        Target& TheTarget = getTheInkVMTarget();
        TargetRegistry::RegisterMCAsmInfo(TheTarget, createInkVMMCAsmInfo);
        TargetRegistry::RegisterMCInstrInfo(TheTarget, createInkVMMCInstrInfo);
        TargetRegistry::RegisterMCRegInfo(TheTarget, createInkVMMCRegisterInfo);
        TargetRegistry::RegisterMCSubtargetInfo(TheTarget, createInkVMMCSubtargetInfo);
    }
} // namespace llvm
