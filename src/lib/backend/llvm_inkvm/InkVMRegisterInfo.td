// InkVM register file description.
// Architectural registers and encodings are generated from
// src/include/ink/backend/vm/instruction/register.def into InkVMRegisterInfoGen.td.

include "InkVMRegisterInfoGen.td"

// Primary 64-bit integer/address register class.
// BP, SP, and ZERO appear in GPR so instructions referencing stack/frame/zero operands share one class;
// InkVMRegisterInfo::getReservedRegs() keeps them out of register allocation.
def GPR : RegisterClass<"InkVM", [i64], 64, (add
  (sequence "R%u", 0, InkVMGPRMaxIndex),
  BP, SP, ZERO
)>;

// Allocatable general-purpose integer registers (R0-R9 only, excluding BP/SP/ZERO).
// Used for integer operands of FP-domain instructions where the VM validator forbids BP/SP.
def GPRAlloc : RegisterClass<"InkVM", [i64], 64, (add
  (sequence "R%u", 0, InkVMGPRMaxIndex)
)>;

// 64-bit IEEE-754 floating-point register class (R0-R9 only, per VM validator rules).
// Shares physical registers R0-R9 with GPR so i64<->f64 bitcasts coalesce to zero instructions.
def FPR : RegisterClass<"InkVM", [f64], 64, (add
  (sequence "R%u", 0, InkVMGPRMaxIndex)
)>;
