// InkVM register file description.
// Hardware encodings match src/include/ink/backend/vm/instruction/register.def 1:1.

class InkVMReg<bits<8> Encoding, string Name> : Register<Name> {
  let Namespace = "InkVM";
  let HWEncoding{7-0} = Encoding;
}

// Special architectural registers (reserved from general register allocation).
def BP   : InkVMReg<0x00, "bp">,   DwarfRegNum<[0]>;
def SP   : InkVMReg<0x01, "sp">,   DwarfRegNum<[1]>;
def PC   : InkVMReg<0x02, "pc">,   DwarfRegNum<[2]>;
def ZERO : InkVMReg<0x03, "zero">, DwarfRegNum<[3]>;

// General-purpose 64-bit registers R0-R9 (encodings 0x04..0x0D), all caller-saved.
foreach I = 0-9 in {
  def R#I : InkVMReg<!add(4, I), "r"#I>, DwarfRegNum<[!add(4, I)]>;
}

// Primary 64-bit register class.
// BP, SP, and ZERO appear in GPR so instructions referencing stack/frame/zero operands share one class;
// InkVMRegisterInfo::getReservedRegs() keeps them out of register allocation.
def GPR : RegisterClass<"InkVM", [i64], 64, (add
  (sequence "R%u", 0, 9),
  BP, SP, ZERO
)>;
