// InkVM instruction descriptions and SelectionDAG patterns.
// Opcode wire encodings, mnemonics, and byte sizes are generated from
// src/include/ink/backend/vm/instruction/*.def into InkVMOpcodeMetaGen.td.

include "InkVMOpcodeMetaGen.td"

// Target-specific SelectionDAG nodes.
def InkVMRetGlue : SDNode<"InkVMISD::RET_GLUE", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def InkVMCall : SDNode<"InkVMISD::CALL", SDTypeProfile<0, -1, [SDTCisVT<0, i64>]>,
                       [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;

def InkVMWrapper : SDNode<"InkVMISD::WRAPPER", SDTIntUnaryOp>;

def InkVMSelectCC : SDNode<"InkVMISD::SELECT_CC",
                           SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>, SDTCisVT<3, i64>,
                                                SDTCisSameAs<0, 4>, SDTCisSameAs<0, 5>]>>;

def InkVMFPConst : SDNode<"InkVMISD::FP_CONST",
                          SDTypeProfile<1, 1, [SDTCisVT<0, f64>, SDTCisVT<1, i64>]>>;

def InkVMBrNZ : SDNode<"InkVMISD::BR_NZ",
                       SDTypeProfile<0, 2, [SDTCisVT<0, i64>, SDTCisVT<1, OtherVT>]>,
                       [SDNPHasChain]>;

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

