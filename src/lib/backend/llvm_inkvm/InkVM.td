// Top-level TableGen description for the InkVM LLVM target backend.

include "llvm/Target/Target.td"

include "InkVMRegisterInfo.td"
include "InkVMInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def InkVMInstrInfo : InstrInfo;

class InkVMProcessor<string Name, list<SubtargetFeature> Features>
    : Processor<Name, NoItineraries, Features>;

def : InkVMProcessor<"generic", []>;

def InkVMAsmWriter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
}

def InkVM : Target {
  let InstructionSet = InkVMInstrInfo;
  let AssemblyWriters = [InkVMAsmWriter];
}
