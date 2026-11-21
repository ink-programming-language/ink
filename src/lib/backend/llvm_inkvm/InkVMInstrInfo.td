// InkVM instruction descriptions.
// Step 1 starts with the minimal 0-operand return instruction (opcode 0x003A).

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
  def RET : InkVMInst<(outs), (ins), "ret">;
}