class InkVMPseudoInst<dag Outs, dag Ins, string AsmStr, list<dag> Pat = []> : Instruction {
  let Namespace = "InkVM";
  dag OutOperandList = Outs;
  dag InOperandList = Ins;
  let AsmString = AsmStr;
  let Pattern = Pat;
  let Size = 0;
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

class InkVMInst<InkVMOpcodeMeta Meta, dag Outs, dag Ins, string OperandAsm = "", list<dag> Pat = []>
    : Instruction {
  let Namespace = "InkVM";
  dag OutOperandList = Outs;
  dag InOperandList = Ins;
  let AsmString = !if(!empty(OperandAsm), Meta.Mnemonic, !strconcat(Meta.Mnemonic, " ", OperandAsm));
  let Pattern = Pat;
  let Size = Meta.Size;
  // TSFlags[15:0] carries the 16-bit VM wire opcode from opcode.def; TSFlags[16] marks real VM instructions.
  let TSFlags{15-0} = Meta.Encoding;
  let TSFlags{16} = 1;
}

// Call-frame reservation/reclamation pseudo instructions, lowered in eliminateCallFramePseudoInstr().
let Defs = [SP], Uses = [SP], hasSideEffects = 1 in {
  def ADJCALLSTACKDOWN : InkVMPseudoInst<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                                         "; ADJCALLSTACKDOWN $amt1, $amt2",
                                         [(InkVMCallSeqStart timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP   : InkVMPseudoInst<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                                         "; ADJCALLSTACKUP $amt1, $amt2",
                                         [(InkVMCallSeqEnd timm:$amt1, timm:$amt2)]>;
}

// Return from function (8-byte header word, no explicit operands).
let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : InkVMInst<InkVMOp_Ret, (outs), (ins), "", [(InkVMRetGlue)]>;
}

// Direct block jump and indirect register jump.
let isBranch = 1, isTerminator = 1, isBarrier = 1, hasSideEffects = 0 in {
  def JMP : InkVMInst<InkVMOp_Jmp, (outs), (ins BrTarget:$dst),
                      "$dst",
                      [(br bb:$dst)]>;

  let isIndirectBranch = 1 in {
    def JMPR : InkVMInst<InkVMOp_Jmpr, (outs), (ins GPR:$dst),
                         "$dst",
                         [(brind GPR:$dst)]>;
  }
}

// Multiclass generating paired 64-bit register-register (rrr) and register-immediate (rri) comparisons.
multiclass CMP64Op<InkVMOpcodeMeta MetaRR, InkVMOpcodeMeta MetaRI, PatFrag CondNode, bit IsComm = 0> {
  let hasSideEffects = 0 in {
    let isCommutable = IsComm in {
      def 64rrr : InkVMInst<MetaRR, (outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                            "$dst, $lhs, $rhs",
                            [(set GPR:$dst, (CondNode GPR:$lhs, GPR:$rhs))]>;
    }

    def 64rri : InkVMInst<MetaRI, (outs GPR:$dst), (ins GPR:$lhs, i64imm:$imm),
                          "$dst, $lhs, $imm",
                          [(set GPR:$dst, (CondNode GPR:$lhs, (i64 imm:$imm)))]>;
  }
}

// 64-bit explicit register comparisons (write 1 when true, 0 otherwise).
defm CMPEQ  : CMP64Op<InkVMOp_Cmpeq64rrr,  InkVMOp_Cmpeq64rri,  seteq,  1>;
defm CMPNE  : CMP64Op<InkVMOp_Cmpne64rrr,  InkVMOp_Cmpne64rri,  setne,  1>;
defm CMPLT  : CMP64Op<InkVMOp_Cmplt64rrr,  InkVMOp_Cmplt64rri,  setlt,  0>;
defm CMPLE  : CMP64Op<InkVMOp_Cmple64rrr,  InkVMOp_Cmple64rri,  setle,  0>;
defm CMPGT  : CMP64Op<InkVMOp_Cmpgt64rrr,  InkVMOp_Cmpgt64rri,  setgt,  0>;
defm CMPGE  : CMP64Op<InkVMOp_Cmpge64rrr,  InkVMOp_Cmpge64rri,  setge,  0>;
defm CMPLTU : CMP64Op<InkVMOp_Cmpltu64rrr, InkVMOp_Cmpltu64rri, setult, 0>;
defm CMPLEU : CMP64Op<InkVMOp_Cmpleu64rrr, InkVMOp_Cmpleu64rri, setule, 0>;
defm CMPGTU : CMP64Op<InkVMOp_Cmpgtu64rrr, InkVMOp_Cmpgtu64rri, setugt, 0>;
defm CMPGEU : CMP64Op<InkVMOp_Cmpgeu64rrr, InkVMOp_Cmpgeu64rri, setuge, 0>;

// Multiclass generating paired 64-bit flagless compare-and-branch instructions (rrb and rib).
multiclass JCC64Op<InkVMOpcodeMeta MetaRR, InkVMOpcodeMeta MetaRI, CondCode CC> {
  let isBranch = 1, isTerminator = 1, hasSideEffects = 0 in {
    def 64rrb : InkVMInst<MetaRR, (outs), (ins GPR:$lhs, GPR:$rhs, BrTarget:$dst),
                          "$lhs, $rhs, $dst",
                          [(brcc CC, GPR:$lhs, GPR:$rhs, bb:$dst)]>;

    def 64rib : InkVMInst<MetaRI, (outs), (ins GPR:$lhs, i64imm:$imm, BrTarget:$dst),
                          "$lhs, $imm, $dst",
                          [(brcc CC, GPR:$lhs, (i64 imm:$imm), bb:$dst)]>;
  }
}

// 64-bit flagless compare-and-branch instructions.
defm JE   : JCC64Op<InkVMOp_Je64rrb,   InkVMOp_Je64rib,   SETEQ>;
defm JNE  : JCC64Op<InkVMOp_Jne64rrb,  InkVMOp_Jne64rib,  SETNE>;
defm JLT  : JCC64Op<InkVMOp_Jlt64rrb,  InkVMOp_Jlt64rib,  SETLT>;
defm JLE  : JCC64Op<InkVMOp_Jle64rrb,  InkVMOp_Jle64rib,  SETLE>;
defm JGT  : JCC64Op<InkVMOp_Jgt64rrb,  InkVMOp_Jgt64rib,  SETGT>;
defm JGE  : JCC64Op<InkVMOp_Jge64rrb,  InkVMOp_Jge64rib,  SETGE>;
defm JLTU : JCC64Op<InkVMOp_Jltu64rrb, InkVMOp_Jltu64rib, SETULT>;
defm JLEU : JCC64Op<InkVMOp_Jleu64rrb, InkVMOp_Jleu64rib, SETULE>;
defm JGTU : JCC64Op<InkVMOp_Jgtu64rrb, InkVMOp_Jgtu64rib, SETUGT>;
defm JGEU : JCC64Op<InkVMOp_Jgeu64rrb, InkVMOp_Jgeu64rib, SETUGE>;

// FP64 conditional branches lower via fcmp64*rrr + InkVMISD::BR_NZ -> jne64rib cond, 0, dst.
def : Pat<(InkVMBrNZ GPR:$cond, bb:$dst), (JNE64rib GPR:$cond, 0, bb:$dst)>;

// Lowered into a diamond CFG + PHI in InkVMTargetLowering::EmitInstrWithCustomInserter().
let usesCustomInserter = 1, hasSideEffects = 0 in {
  def SELECT_CC_I64 : InkVMPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$lhs, GPR:$rhs, i64imm:$cc, GPR:$tval, GPR:$fval),
                                      "; SELECT_CC_I64 $dst, $lhs, $rhs, $cc, $tval, $fval",
                                      [(set GPR:$dst, (InkVMSelectCC GPR:$lhs, GPR:$rhs, timm:$cc, GPR:$tval, GPR:$fval))]>;

  def SELECT_CC_F64 : InkVMPseudoInst<(outs FPR:$dst),
                                      (ins GPR:$lhs, GPR:$rhs, i64imm:$cc, FPR:$tval, FPR:$fval),
                                      "; SELECT_CC_F64 $dst, $lhs, $rhs, $cc, $tval, $fval",
                                      [(set FPR:$dst, (InkVMSelectCC GPR:$lhs, GPR:$rhs, timm:$cc, FPR:$tval, FPR:$fval))]>;
}

// Direct symbol call and indirect register call.
let isCall = 1, Uses = [SP] in {
  def CALLi : InkVMInst<InkVMOp_Calli, (outs), (ins SymOp:$target),
                        "$target",
                        [(InkVMCall (i64 tglobaladdr:$target))]>;

  def CALLr : InkVMInst<InkVMOp_Callr, (outs), (ins GPR:$target),
                        "$target",
                        [(InkVMCall GPR:$target)]>;
}

def : Pat<(InkVMCall (i64 texternalsym:$target)), (CALLi texternalsym:$target)>;

// 64-bit register move, immediate load, and symbol address materialization.
let hasSideEffects = 0, isAsCheapAsAMove = 1 in {
  let isMoveReg = 1 in {
    def MOV64rr : InkVMInst<InkVMOp_Mov64rr, (outs GPR:$dst), (ins GPR:$src),
                            "$dst, $src">;
  }

  let isMoveImm = 1, isReMaterializable = 1 in {
    def MOV64ri : InkVMInst<InkVMOp_Mov64ri, (outs GPR:$dst), (ins i64imm:$imm),
                            "$dst, $imm",
                            [(set GPR:$dst, (i64 imm:$imm))]>;

    def MOV64r_sym : InkVMInst<InkVMOp_Mov64ri, (outs GPR:$dst), (ins SymOp:$sym),
                               "$dst, $sym",
                               [(set GPR:$dst, (InkVMWrapper tglobaladdr:$sym))]>;
  }
}

def : Pat<(InkVMWrapper texternalsym:$sym), (MOV64r_sym texternalsym:$sym)>;

// Multiclass generating paired 64-bit register-register (rrr) and register-immediate (rri) ALU ops.
multiclass ALU64BinOp<InkVMOpcodeMeta MetaRR, InkVMOpcodeMeta MetaRI, SDNode OpNode, bit IsComm = 0> {
  let hasSideEffects = 0 in {
    let isCommutable = IsComm in {
      def 64rrr : InkVMInst<MetaRR, (outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                            "$dst, $lhs, $rhs",
                            [(set GPR:$dst, (OpNode GPR:$lhs, GPR:$rhs))]>;
    }

    def 64rri : InkVMInst<MetaRI, (outs GPR:$dst), (ins GPR:$lhs, i64imm:$imm),
                          "$dst, $lhs, $imm",
                          [(set GPR:$dst, (OpNode GPR:$lhs, (i64 imm:$imm)))]>;
  }
}

// 64-bit integer add, sub, mul.
defm ADD : ALU64BinOp<InkVMOp_Add64rrr, InkVMOp_Add64rri, add, 1>;
defm SUB : ALU64BinOp<InkVMOp_Sub64rrr, InkVMOp_Sub64rri, sub, 0>;
defm MUL : ALU64BinOp<InkVMOp_Mul64rrr, InkVMOp_Mul64rri, mul, 1>;

// 64-bit bitwise operations and shifts.
defm AND  : ALU64BinOp<InkVMOp_And64rrr,  InkVMOp_And64rri,  and, 1>;
defm OR   : ALU64BinOp<InkVMOp_Or64rrr,   InkVMOp_Or64rri,   or,  1>;
defm XOR  : ALU64BinOp<InkVMOp_Xor64rrr,  InkVMOp_Xor64rri,  xor, 1>;
defm SHL  : ALU64BinOp<InkVMOp_Shl64rrr,  InkVMOp_Shl64rri,  shl, 0>;
defm LSHR : ALU64BinOp<InkVMOp_Lshr64rrr, InkVMOp_Lshr64rri, srl, 0>;
defm ASHR : ALU64BinOp<InkVMOp_Ashr64rrr, InkVMOp_Ashr64rri, sra, 0>;

// Negation uses architectural ZERO as LHS: sub64rrr dst, zero, rhs.
def : Pat<(sub 0, GPR:$rhs), (SUB64rrr ZERO, GPR:$rhs)>;

// 64-bit signed/unsigned division and remainder (traps on /0 or MIN/-1).
class ALU64DivRem<InkVMOpcodeMeta Meta, SDNode OpNode>
    : InkVMInst<Meta, (outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                "$dst, $lhs, $rhs",
                [(set GPR:$dst, (OpNode GPR:$lhs, GPR:$rhs))]> {
  let hasSideEffects = 1;
}

def SDIV64rrr : ALU64DivRem<InkVMOp_Sdiv64rrr, sdiv>;
def UDIV64rrr : ALU64DivRem<InkVMOp_Udiv64rrr, udiv>;
def SREM64rrr : ALU64DivRem<InkVMOp_Srem64rrr, srem>;
def UREM64rrr : ALU64DivRem<InkVMOp_Urem64rrr, urem>;

// 8/16/32/64-bit register-relative memory loads (zero/sign-extending) and stores.
let hasSideEffects = 0 in {
  let mayLoad = 1 in {
    def LOAD8urm  : InkVMInst<InkVMOp_Load8urm,  (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (zextloadi8 AddrRI:$addr))]>;
    def LOAD16urm : InkVMInst<InkVMOp_Load16urm, (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (zextloadi16 AddrRI:$addr))]>;
    def LOAD32urm : InkVMInst<InkVMOp_Load32urm, (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (zextloadi32 AddrRI:$addr))]>;

    def LOAD8srm  : InkVMInst<InkVMOp_Load8srm,  (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (sextloadi8 AddrRI:$addr))]>;
    def LOAD16srm : InkVMInst<InkVMOp_Load16srm, (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (sextloadi16 AddrRI:$addr))]>;
    def LOAD32srm : InkVMInst<InkVMOp_Load32srm, (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (sextloadi32 AddrRI:$addr))]>;

    def LOAD64rm  : InkVMInst<InkVMOp_Load64rm,  (outs GPR:$dst), (ins MemOp:$addr),
                              "$dst, $addr",
                              [(set GPR:$dst, (load AddrRI:$addr))]>;
  }

  let mayStore = 1 in {
    def STORE8mr  : InkVMInst<InkVMOp_Store8mr,  (outs), (ins MemOp:$addr, GPR:$src),
                              "$addr, $src",
                              [(truncstorei8 GPR:$src, AddrRI:$addr)]>;
    def STORE16mr : InkVMInst<InkVMOp_Store16mr, (outs), (ins MemOp:$addr, GPR:$src),
                              "$addr, $src",
                              [(truncstorei16 GPR:$src, AddrRI:$addr)]>;
    def STORE32mr : InkVMInst<InkVMOp_Store32mr, (outs), (ins MemOp:$addr, GPR:$src),
                              "$addr, $src",
                              [(truncstorei32 GPR:$src, AddrRI:$addr)]>;
    def STORE64mr : InkVMInst<InkVMOp_Store64mr, (outs), (ins MemOp:$addr, GPR:$src),
                              "$addr, $src",
                              [(store GPR:$src, AddrRI:$addr)]>;
  }
}

