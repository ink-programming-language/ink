// InkVM instruction descriptions and SelectionDAG patterns.

// Target-specific SelectionDAG nodes.
def InkVMRetGlue : SDNode<"InkVMISD::RET_GLUE", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def InkVMCall : SDNode<"InkVMISD::CALL", SDTypeProfile<0, -1, [SDTCisVT<0, i64>]>,
                       [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;

def InkVMWrapper : SDNode<"InkVMISD::WRAPPER", SDTIntUnaryOp>;

def InkVMSelectCC : SDNode<"InkVMISD::SELECT_CC",
                           SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>, SDTCisVT<3, i64>,
                                                SDTCisSameAs<0, 4>, SDTCisSameAs<0, 5>]>>;

def InkVMCallSeqStart : SDNode<"ISD::CALLSEQ_START",
                               SDCallSeqStart<[SDTCisVT<0, i64>, SDTCisVT<1, i64>]>,
                               [SDNPHasChain, SDNPOutGlue]>;

def InkVMCallSeqEnd : SDNode<"ISD::CALLSEQ_END",
                             SDCallSeqEnd<[SDTCisVT<0, i64>, SDTCisVT<1, i64>]>,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Register-plus-signed-64-bit-displacement memory operand: [base + disp].
def MemOp : Operand<i64> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPR:$base, i64imm:$disp);
}

// Symbol address operand formatted as @name [+ offset].
def SymOp : Operand<i64> {
  let PrintMethod = "printSymbolOperand";
}

// Symbol-relative memory operand formatted as [@name [+ offset]].
def SymMemOp : Operand<i64> {
  let PrintMethod = "printSymMemOperand";
}

// Basic-block branch target operand.
def BrTarget : Operand<OtherVT>;

def AddrRI  : ComplexPattern<i64, 2, "SelectAddrRI", [frameindex], []>;
def AddrSym : ComplexPattern<i64, 1, "SelectAddrSym", [], []>;

class InkVMInst<dag Outs, dag Ins, string AsmStr, list<dag> Pat = []> : Instruction {
  let Namespace = "InkVM";
  dag OutOperandList = Outs;
  dag InOperandList = Ins;
  let AsmString = AsmStr;
  let Pattern = Pat;
  let Size = 8;
}

