; MachineModule text example: compute 20 + 22 in two stack slots.
.module "sum_example"

.section .rodata
.constant message, 1
    .byte 0x48, 0x69, 0x00 ; "Hi\0"
.endconstant

.section .data
.global scratch, 16, 8
    .byte 0x00, 0x00, 0x00, 0x00
.endglobal

.section .text
.function sum
    .stackslot 4, 4 ; input: offset 0
    .stackslot 4, 4 ; result: offset 4
    .frame 0
entry:
    ; Assume BP and SP initially point to the start of this local area.
    add64rri sp, sp, 8
    store32i [bp], 20
    add32mmi [bp + 4], [bp], 22 ; result = 42
    add64rri sp, sp, -8
.endfunction