// Any-extending narrow loads reuse unsigned zero-extending loads.
def : Pat<(i64 (extloadi8  AddrRI:$addr)), (LOAD8urm  AddrRI:$addr)>;
def : Pat<(i64 (extloadi16 AddrRI:$addr)), (LOAD16urm AddrRI:$addr)>;
def : Pat<(i64 (extloadi32 AddrRI:$addr)), (LOAD32urm AddrRI:$addr)>;

// Symbol-relative direct memory loads and stores: load*rm dst, [@sym + off] / store*mr [@sym + off], src.
// Higher complexity ensures ISel folds global symbol addresses directly instead of emitting mov64ri + [reg].
let hasSideEffects = 0, AddedComplexity = 5 in {
  let mayLoad = 1 in {
    def LOAD8urm_sym  : InkVMInst<InkVMOp_Load8urm,  (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (zextloadi8 AddrSym:$addr))]>;
    def LOAD16urm_sym : InkVMInst<InkVMOp_Load16urm, (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (zextloadi16 AddrSym:$addr))]>;
    def LOAD32urm_sym : InkVMInst<InkVMOp_Load32urm, (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (zextloadi32 AddrSym:$addr))]>;

    def LOAD8srm_sym  : InkVMInst<InkVMOp_Load8srm,  (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (sextloadi8 AddrSym:$addr))]>;
    def LOAD16srm_sym : InkVMInst<InkVMOp_Load16srm, (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (sextloadi16 AddrSym:$addr))]>;
    def LOAD32srm_sym : InkVMInst<InkVMOp_Load32srm, (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (sextloadi32 AddrSym:$addr))]>;

    def LOAD64rm_sym  : InkVMInst<InkVMOp_Load64rm,  (outs GPR:$dst), (ins SymMemOp:$addr),
                                  "$dst, $addr",
                                  [(set GPR:$dst, (load AddrSym:$addr))]>;
  }

  let mayStore = 1 in {
    def STORE8mr_sym  : InkVMInst<InkVMOp_Store8mr,  (outs), (ins SymMemOp:$addr, GPR:$src),
                                  "$addr, $src",
                                  [(truncstorei8 GPR:$src, AddrSym:$addr)]>;
    def STORE16mr_sym : InkVMInst<InkVMOp_Store16mr, (outs), (ins SymMemOp:$addr, GPR:$src),
                                  "$addr, $src",
                                  [(truncstorei16 GPR:$src, AddrSym:$addr)]>;
    def STORE32mr_sym : InkVMInst<InkVMOp_Store32mr, (outs), (ins SymMemOp:$addr, GPR:$src),
                                  "$addr, $src",
                                  [(truncstorei32 GPR:$src, AddrSym:$addr)]>;
    def STORE64mr_sym : InkVMInst<InkVMOp_Store64mr, (outs), (ins SymMemOp:$addr, GPR:$src),
                                  "$addr, $src",
                                  [(store GPR:$src, AddrSym:$addr)]>;
  }
}