// Call-frame reservation/reclamation pseudo instructions, lowered in eliminateCallFramePseudoInstr().
let Defs = [SP], Uses = [SP], isPseudo = 1, isCodeGenOnly = 1, hasSideEffects = 1 in {
  def ADJCALLSTACKDOWN : InkVMInst<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                                   "; ADJCALLSTACKDOWN $amt1, $amt2",
                                   [(InkVMCallSeqStart timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP   : InkVMInst<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                                   "; ADJCALLSTACKUP $amt1, $amt2",
                                   [(InkVMCallSeqEnd timm:$amt1, timm:$amt2)]>;
}

// 0x003A: ret (8-byte header word, no explicit operands).
let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : InkVMInst<(outs), (ins), "ret", [(InkVMRetGlue)]>;
}

// 0x002B, 0x0040: direct block jump and indirect register jump.
let isBranch = 1, isTerminator = 1, isBarrier = 1, Size = 16, hasSideEffects = 0 in {
  def JMP : InkVMInst<(outs), (ins BrTarget:$dst),
                      "jmp $dst",
                      [(br bb:$dst)]>;

  let isIndirectBranch = 1 in {
    def JMPR : InkVMInst<(outs), (ins GPR:$dst),
                         "jmpr $dst",
                         [(brind GPR:$dst)]>;
  }
}

// Multiclass generating paired 64-bit register-register (rrr) and register-immediate (rri) comparisons.
multiclass CMP64Op<string MnemonicBase, PatFrag CondNode, bit IsComm = 0> {
  let Size = 32, hasSideEffects = 0 in {
    let isCommutable = IsComm in {
      def 64rrr : InkVMInst<(outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                            !strconcat(MnemonicBase, "64rrr $dst, $lhs, $rhs"),
                            [(set GPR:$dst, (CondNode GPR:$lhs, GPR:$rhs))]>;
    }

    def 64rri : InkVMInst<(outs GPR:$dst), (ins GPR:$lhs, i64imm:$imm),
                          !strconcat(MnemonicBase, "64rri $dst, $lhs, $imm"),
                          [(set GPR:$dst, (CondNode GPR:$lhs, (i64 imm:$imm)))]>;
  }
}

// 0x00B8-0x00CB: 64-bit explicit register comparisons (write 1 when true, 0 otherwise).
defm CMPEQ  : CMP64Op<"cmpeq",  seteq,  1>;
defm CMPNE  : CMP64Op<"cmpne",  setne,  1>;
defm CMPLT  : CMP64Op<"cmplt",  setlt,  0>;
defm CMPLE  : CMP64Op<"cmple",  setle,  0>;
defm CMPGT  : CMP64Op<"cmpgt",  setgt,  0>;
defm CMPGE  : CMP64Op<"cmpge",  setge,  0>;
defm CMPLTU : CMP64Op<"cmpltu", setult, 0>;
defm CMPLEU : CMP64Op<"cmpleu", setule, 0>;
defm CMPGTU : CMP64Op<"cmpgtu", setugt, 0>;
defm CMPGEU : CMP64Op<"cmpgeu", setuge, 0>;

// Multiclass generating paired 64-bit flagless compare-and-branch instructions (rrb and rib).
multiclass JCC64Op<string MnemonicBase, CondCode CC> {
  let isBranch = 1, isTerminator = 1, Size = 32, hasSideEffects = 0 in {
    def 64rrb : InkVMInst<(outs), (ins GPR:$lhs, GPR:$rhs, BrTarget:$dst),
                          !strconcat(MnemonicBase, "64rrb $lhs, $rhs, $dst"),
                          [(brcc CC, GPR:$lhs, GPR:$rhs, bb:$dst)]>;

    def 64rib : InkVMInst<(outs), (ins GPR:$lhs, i64imm:$imm, BrTarget:$dst),
                          !strconcat(MnemonicBase, "64rib $lhs, $imm, $dst"),
                          [(brcc CC, GPR:$lhs, (i64 imm:$imm), bb:$dst)]>;
  }
}

// 0x00CC-0x00DF: 64-bit flagless compare-and-branch instructions.
defm JE   : JCC64Op<"je",   SETEQ>;
defm JNE  : JCC64Op<"jne",  SETNE>;
defm JLT  : JCC64Op<"jlt",  SETLT>;
defm JLE  : JCC64Op<"jle",  SETLE>;
defm JGT  : JCC64Op<"jgt",  SETGT>;
defm JGE  : JCC64Op<"jge",  SETGE>;
defm JLTU : JCC64Op<"jltu", SETULT>;
defm JLEU : JCC64Op<"jleu", SETULE>;
defm JGTU : JCC64Op<"jgtu", SETUGT>;
defm JGEU : JCC64Op<"jgeu", SETUGE>;

// Lowered into a diamond CFG + PHI in InkVMTargetLowering::EmitInstrWithCustomInserter().
let usesCustomInserter = 1, isPseudo = 1, isCodeGenOnly = 1, hasSideEffects = 0 in {
  def SELECT_CC_I64 : InkVMInst<(outs GPR:$dst),
                                (ins GPR:$lhs, GPR:$rhs, i64imm:$cc, GPR:$tval, GPR:$fval),
                                "; SELECT_CC_I64 $dst, $lhs, $rhs, $cc, $tval, $fval",
                                [(set GPR:$dst, (InkVMSelectCC GPR:$lhs, GPR:$rhs, timm:$cc, GPR:$tval, GPR:$fval))]>;
}

// 0x003B, 0x00F3: direct symbol call and indirect register call.
let isCall = 1, Size = 16, Uses = [SP] in {
  def CALLi : InkVMInst<(outs), (ins SymOp:$target),
                        "call $target",
                        [(InkVMCall (i64 tglobaladdr:$target))]>;

  def CALLr : InkVMInst<(outs), (ins GPR:$target),
                        "call $target",
                        [(InkVMCall GPR:$target)]>;
}

def : Pat<(InkVMCall (i64 texternalsym:$target)), (CALLi texternalsym:$target)>;

// 0x0097-0x0098: 64-bit register move, immediate load, and symbol address materialization.
let Size = 24, hasSideEffects = 0, isAsCheapAsAMove = 1 in {
  let isMoveReg = 1 in {
    def MOV64rr : InkVMInst<(outs GPR:$dst), (ins GPR:$src),
                            "mov64rr $dst, $src">;
  }

  let isMoveImm = 1, isReMaterializable = 1 in {
    def MOV64ri : InkVMInst<(outs GPR:$dst), (ins i64imm:$imm),
                            "mov64ri $dst, $imm",
                            [(set GPR:$dst, (i64 imm:$imm))]>;

    def MOV64r_sym : InkVMInst<(outs GPR:$dst), (ins SymOp:$sym),
                               "mov64ri $dst, $sym",
                               [(set GPR:$dst, (InkVMWrapper tglobaladdr:$sym))]>;
  }
}

def : Pat<(InkVMWrapper texternalsym:$sym), (MOV64r_sym texternalsym:$sym)>;

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

// Symbol-relative direct memory loads and stores: load*rm dst, [@sym + off] / store*mr [@sym + off], src.
// Higher complexity ensures ISel folds global symbol addresses directly instead of emitting mov64ri + [reg].
let Size = 24, hasSideEffects = 0, AddedComplexity = 5 in {
  let mayLoad = 1 in {
    def LOAD8urm_sym  : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load8urm $dst, $addr",
                                  [(set GPR:$dst, (zextloadi8 AddrSym:$addr))]>;
    def LOAD16urm_sym : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load16urm $dst, $addr",
                                  [(set GPR:$dst, (zextloadi16 AddrSym:$addr))]>;
    def LOAD32urm_sym : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load32urm $dst, $addr",
                                  [(set GPR:$dst, (zextloadi32 AddrSym:$addr))]>;

    def LOAD8srm_sym  : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load8srm $dst, $addr",
                                  [(set GPR:$dst, (sextloadi8 AddrSym:$addr))]>;
    def LOAD16srm_sym : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load16srm $dst, $addr",
                                  [(set GPR:$dst, (sextloadi16 AddrSym:$addr))]>;
    def LOAD32srm_sym : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load32srm $dst, $addr",
                                  [(set GPR:$dst, (sextloadi32 AddrSym:$addr))]>;

    def LOAD64rm_sym  : InkVMInst<(outs GPR:$dst), (ins SymMemOp:$addr),
                                  "load64rm $dst, $addr",
                                  [(set GPR:$dst, (load AddrSym:$addr))]>;
  }

  let mayStore = 1 in {
    def STORE8mr_sym  : InkVMInst<(outs), (ins SymMemOp:$addr, GPR:$src),
                                  "store8mr $addr, $src",
                                  [(truncstorei8 GPR:$src, AddrSym:$addr)]>;
    def STORE16mr_sym : InkVMInst<(outs), (ins SymMemOp:$addr, GPR:$src),
                                  "store16mr $addr, $src",
                                  [(truncstorei16 GPR:$src, AddrSym:$addr)]>;
    def STORE32mr_sym : InkVMInst<(outs), (ins SymMemOp:$addr, GPR:$src),
                                  "store32mr $addr, $src",
                                  [(truncstorei32 GPR:$src, AddrSym:$addr)]>;
    def STORE64mr_sym : InkVMInst<(outs), (ins SymMemOp:$addr, GPR:$src),
                                  "store64mr $addr, $src",
                                  [(store GPR:$src, AddrSym:$addr)]>;
  }
}

let AddedComplexity = 5 in {
  def : Pat<(i64 (extloadi8  AddrSym:$addr)), (LOAD8urm_sym  AddrSym:$addr)>;
  def : Pat<(i64 (extloadi16 AddrSym:$addr)), (LOAD16urm_sym AddrSym:$addr)>;
  def : Pat<(i64 (extloadi32 AddrSym:$addr)), (LOAD32urm_sym AddrSym:$addr)>;
}
