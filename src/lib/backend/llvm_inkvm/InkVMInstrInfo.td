// InkVM instruction descriptions and SelectionDAG patterns.

// Target-specific SelectionDAG node emitted by InkVMTargetLowering::LowerReturn().
def InkVMRetGlue : SDNode<"InkVMISD::RET_GLUE", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

class InkVMInst<dag Outs, dag Ins, string AsmStr, list<dag> Pat = []> : Instruction {
  let Namespace = "InkVM";
  dag OutOperandList = Outs;
  dag InOperandList = Ins;
  let AsmString = AsmStr;
  let Pattern = Pat;
  let Size = 8;
}

// 0x003A: ret (8-byte header word, no explicit operands).
let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : InkVMInst<(outs), (ins), "ret", [(InkVMRetGlue)]>;
}