let AddedComplexity = 5 in {
  def : Pat<(i64 (extloadi8  AddrSym:$addr)), (LOAD8urm_sym  AddrSym:$addr)>;
  def : Pat<(i64 (extloadi16 AddrSym:$addr)), (LOAD16urm_sym AddrSym:$addr)>;
  def : Pat<(i64 (extloadi32 AddrSym:$addr)), (LOAD32urm_sym AddrSym:$addr)>;
}

// FP64 loads and stores (same wire opcodes as 64-bit integer loads/stores).
let hasSideEffects = 0 in {
  let mayLoad = 1 in {
    def LOAD64rm_f64 : InkVMInst<InkVMOp_Load64rm, (outs FPR:$dst), (ins MemOp:$addr),
                                 "$dst, $addr",
                                 [(set FPR:$dst, (load AddrRI:$addr))]>;
  }

  let mayStore = 1 in {
    def STORE64mr_f64 : InkVMInst<InkVMOp_Store64mr, (outs), (ins MemOp:$addr, FPR:$src),
                                  "$addr, $src",
                                  [(store FPR:$src, AddrRI:$addr)]>;
  }
}

let hasSideEffects = 0, AddedComplexity = 5 in {
  let mayLoad = 1 in {
    def LOAD64rm_f64_sym : InkVMInst<InkVMOp_Load64rm, (outs FPR:$dst), (ins SymMemOp:$addr),
                                     "$dst, $addr",
                                     [(set FPR:$dst, (load AddrSym:$addr))]>;
  }

  let mayStore = 1 in {
    def STORE64mr_f64_sym : InkVMInst<InkVMOp_Store64mr, (outs), (ins SymMemOp:$addr, FPR:$src),
                                      "$addr, $src",
                                      [(store FPR:$src, AddrSym:$addr)]>;
  }
}

