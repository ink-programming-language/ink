// InkVM calling convention and callee-saved register definitions.
// Per docs/VM-Calling-Convention.md, all general-purpose registers R0-R9 are caller-saved.
// BP and SP are saved/restored by call/ret, and ZERO is constant zero.

def CSR_InkVM : CalleeSavedRegs<(add)> {
  let OtherPreserved = (add BP, SP, ZERO);
}
