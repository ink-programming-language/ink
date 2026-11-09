; MachineModule text example: compute and check 20 + 22 in local stack slots.
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
    .stackslot 4, 4 ; input: offset 16
    .stackslot 4, 4 ; result: offset 20
    .frame 16
entry:
    ; Assume the call frame saved BP and PC, with SP initially equal to BP + 16.
    add64rri sp, sp, 8
    store32i [bp + 16], 20
    add32mmi [bp + 20], [bp + 16], 22 ; result = 42
    cmp32mi [bp + 20], 42
    jne failure
    jmp done
failure:
    store32i [bp + 20], 0
done:
    ret
.endfunction