// Unified R0-R9 register file: i64<->f64 bitcasts coalesce to zero instructions when already in R0-R9.
def : Pat<(f64 (bitconvert GPR:$src)), (COPY_TO_REGCLASS GPR:$src, FPR)>;
def : Pat<(i64 (bitconvert FPR:$src)), (COPY_TO_REGCLASS FPR:$src, GPR)>;

// Materialize any 64-bit IEEE-754 double literal directly via mov64ri bit pattern.
let hasSideEffects = 0, isAsCheapAsAMove = 1, isMoveImm = 1, isReMaterializable = 1 in {
  def MOV64ri_f64 : InkVMInst<InkVMOp_Mov64ri, (outs FPR:$dst), (ins i64imm:$imm),
                              "$dst, $imm",
                              [(set FPR:$dst, (InkVMFPConst timm:$imm))]>;
}

// IEEE-754 FP64 arithmetic, square root, and unary negation.
class FP64BinOp<InkVMOpcodeMeta Meta, SDNode OpNode, bit IsComm = 0>
    : InkVMInst<Meta, (outs FPR:$dst), (ins FPR:$lhs, FPR:$rhs),
                "$dst, $lhs, $rhs",
                [(set FPR:$dst, (OpNode FPR:$lhs, FPR:$rhs))]> {
  let hasSideEffects = 0;
  let isCommutable = IsComm;
}

