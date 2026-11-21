// InkVM instruction descriptions and SelectionDAG patterns.

// Target-specific SelectionDAG node emitted by InkVMTargetLowering::LowerReturn().
def InkVMRetGlue : SDNode<"InkVMISD::RET_GLUE", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Register-plus-signed-64-bit-displacement memory operand: [base + disp].
def MemOp : Operand<i64> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPR:$base, i64imm:$disp);
}

def AddrRI : ComplexPattern<i64, 2, "SelectAddrRI", [frameindex], []>;

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

// 0x0097-0x0098: 64-bit register move and full 64-bit immediate materialization.
let Size = 24, hasSideEffects = 0, isAsCheapAsAMove = 1 in {
  let isMoveReg = 1 in {
    def MOV64rr : InkVMInst<(outs GPR:$dst), (ins GPR:$src),
                            "mov64rr $dst, $src">;
  }

  let isMoveImm = 1, isReMaterializable = 1 in {
    def MOV64ri : InkVMInst<(outs GPR:$dst), (ins i64imm:$imm),
                            "mov64ri $dst, $imm",
                            [(set GPR:$dst, (i64 imm:$imm))]>;
  }
}

// Multiclass generating paired 64-bit register-register (rrr) and register-immediate (rri) ALU ops.
multiclass ALU64BinOp<string MnemonicBase, SDNode OpNode, bit IsComm = 0> {
  let Size = 32, hasSideEffects = 0 in {
    let isCommutable = IsComm in {
      def 64rrr : InkVMInst<(outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                            !strconcat(MnemonicBase, "64rrr $dst, $lhs, $rhs"),
                            [(set GPR:$dst, (OpNode GPR:$lhs, GPR:$rhs))]>;
    }

    def 64rri : InkVMInst<(outs GPR:$dst), (ins GPR:$lhs, i64imm:$imm),
                          !strconcat(MnemonicBase, "64rri $dst, $lhs, $imm"),
                          [(set GPR:$dst, (OpNode GPR:$lhs, (i64 imm:$imm)))]>;
  }
}

// 0x0019, 0x001B, 0x00A4-0x00A7: 64-bit integer add, sub, mul.
defm ADD : ALU64BinOp<"add", add, 1>;
defm SUB : ALU64BinOp<"sub", sub, 0>;
defm MUL : ALU64BinOp<"mul", mul, 1>;

// 0x00AC-0x00B7: 64-bit bitwise operations and shifts.
defm AND  : ALU64BinOp<"and",  and, 1>;
defm OR   : ALU64BinOp<"or",   or,  1>;
defm XOR  : ALU64BinOp<"xor",  xor, 1>;
defm SHL  : ALU64BinOp<"shl",  shl, 0>;
defm LSHR : ALU64BinOp<"lshr", srl, 0>;
defm ASHR : ALU64BinOp<"ashr", sra, 0>;

// Negation uses architectural ZERO as LHS: sub64rrr dst, zero, rhs.
def : Pat<(sub 0, GPR:$rhs), (SUB64rrr ZERO, GPR:$rhs)>;

// 0x00A8-0x00AB: 64-bit signed/unsigned division and remainder (traps on /0 or MIN/-1).
class ALU64DivRem<string Mnemonic, SDNode OpNode>
    : InkVMInst<(outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                !strconcat(Mnemonic, " $dst, $lhs, $rhs"),
                [(set GPR:$dst, (OpNode GPR:$lhs, GPR:$rhs))]> {
  let Size = 32;
  let hasSideEffects = 1;
}

def SDIV64rrr : ALU64DivRem<"sdiv64rrr", sdiv>;
def UDIV64rrr : ALU64DivRem<"udiv64rrr", udiv>;
def SREM64rrr : ALU64DivRem<"srem64rrr", srem>;
def UREM64rrr : ALU64DivRem<"urem64rrr", urem>;

// 0x0099-0x00A3: 8/16/32/64-bit register-relative memory loads (zero/sign-extending) and stores.
let Size = 24, hasSideEffects = 0 in {
  let mayLoad = 1 in {
    def LOAD8urm  : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load8urm $dst, $addr",
                              [(set GPR:$dst, (zextloadi8 AddrRI:$addr))]>;
    def LOAD16urm : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load16urm $dst, $addr",
                              [(set GPR:$dst, (zextloadi16 AddrRI:$addr))]>;
    def LOAD32urm : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load32urm $dst, $addr",
                              [(set GPR:$dst, (zextloadi32 AddrRI:$addr))]>;

    def LOAD8srm  : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load8srm $dst, $addr",
                              [(set GPR:$dst, (sextloadi8 AddrRI:$addr))]>;
    def LOAD16srm : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load16srm $dst, $addr",
                              [(set GPR:$dst, (sextloadi16 AddrRI:$addr))]>;
    def LOAD32srm : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load32srm $dst, $addr",
                              [(set GPR:$dst, (sextloadi32 AddrRI:$addr))]>;

    def LOAD64rm  : InkVMInst<(outs GPR:$dst), (ins MemOp:$addr),
                              "load64rm $dst, $addr",
                              [(set GPR:$dst, (load AddrRI:$addr))]>;
  }

  let mayStore = 1 in {
    def STORE8mr  : InkVMInst<(outs), (ins MemOp:$addr, GPR:$src),
                              "store8mr $addr, $src",
                              [(truncstorei8 GPR:$src, AddrRI:$addr)]>;
    def STORE16mr : InkVMInst<(outs), (ins MemOp:$addr, GPR:$src),
                              "store16mr $addr, $src",
                              [(truncstorei16 GPR:$src, AddrRI:$addr)]>;
    def STORE32mr : InkVMInst<(outs), (ins MemOp:$addr, GPR:$src),
                              "store32mr $addr, $src",
                              [(truncstorei32 GPR:$src, AddrRI:$addr)]>;
    def STORE64mr : InkVMInst<(outs), (ins MemOp:$addr, GPR:$src),
                              "store64mr $addr, $src",
                              [(store GPR:$src, AddrRI:$addr)]>;
  }
}

// Any-extending narrow loads reuse unsigned zero-extending loads.
def : Pat<(i64 (extloadi8  AddrRI:$addr)), (LOAD8urm  AddrRI:$addr)>;
def : Pat<(i64 (extloadi16 AddrRI:$addr)), (LOAD16urm AddrRI:$addr)>;
def : Pat<(i64 (extloadi32 AddrRI:$addr)), (LOAD32urm AddrRI:$addr)>;
