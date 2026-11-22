.module "B"
.declare global @buffer_b

.section .data
.global @counter size 8 align 8 external
    .byte 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal
.global @scratch size 8 align 8
    .byte 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal

.section .text
.function @helper external
entry:
    load64 [bp], [@scratch]
    load64 [bp + 16], [@buffer_b + 8]
    ret
.endfunction