def FADD64rrr : FP64BinOp<InkVMOp_Fadd64rrr, fadd, 1>;
def FSUB64rrr : FP64BinOp<InkVMOp_Fsub64rrr, fsub, 0>;
def FMUL64rrr : FP64BinOp<InkVMOp_Fmul64rrr, fmul, 1>;
def FDIV64rrr : FP64BinOp<InkVMOp_Fdiv64rrr, fdiv, 0>;

let hasSideEffects = 0 in {
  def FSQRT64rr : InkVMInst<InkVMOp_Fsqrt64rr, (outs FPR:$dst), (ins FPR:$src),
                            "$dst, $src",
                            [(set FPR:$dst, (fsqrt FPR:$src))]>;

  def FNEG64rr : InkVMInst<InkVMOp_Fneg64rr, (outs FPR:$dst), (ins FPR:$src),
                           "$dst, $src",
                           [(set FPR:$dst, (fneg FPR:$src))]>;

  def FABS64rr : InkVMInst<InkVMOp_Fabs64rr, (outs FPR:$dst), (ins FPR:$src),
                           "$dst, $src",
                           [(set FPR:$dst, (fabs FPR:$src))]>;
}

// 14 IEEE-754 ordered/unordered FP64 comparisons (write 0 or 1 to R0-R9).
class FCMP64Op<InkVMOpcodeMeta Meta, PatFrag CondNode, bit IsComm = 0>
    : InkVMInst<Meta, (outs GPRAlloc:$dst), (ins FPR:$lhs, FPR:$rhs),
                "$dst, $lhs, $rhs",
                [(set GPRAlloc:$dst, (CondNode FPR:$lhs, FPR:$rhs))]> {
  let hasSideEffects = 0;
  let isCommutable = IsComm;
}

def FCMP64OEQrrr : FCMP64Op<InkVMOp_Fcmp64oeqrrr, setoeq, 1>;
def FCMP64ONErrr : FCMP64Op<InkVMOp_Fcmp64onerrr, setone, 1>;
def FCMP64OLTrrr : FCMP64Op<InkVMOp_Fcmp64oltrrr, setolt, 0>;
def FCMP64OLErrr : FCMP64Op<InkVMOp_Fcmp64olerrr, setole, 0>;
def FCMP64OGTrrr : FCMP64Op<InkVMOp_Fcmp64ogtrrr, setogt, 0>;
def FCMP64OGErrr : FCMP64Op<InkVMOp_Fcmp64ogerrr, setoge, 0>;
def FCMP64ORDrrr : FCMP64Op<InkVMOp_Fcmp64ordrrr, seto,   1>;
def FCMP64UNOrrr : FCMP64Op<InkVMOp_Fcmp64unorrr, setuo,  1>;
def FCMP64UEQrrr : FCMP64Op<InkVMOp_Fcmp64ueqrrr, setueq, 1>;
def FCMP64UNErrr : FCMP64Op<InkVMOp_Fcmp64unerrr, setune, 1>;
def FCMP64ULTrrr : FCMP64Op<InkVMOp_Fcmp64ultrrr, setult, 0>;
def FCMP64ULErrr : FCMP64Op<InkVMOp_Fcmp64ulerrr, setule, 0>;
def FCMP64UGTrrr : FCMP64Op<InkVMOp_Fcmp64ugtrrr, setugt, 0>;
def FCMP64UGErrr : FCMP64Op<InkVMOp_Fcmp64ugerrr, setuge, 0>;

// Map unspecified-NaN FP condition codes (e.g. fast-math folds) to ordered FP64 comparisons.
def : Pat<(i64 (seteq FPR:$lhs, FPR:$rhs)), (FCMP64OEQrrr FPR:$lhs, FPR:$rhs)>;
def : Pat<(i64 (setne FPR:$lhs, FPR:$rhs)), (FCMP64ONErrr FPR:$lhs, FPR:$rhs)>;
def : Pat<(i64 (setlt FPR:$lhs, FPR:$rhs)), (FCMP64OLTrrr FPR:$lhs, FPR:$rhs)>;
def : Pat<(i64 (setle FPR:$lhs, FPR:$rhs)), (FCMP64OLErrr FPR:$lhs, FPR:$rhs)>;
def : Pat<(i64 (setgt FPR:$lhs, FPR:$rhs)), (FCMP64OGTrrr FPR:$lhs, FPR:$rhs)>;
def : Pat<(i64 (setge FPR:$lhs, FPR:$rhs)), (FCMP64OGErrr FPR:$lhs, FPR:$rhs)>;

// 64-bit signed/unsigned integer <-> FP64 conversions (operands constrained to R0-R9).
let hasSideEffects = 0 in {
  def SITOF64rr : InkVMInst<InkVMOp_Sitof64rr, (outs FPR:$dst), (ins GPRAlloc:$src),
                            "$dst, $src",
                            [(set FPR:$dst, (sint_to_fp GPRAlloc:$src))]>;

  def UITOF64rr : InkVMInst<InkVMOp_Uitof64rr, (outs FPR:$dst), (ins GPRAlloc:$src),
                            "$dst, $src",
                            [(set FPR:$dst, (uint_to_fp GPRAlloc:$src))]>;

  def F64TOSI64rr : InkVMInst<InkVMOp_F64tosi64rr, (outs GPRAlloc:$dst), (ins FPR:$src),
                              "$dst, $src",
                              [(set GPRAlloc:$dst, (fp_to_sint FPR:$src))]>;

  def F64TOUI64rr : InkVMInst<InkVMOp_F64toui64rr, (outs GPRAlloc:$dst), (ins FPR:$src),
                              "$dst, $src",
                              [(set GPRAlloc:$dst, (fp_to_uint FPR:$src))]>;
}
